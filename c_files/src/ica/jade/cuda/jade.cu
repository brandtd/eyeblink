#include "ica/ica.h"
#include "ica/aux.h"
#include "ica/setup.h"
#include "ica/jade/kernels.h"

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

// The number of variables in the X matrix, the number of cumulant matrices
// that we will generate, the number of elements in a cumulant matrix, and the
// `m' parameter for the rotation matrix generation kernel.
static unsigned int _num_var = 0;
static unsigned int _num_cm = 0;
static unsigned int _num_elem = 0;
static unsigned int _m_param = 0;

// The size (in bytes) of a cumulant matrix.
static size_t _mat_size = 0;

// Device memory pointers.
static Matrix _d_Q = {0}, _d_Z[2] = {0}, _d_W[4] = {0};
static NUMTYPE *_d_vals = NULL;

// Host memory pointers.
static Matrix _h_white = {0}, _h_dewhite = {0}, _h_Z = {0};
static NUMTYPE *_h_Q = NULL, *_mu_X = NULL;

// Dimensions used by CUDA kernel functions.
static dim3 _grid_size(0), _block_size(0), _s1_grid(0), _s1_block(0);
static dim3 _rot_grid(0),  _rot_block(0),  _s2_grid(0), _s2_block(0);
static size_t _s2_mem = 0, _rot_mem = 0;

