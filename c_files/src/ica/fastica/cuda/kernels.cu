#include "ica/fastica/kernels.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_sumAbs( float *d_sum, float *d_X )
{
  // Based on the "Optimizing Parallel Reduction in CUDA" paper by Mark Harris.
  extern __shared__ float s_data[];

  unsigned int row = threadIdx.y;
  unsigned int col = 0;

  float sum = 0.0f;

  #pragma unroll
  for (col = 0; col < blockDim.y; col++) {
    sum += fabsf( d_X[ col * blockDim.y + row ] );
  }

  s_data[row] = sum;
  __syncthreads();

  if (blockDim.y >= 256) { if (row < 128) { s_data[row] += s_data[row + 128]; }}
  __syncthreads();

  if (blockDim.y >= 128) { if (row <  64) { s_data[row] += s_data[row +  64]; }}
  __syncthreads();

  if (blockDim.y >=  64) { if (row <  32) { s_data[row] += s_data[row +  32]; }}
  if (blockDim.y >=  32) { if (row <  16) { s_data[row] += s_data[row +  16]; }}
  if (blockDim.y >=  16) { if (row <   8) { s_data[row] += s_data[row +   8]; }}

  if (row < 8) {
    s_data[row] += s_data[row + 4];
    s_data[row] += s_data[row + 2];
    s_data[row] += s_data[row + 1];
  }

  if (row == 0) {
    *d_sum = s_data[0];
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_scaleMatrix( float *d_X, float *alpha )
{
  unsigned int idx = blockIdx.x * blockDim.x + blockIdx.y * blockDim.y +
                     threadIdx.y + threadIdx.x;
  d_X[idx] = d_X[idx] / alpha[0];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_tanh( float *d_ws, int ld )
{
  int idx = blockIdx.x * ld + threadIdx.y;
  d_ws[idx] = tanh( d_ws[idx] ); 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_tanhDer(float *d_ws, unsigned int ld, unsigned int n_cols)
{
  // Based on the "Optimizing Parallel Reduction in CUDA" paper by Mark Harris.
  float __shared__ s_data[256];

  const unsigned int tid = threadIdx.x;
  const unsigned int row = blockIdx.y;
  unsigned int i = tid;
  float accum;
  s_data[tid] = 0;

  // The big difference between this code and Mark Harris' code, is the change
  // from a simple summation, to the summing of (1 - x^2) for every element
  // 'x' in a row.
  while (i < n_cols) {
    accum = d_ws[ i * ld + row ];
    accum = 1.0f - accum * accum;
    s_data[tid] += accum;

    i += 256;
  }
  __syncthreads();

  if (tid < 128) {
    s_data[tid] += s_data[tid + 128];
  }
  __syncthreads();

  if (tid < 64) {
    s_data[tid] += s_data[tid + 64];
  }
  __syncthreads();

  if (tid < 32) {
    s_data[tid] += s_data[tid + 32];
    s_data[tid] += s_data[tid + 16];
    s_data[tid] += s_data[tid +  8];
    s_data[tid] += s_data[tid +  4];
    s_data[tid] += s_data[tid +  2];
    s_data[tid] += s_data[tid +  1];
  }

  if (tid == 0) {
    d_ws[ row ] = s_data[0];
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_cubeRule( float *d_wsum, float *d_ws,
                               unsigned int ld, unsigned int n_cols )
{
  float __shared__ s_data[256];

  const unsigned int tid = threadIdx.x;
  const unsigned int row = blockIdx.y;
  unsigned int i = tid;
  float val;

  s_data[tid] = 0.0f;

  while (i < n_cols) {
    val = d_ws[ i * ld + row ];
    s_data[tid] += 3.0f * val * val;
    d_ws[ i * ld + row ] = val * val * val;

    i += 256;
  }
  __syncthreads();

  if (tid < 128) {
    s_data[tid] += s_data[tid + 128];
  }
  __syncthreads();

  if (tid < 64) {
    s_data[tid] += s_data[tid + 64];
  }
  __syncthreads();

  if (tid < 32) {
    s_data[tid] += s_data[tid + 32];
    s_data[tid] += s_data[tid + 16];
    s_data[tid] += s_data[tid +  8];
    s_data[tid] += s_data[tid +  4];
    s_data[tid] += s_data[tid +  2];
    s_data[tid] += s_data[tid +  1];
  }

  if (tid == 0) {
    d_wsum[row] = s_data[0];
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_gaussRule( float *d_wsum, float *d_ws,
                                unsigned int ld, unsigned int n_cols )
{
  float __shared__ s_data[256];

  const unsigned int tid = threadIdx.x;
  const unsigned int row = blockIdx.y;
  unsigned int i = tid;
  unsigned int ws_idx;
  float wz, sqr, expo;

  s_data[tid] = 0.0f;

  while (i < n_cols) {
    ws_idx = i * ld + row;

    wz = d_ws[ ws_idx ];
    sqr = wz * wz;
    expo = exp( -sqr / 2.0f );

    s_data[tid] += (1.0f - sqr) * expo;
    d_ws[ ws_idx ] = wz * expo;

    i += 256;
  }
  __syncthreads();

  if (tid < 128) {
    s_data[tid] += s_data[tid + 128];
  }
  __syncthreads();

  if (tid < 64) {
    s_data[tid] += s_data[tid + 64];
  }
  __syncthreads();

  if (tid < 32) {
    s_data[tid] += s_data[tid + 32];
    s_data[tid] += s_data[tid + 16];
    s_data[tid] += s_data[tid +  8];
    s_data[tid] += s_data[tid +  4];
    s_data[tid] += s_data[tid +  2];
    s_data[tid] += s_data[tid +  1];
  }

  if (tid == 0) {
    d_wsum[row] = s_data[0];
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ fica_wnext( float *d_w, float *d_wx, float *d_sums,
                            unsigned int ld, unsigned int n_cols )
{
  unsigned int col = blockDim.x * blockIdx.x + threadIdx.x;
  unsigned int row = blockDim.y * blockIdx.y + threadIdx.y;
  unsigned int idx = col * ld + row;

  d_w[idx] = (d_wx[idx] - d_sums[row] * d_w[idx]) / (float) n_cols;
}
