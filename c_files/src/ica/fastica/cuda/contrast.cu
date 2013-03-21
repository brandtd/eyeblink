#include "ica/fastica/contrast.h"

#include "ica/fastica/kernels.h"

#include <stdio.h>
#include <cublas.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void gpu_negent_tanh( Matrix *d_W, Matrix *d_Z )
{
  // Device workspace matrices.
  Matrix d_WS[2];
  d_WS[0].rows = d_Z->rows;
  d_WS[0].cols = d_Z->cols;
  d_WS[0].ld   = d_Z->rows;
  d_WS[0].lag  = d_Z->lag;

  d_WS[1].rows = d_W->rows;
  d_WS[1].cols = d_W->cols;
  d_WS[1].ld   = d_W->rows;
  d_WS[1].lag  = d_W->lag;

  // Allocate memory for the workspace matrices.
  cudaMalloc( (void**) &(d_WS[0].elem),
              sizeof(float) * d_Z->ld * d_Z->lag );
  cudaMalloc( (void**) &(d_WS[1].elem),
              sizeof(float) * d_W->ld * d_W->lag );

  // Make sure the d_WS[0] matrix has been zero-padded.
  cudaMemset( d_WS[0].elem, 0x00, sizeof(float) * d_Z->ld * d_Z->lag );

  // d_WS[0] = W * Z
  CUBLAS_GEMM( d_WS[0], *d_W, *d_Z );

  // Calculate the tanh(.) of each element in the workspace, using one block
  // per column.
  // d_WS[0] = tanh( W * Z )
  dim3 dim_grid( d_WS[0].cols );
  dim3 dim_block( 1, d_WS[0].rows );
  fica_tanh<<< dim_grid, dim_block >>>( d_WS[0].elem, d_WS[0].rows );
  cudaThreadSynchronize();

  // d_WS[1] = tanh( W * Z ) * Z'
  CUBLAS_GEMM_NT( d_WS[1], d_WS[0], *d_Z );

  // Calculate (1 - tanh^2( W * Z )) and sum the rows of the resulting matrix.
  dim_grid   = dim3( 1, d_WS[0].rows );
  dim_block  = dim3( 256, 1 );
  fica_tanhDer<<< dim_grid, dim_block >>>( d_WS[0].elem, d_WS[0].ld,
                                           d_WS[0].cols );
  cudaThreadSynchronize();

  // Finish the application of the learning rule.
  dim_grid  = dim3( d_W->cols );
  dim_block = dim3( 1, d_W->rows );
  fica_wnext<<< dim_grid, dim_block >>>( d_W->elem, d_WS[1].elem, d_WS[0].elem,
                                         d_W->ld, d_Z->cols );
  cudaThreadSynchronize();

  cudaFree( d_WS[0].elem );
  cudaFree( d_WS[1].elem );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void gpu_negent_cube( Matrix *d_W, Matrix *d_Z )
{
  Matrix d_WS;
  d_WS.rows = d_Z->rows;
  d_WS.cols = d_Z->cols;
  d_WS.ld   = d_Z->rows;
  d_WS.lag  = d_Z->lag;

  float *d_wsum;

  cudaMalloc( (void**) &(d_WS.elem),
              sizeof(float) * d_Z->ld * d_Z->lag );
  cudaMalloc( (void**) &d_wsum, sizeof(float) * d_Z->rows );

  cudaMemset( d_WS.elem, 0, sizeof(float) * d_Z->ld * d_Z->lag );

  CUBLAS_GEMM( d_WS, *d_W, *d_Z );

  dim3 grid( 1, d_WS.rows );
  dim3 block( 256 );
  fica_cubeRule<<< grid, block >>>( d_wsum, d_WS.elem, d_WS.ld,
                                    d_WS.cols );
  cudaThreadSynchronize();

  CUBLAS_GEMM_NT( d_WS, d_WS, *d_Z );

  grid = dim3( 1, d_WS.rows );
  block = dim3( 256 );
  fica_wnext<<< grid, block >>>( d_W->elem, d_WS.elem, d_wsum, d_W->ld,
                                 d_Z->cols );
  cudaThreadSynchronize();

  cudaFree( d_WS.elem );
  cudaFree( d_wsum );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void gpu_negent_gauss( Matrix *d_W, Matrix *d_Z )
{
  Matrix d_WS;
  d_WS.rows = d_Z->rows;
  d_WS.cols = d_Z->cols;
  d_WS.ld   = d_Z->rows;
  d_WS.lag  = d_Z->lag;

  float *d_wsum;

  cudaMalloc( (void**) &(d_WS.elem),
              sizeof(float) * d_Z->ld * d_Z->lag );
  cudaMalloc( (void**) &d_wsum, sizeof(float) * d_Z->rows );

  cudaMemset( d_WS.elem, 0, sizeof(float) * d_Z->ld * d_Z->lag );

  CUBLAS_GEMM( d_WS, *d_W, *d_Z );

  dim3 grid( 1, d_WS.rows );
  dim3 block( 256 );
  fica_gaussRule<<< grid, block >>>( d_wsum, d_WS.elem, d_WS.ld,
                                     d_WS.cols );
  cudaThreadSynchronize();

  CUBLAS_GEMM_NT( d_WS, d_WS, *d_Z );

  grid = dim3( 1, d_WS.rows );
  block = dim3( 256 );
  fica_wnext<<< grid, block >>>( d_W->elem, d_WS.elem, d_wsum, d_W->ld,
                                 d_Z->cols );
  cudaThreadSynchronize();

  cudaFree( d_WS.elem );
  cudaFree( d_wsum );
}