// Which GPU device to use.
static int _device = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int jade_gpuInit( ICAParams *params )
{
  unsigned int i, size_diff, padded_X_size;

  _num_var  = params->num_var;
  _num_cm   = (_num_var * (_num_var + 1)) / 2;
  _num_elem = _num_var * _num_var;
  _m_param  = _num_var - (_num_var + 1) / 2;

  _mat_size = sizeof(NUMTYPE) * _num_elem;

  // Used to form cumulant matrices.
  _grid_size  = dim3( _num_var, _num_var * _num_cm, 1 );
  _block_size = dim3( 256 );

  // Used to create rotation matrix.
  _s1_grid = dim3( _num_cm, 1, 1 ); _s1_block = dim3( _num_var / 2, 1, 1 );

  // Used to create rotation matrix.
  _s2_grid = dim3( 1, 1, 1 ); _s2_block = dim3( 3 * (_num_var/2), 1, 1 );
  _s2_mem  = 3 * (_num_var/2) * sizeof(NUMTYPE);

  // Used to rotate cumulant matrices.
  _rot_grid = dim3( _m_param * _num_cm ); _rot_block = dim3( _num_var, 2 );
  _rot_mem  = 4 * _num_var * sizeof(NUMTYPE);

  // Initialize CUDA.
  _device = params->gpu_device;
  cudaSetDevice( _device );

  //////////////////////////////////////////////////////////////////////////////
  // Allocate device memory.
  //////////////////////////////////////////////////////////////////////////////

  // Three values, per cumulant matrix, per pair of rows/columns.
  cudaMalloc((void**) &_d_vals, 3 * _num_cm * (_num_var / 2) * sizeof(NUMTYPE));

  cudaMalloc((void**) &(_d_Q.elem), _mat_size * _num_cm);
  _d_Q.ld  = _d_Q.rows = _num_var;
  _d_Q.lag = _d_Q.cols = _num_cm * _num_var;

  // We will pad the X matrix with zero rows so that the length of X is a
  // multiple of 256. This will give our CUDA kernels an easier time. We will
  // also be storing the transpose of X on the GPU, as this will allow for
  // memory coalescing later on.
  size_diff = 256 - (params->num_obs % 256);
  if (size_diff == 256) { size_diff = 0; }
  padded_X_size = _num_var * (params->num_obs + size_diff) * sizeof(NUMTYPE);

  for (i = 0; i < sizeof(_d_Z) / sizeof(Matrix); i++) {
    cudaMalloc( (void**) &(_d_Z[i].elem), padded_X_size );

    _d_Z[i].cols = _d_Z[i].lag = _num_var;
    _d_Z[i].rows = params->num_obs;
    _d_Z[i].ld   = params->num_obs + size_diff;

    cudaMemset( _d_Z[i].elem, 0, padded_X_size );
  }

  for (i = 0; i < sizeof(_d_W) / sizeof(Matrix); i++) {
    cudaMalloc((void**)&(_d_W[i].elem), sizeof(NUMTYPE) * _num_var * _num_var );
    _d_W[i].rows = _d_W[i].ld  = _num_var;
    _d_W[i].cols = _d_W[i].lag = _num_var;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Allocate host memory and initialize matrices.
  //////////////////////////////////////////////////////////////////////////////
  _h_dewhite.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * _num_var * _num_var );
  _h_white.elem   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * _num_var * _num_var );
  _h_Z.elem       = (NUMTYPE*) malloc( sizeof(NUMTYPE)*_d_Z[0].ld*_d_Z[0].lag);

  _h_dewhite.lag  = _h_white.lag  = _num_var;
  _h_dewhite.cols = _h_white.cols = _num_var;
  _h_dewhite.rows = _h_white.rows = _num_var;
  _h_dewhite.ld   = _h_white.ld   = _num_var;

  _h_Z.lag  = _d_Z[0].lag;
  _h_Z.cols = _d_Z[0].cols;
  _h_Z.rows = _d_Z[0].rows;
  _h_Z.ld   = _d_Z[0].ld;

  memset( _h_Z.elem, 0, _h_Z.ld * _h_Z.lag * sizeof(NUMTYPE) );

  _h_Q  = (NUMTYPE*) malloc( _mat_size * _num_cm );
  _mu_X = (NUMTYPE*) malloc( sizeof(NUMTYPE) * _num_var );

  // Return that everything went OK.
  // TODO: check for CUDA errors.
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void jade_gpuShutdown()
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

  cudaFree( _d_Q.elem );
  _d_Q.elem = NULL;
  _d_Q.ld = _d_Q.lag = _d_Q.rows = _d_Q.cols = 0;

  cudaFree( _d_vals );
  _d_vals = NULL;

  free( _h_dewhite.elem ); free( _h_white.elem ); free( _h_Q ); free( _mu_X );
  _h_dewhite.elem = _h_white.elem = _h_Q = _mu_X = NULL;
  _h_dewhite.ld = _h_dewhite.lag = _h_dewhite.rows = _h_dewhite.cols = 0;
  _h_white.ld   = _h_white.lag   = _h_white.rows   = _h_white.cols = 0;

  _grid_size = _block_size = _s1_grid = _s1_block = dim3(0);
  _rot_grid  = _rot_block  = _s2_grid = _s2_block = dim3(0);
  _s2_mem = _rot_mem = 0;

  _device = 0;

  cublasShutdown();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int jade_gpu( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                       Matrix const *X )
{
  // The number of variables could probably be cut down, but we leave that to
  // the compiler. Readibility is the more important goal.
  unsigned int i, row, col, s_i, z_i, sweeps;

  //////////////////////////////////////////////////////////////////////////////
  // Make the observations zero-mean, remembering the observation means, so that
  // we can calculate the source signal means later.
  //////////////////////////////////////////////////////////////////////////////

  remmeanTranspose( _mu_X, &_h_Z, X );

  //////////////////////////////////////////////////////////////////////////////
  // Find the whitening/dewhitening matrices.
  //////////////////////////////////////////////////////////////////////////////

  // We use the CPU for this because we don't have a convenient method for
  // getting the eigenvalue decomposition using the GPU.
  computeWhiten( &_h_white, &_h_dewhite, &_h_Z, 1 );

  //////////////////////////////////////////////////////////////////////////////
  // Copy the things we've calculated so far to the GPU. Almost all of the rest
  // of the ICA algorithm will now take place using the GPU.
  //////////////////////////////////////////////////////////////////////////////

  cudaMemcpy( _d_W[0].elem, _h_dewhite.elem, sizeof(NUMTYPE) * W->rows * W->cols,
              cudaMemcpyHostToDevice );
  cudaMemcpy( _d_W[1].elem, _h_white.elem,   sizeof(NUMTYPE) * W->rows * W->cols,
              cudaMemcpyHostToDevice );
  cudaMemcpy( _d_Z[0].elem, _h_Z.elem, sizeof(NUMTYPE) * _h_Z.ld * _h_Z.lag,
              cudaMemcpyHostToDevice );

  // Whiten the zero-mean observations, remembering that the _d_Z matrices are
  // stored as their transpositions, i.e. we want Y = W * X, but both Y and X
  // must be stored as their transposes, so we end up calculating Y' = X' * W'
  CUBLAS_GEMM_NT( _d_Z[1], _d_Z[0], _d_W[1] );

  //////////////////////////////////////////////////////////////////////////////
  // Form the cumulant matrices.
  //////////////////////////////////////////////////////////////////////////////
  jade_genCumulants<<< _grid_size, _block_size >>>( _d_Q.elem,
                                                  _d_Z[1].elem,
                                                  _num_var,
                                                  _d_Z[1].ld,
                                                  _d_Z[1].cols,
                                                  _d_Z[1].rows );

  // Initialize the accumulated rotation matrix to identity.
  jade_identity<<< _num_var, _num_var >>>( _d_W[3].elem, _d_W[3].ld );

  cudaThreadSynchronize();

  //////////////////////////////////////////////////////////////////////////////
  // Begin performing Jacobi sweeps in an attempt to diagonalize all cumulant
  // matrices simulaneously.
  //////////////////////////////////////////////////////////////////////////////
  for (sweeps = 0; sweeps < 100; sweeps++) {
    for (i = 1; i < 2 * ((_num_var + 1) / 2); i++) {
      // Reset the matrix we use to store rotation values during each sweep.
      jade_identity<<< _num_var, _num_var >>>( _d_W[2].elem, _d_W[2].ld );
      cudaThreadSynchronize();

      // Calculate the rotation matrix to apply.
      jade_anglesStepOne<<< _s1_grid, _s1_block >>>( _d_vals, _d_Q.elem, _d_Q.ld,
                                                   i, _num_var );
      cudaThreadSynchronize();

      jade_anglesStepTwo<<< _s2_grid, _s2_block, _s2_mem >>>(
                            _d_W[2].elem, _d_vals, _d_W[2].ld, i, _num_var );
      cudaThreadSynchronize();

      // First, left multiple the rotation matrix to every cumulant matrix. We
      // can use the CUBLAS function for this.
      CUBLAS_GEMM_TN( _d_Q, _d_W[2], _d_Q );
      cudaThreadSynchronize();

      // Next, right multiple the rotation matrix to every cumulant matrix.
      jade_rightRot<<< _rot_grid, _rot_block, _rot_mem >>>( _d_Q.elem,
                                                         _d_W[2].elem,
                                                         _d_Q.ld,
                                                         _d_W[2].ld,
                                                         i, _num_var );
      // While that's going on, accumulate the rotation matrix.
      jade_rightRot<<< _m_param, _rot_block, _rot_mem >>>(
                                                      _d_W[3].elem, _d_W[2].elem,
                                                      _d_W[3].ld,   _d_W[2].ld,
                                                      i, _num_var );
      cudaThreadSynchronize();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // We now have a best guess at an unmixing matrix. We need to finish up our
  // computations by computing the source signals, the source signal mixing
  // matrix, the unmixing matrix that will unmix the original, nonwhitened
  // observations, and the means of the source signals.
  //////////////////////////////////////////////////////////////////////////////

  // _d_W[3] holds the accumulated rotation matrix.
  // _d_W[1] holds the whitening matrix.
  // _d_W[0] holds the dewhitening matrix.
  // _d_Z[1] holds the whitened, zero-mean observations (transposed).

  CUBLAS_GEMM( _d_Z[0], _d_Z[1], _d_W[3] );    // _d_Z[0] <- the transposed sources
  CUBLAS_GEMM_TN( _d_W[2], _d_W[3], _d_W[1] ); // _d_W[2] <- the unmixing matrix
  CUBLAS_GEMM( _d_W[1], _d_W[0], _d_W[3] );    // _d_W[1] <- the mixing matrix

  // Copy the results from the device back to the host.
  cudaMemcpy( _h_Z.elem, _d_Z[0].elem, sizeof(NUMTYPE) * _h_Z.ld * _h_Z.lag,
              cudaMemcpyDeviceToHost );
  cudaMemcpy( W->elem, _d_W[2].elem, sizeof(NUMTYPE) * W->rows * W->cols,
              cudaMemcpyDeviceToHost );
  cudaMemcpy( A->elem, _d_W[1].elem, sizeof(NUMTYPE) * A->rows * A->cols,
              cudaMemcpyDeviceToHost );

  // Transpose the calculated source signals so they fit in the S matrix.
  for (col = 0; col < S->cols; col++) {
    for( row = 0; row < S->rows; row++) {
      s_i = col * S->ld + row;
      z_i = row * _h_Z.ld + col;
      S->elem[s_i] = _h_Z.elem[z_i];
    }
  }

  // Find the source signal means.
  GEMV( mu_S, *W, _mu_X );

  //////////////////////////////////////////////////////////////////////////////
  // We're done!
  //////////////////////////////////////////////////////////////////////////////

  return sweeps;
}
