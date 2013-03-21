#include "ica/ica.h"
#include "ica/aux.h"
#include "ica/setup.h"
#include "ica/fastica/contrast.h"
#include "ica/fastica/kernels.h"

#include <math.h>
#include <cublas.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Global variables setup in our initialization function. Setup of these
 * variables is an overhead that we shouldn't have to incur for every run of
 * the fastica() computation, since the values for these variables is dependent
 * on the ICA configuration parameters and nothing else.
 */
static Matrix _d_W[6] = {0}, _d_Z[2] = {0};     // Device matrices.
static Matrix _h_white = {0}, _h_dewhite = {0}; // Host matrices.
static GPUContFunc _contrast = NULL;            // Contrast function we apply.
static int _device = 0;                         // Which GPU device to use.

static NUMTYPE *_diag = NULL, *_eig_vals = NULL, *_mu_X = NULL; // Host memory.
static NUMTYPE *_d_sum = NULL;                  // Device memory.

static unsigned int _max_iter = 0;              // Max iterations to perform.

static dim3   _sum_grid(0);     // Grid size for summing kernel.
static dim3   _sum_block(0);    // Block size for summing kernel.
static size_t _sum_mem = 0;     // Size of shared memory for summing kernel.

static dim3 _scale_grid(0);     // Grid size for scaling kernel.
static dim3 _scale_block(0);    // Block size for scaling kernel.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int fastica_gpuInit( ICAParams *params )
{
  unsigned int i, size_diff, padded_w_size;

  _device = params->gpu_device;
  cudaSetDevice( _device );
  cublasInit();

  _max_iter = params->max_iter;

  switch (params->contrast) {
    case NONLIN_CUBE:
      _contrast = gpu_negent_cube;
      break;
    case NONLIN_GAUSS:
      _contrast = gpu_negent_gauss;
      break;
    case NONLIN_TANH:
    default:
      _contrast = gpu_negent_tanh;
      break;
  }

  _sum_grid  = dim3( 1 );
  _sum_block = dim3( 1, params->num_var );
  _sum_mem   = sizeof(NUMTYPE) * params->num_var;

  //////////////////////////////////////////////////////////////////////////////
  // Allocate device memory.
  //////////////////////////////////////////////////////////////////////////////

  // We pad the ends of the 'W' matrices to ensure that we have allocated space
  // for a multiple of 256 floating point numbers. This makes thread allocation
  // later on easier on us.
  size_diff = 256 - ((params->num_var * params->num_var) % 256);
  if (size_diff == 256) { size_diff = 0; }

  padded_w_size = params->num_var * params->num_var + size_diff;
  _scale_grid  = dim3( padded_w_size / 256 );
  _scale_block = dim3( 256 );

  for (i = 0; i < sizeof(_d_W) / sizeof(Matrix); i++) {
    cudaMalloc( (void**) &(_d_W[i].elem), sizeof(float) * padded_w_size );
    _d_W[i].rows = _d_W[i].ld  = params->num_var;
    _d_W[i].cols = _d_W[i].lag = params->num_var;
  }

  // We pad the end of the observation matrices for the same reason, though for
  // these guys we want each row to have a multiple of 256 elements. We don't
  // want those extra elements affecting things, though, so we set them to 0.
  size_diff = 256 - (params->num_obs % 256);
  if (size_diff == 256) {
    size_diff = 0;
  }

  for (i = 0; i < sizeof(_d_Z) / sizeof(Matrix); i++) {
    cudaMalloc((void**) &(_d_Z[i].elem),
               sizeof(float) * params->num_var * (params->num_obs + size_diff));

    _d_Z[i].rows = _d_Z[i].ld = params->num_var;
    _d_Z[i].cols = params->num_obs;
    _d_Z[i].lag  = params->num_obs + size_diff;

    cudaMemset( _d_Z[i].elem + params->num_var * params->num_obs, 0,
                size_diff * params->num_var * sizeof(float) );
  }

  cudaMalloc( (void**) &_d_sum, sizeof(float) );

  //////////////////////////////////////////////////////////////////////////////
  // Allocate host memory.
  //////////////////////////////////////////////////////////////////////////////
  _h_dewhite.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) *
                                       params->num_var * params->num_var );
  _h_white.elem   = (NUMTYPE*) malloc( sizeof(NUMTYPE) *
                                       params->num_var * params->num_var );

  _h_dewhite.lag  = _h_white.lag  = params->num_var;
  _h_dewhite.cols = _h_white.cols = params->num_var;
  _h_dewhite.rows = _h_white.rows = params->num_var;
  _h_dewhite.ld   = _h_white.ld   = params->num_var;

  _eig_vals = (NUMTYPE*) malloc( sizeof(NUMTYPE) * params->num_var );
  _diag     = (NUMTYPE*) malloc( sizeof(NUMTYPE) * params->num_var );
  _mu_X     = (NUMTYPE*) malloc( sizeof(NUMTYPE) * params->num_var );

  // Return that everything went OK.
  // TODO: check for CUDA errors.
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void fastica_gpuShutdown()
{
  int i;

  //////////////////////////////////////////////////////////////////////////////
  // Free allocated memory and set everything to NULL/0.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < sizeof(_d_W) / sizeof(Matrix); i++) {
    cudaFree( _d_W[i].elem );
    _d_W[i].elem = NULL;
    _d_W[i].ld = _d_W[i].lag = _d_W[i].rows = _d_W[i].cols = 0;
  }

  for (i = 0; i < sizeof(_d_Z) / sizeof(Matrix); i++) {
    cudaFree( _d_Z[i].elem );
    _d_Z[i].elem = NULL;
    _d_Z[i].ld = _d_Z[i].lag = _d_Z[i].rows = _d_Z[i].cols = 0;
  }

  cudaFree( _d_sum );
  _d_sum = NULL;

  free( _h_dewhite.elem ); free( _h_white.elem ); free( _eig_vals );
  free( _diag ); free( _mu_X );

  _mu_X = _diag = _eig_vals = _h_dewhite.elem = _h_white.elem = NULL;
  _h_dewhite.ld = _h_dewhite.lag = _h_dewhite.rows = _h_dewhite.cols = 0;
  _h_white.ld = _h_white.lag = _h_white.rows = _h_white.cols = 0;

  _contrast = NULL;
  _device = _max_iter = 0;

  _sum_grid = _sum_block = dim3(0);
  _sum_mem = 0;

  _scale_grid = _scale_block = dim3(0);

  cublasShutdown();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int fastica_gpu( Matrix *W, Matrix *A, Matrix *S, float *mu_S,
                          Matrix const *X )
{
  int row, col, i, num_iter;
  float eig_inv_sqr, eig_sqr;

  //////////////////////////////////////////////////////////////////////////////
  // Make the observations zero-mean and find the matrix that will make those
  // zero-mean observations 'white' using the CPU. We only use the CPU here
  // because we don't have a handy function for computing eigenvalues (needed to
  // make the observations 'white') on the GPU.
  //////////////////////////////////////////////////////////////////////////////

  // Find the mean of each row of the observation matrix.
  remmean( _mu_X, S, X );

  //////////////////////////////////////////////////////////////////////////////
  // Find the whitening/dewhitening matrices.
  //////////////////////////////////////////////////////////////////////////////

  // We use the CPU for this because we don't have a convenient method for
  // getting the eigenvalue decomposition using the GPU.
  COVARIANCE( *W, *S );
  SYEV( *W, _eig_vals );

  for (col = 0; col < W->cols; col++) {
    eig_inv_sqr = 1.0 / sqrt( _eig_vals[col] );
    eig_sqr     = sqrt( _eig_vals[col] );

    for (row = 0; row < W->rows; row++) {
      i = col * W->rows + row;
      _h_white.elem[ row * W->rows + col ] = eig_inv_sqr * W->elem[i];
      _h_dewhite.elem[i] = eig_sqr * W->elem[i];
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Copy the things we've calculated so far to the GPU. Almost all of the rest
  // of the ICA algorithm will now take place using the GPU.
  //////////////////////////////////////////////////////////////////////////////

  cudaMemcpy( _d_W[0].elem, W->elem, sizeof(float) * W->rows * W->cols,
              cudaMemcpyHostToDevice );
  cudaMemcpy( _d_W[5].elem, _h_dewhite.elem, sizeof(float) * W->rows * W->cols,
              cudaMemcpyHostToDevice );
  cudaMemcpy( _d_W[4].elem, _h_white.elem,   sizeof(float) * W->rows * W->cols,
              cudaMemcpyHostToDevice );
  cudaMemcpy( _d_Z[1].elem, S->elem, sizeof(float) * X->rows * X->cols,
              cudaMemcpyHostToDevice );

  // Whiten the zero-mean observations.
  CUBLAS_GEMM( _d_Z[0], _d_W[4], _d_Z[1] );

  //////////////////////////////////////////////////////////////////////////////
  // Now that the device holds the zero'd, whitened observations in _d_Z[0], and
  // the initial value for the unmixing matrix, W, in _d_W[0], we can begin
  // iterating.
  //////////////////////////////////////////////////////////////////////////////
  for (num_iter = 0; num_iter < _max_iter; num_iter++) {
    ////////////////////////////////////////////////////////////////////////////
    // Apply contrast to _d_W[x], store result in _d_W[0].
    ////////////////////////////////////////////////////////////////////////////
    _contrast( &(_d_W[0]), &(_d_Z[0]) );

    ////////////////////////////////////////////////////////////////////////////
    // Orthogonalize W.
    ////////////////////////////////////////////////////////////////////////////

    // Normalize W using the sum of the absolute value of its elements. This
    // ensures that all of its eigenvectors are less than one.
    fica_sumAbs<<< _sum_grid, _sum_block, _sum_mem >>>(
                   _d_sum, _d_W[0].elem );
    cudaThreadSynchronize();

    fica_scaleMatrix<<< _scale_grid, _scale_block >>>( _d_W[0].elem, _d_sum );
    cudaThreadSynchronize();

    do {
      // _d_W[3] = W * W'
      CUBLAS_GEMM_NT( _d_W[3], _d_W[0], _d_W[0] );

      // _d_W[0] = 1.5 * W - 0.5 * W * W' * W
      cublasSgemm( 'N', 'N', W->rows, W->rows, W->rows,
                   -0.5,
                   _d_W[3].elem, W->rows,   // _d_W[3] == W * W'
                   _d_W[0].elem, W->rows,   // _d_W[0] == W
                   1.5,
                   _d_W[0].elem, W->rows ); 

      // _d_W[2] = _d_W[0] * _d_W[0]'
      CUBLAS_GEMM_NT( _d_W[2], _d_W[0], _d_W[0] );

      cublasGetVector( W->rows, sizeof(float),
                       _d_W[2].elem, W->rows + 1,
                       _diag, 1 );
      for (i = 1; i < W->rows; i++) {
        _diag[0] += _diag[i];
      }
    } while ((float) W->rows - _diag[0] > 0.00001);
  }

  //////////////////////////////////////////////////////////////////////////////
  // We now have a best guess at an unmixing matrix. We need to finish up our
  // computations by computing the source signals, the source signal mixing
  // matrix, the unmixing matrix that will unmix the original, nonwhitened
  // observations, and the means of the source signals.
  //////////////////////////////////////////////////////////////////////////////

  // _d_W[5] holds the dewhitening matrix.
  // _d_W[4] holds the whitening matrix.
  // _d_W[0] holds the best guess at an unmixing matrix after our iterations.
  // _d_Z[0] holds the whitened, zero-mean observations.

  CUBLAS_GEMM_NT( _d_W[1], _d_W[5], _d_W[0] ); // _d_W[1] <- the mixing matrix
  CUBLAS_GEMM( _d_W[2], _d_W[0], _d_W[4] );    // _d_W[2] <- the unmixing matrix
  CUBLAS_GEMM( _d_Z[1], _d_W[0], _d_Z[0] );    // _d_Z[1] <- the source signals

  // Copy the results from the device back to the host.
  cudaMemcpy( S->elem, _d_Z[1].elem, sizeof(float) * S->rows * S->cols,
              cudaMemcpyDeviceToHost );
  cudaMemcpy( W->elem, _d_W[0].elem, sizeof(float) * W->rows * W->cols,
              cudaMemcpyDeviceToHost );
  cudaMemcpy( A->elem, _d_W[1].elem, sizeof(float) * A->rows * A->cols,
              cudaMemcpyDeviceToHost );

  GEMV( mu_S, *W, _mu_X );

  //////////////////////////////////////////////////////////////////////////////
  // And we're done!
  //////////////////////////////////////////////////////////////////////////////

  return num_iter;
}
