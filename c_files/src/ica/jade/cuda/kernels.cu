#include "ica/jade/kernels.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ jade_identity( NUMTYPE *d_M, unsigned int ld )
{
  NUMTYPE val;
  if (threadIdx.x == blockIdx.x) {
    val = 1.0f;
  } else {
    val = 0.0f;
  }

  d_M[ blockIdx.x * ld + threadIdx.x ] = val;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ jade_genCumulants( NUMTYPE *d_Q, NUMTYPE const *d_X,
                                   unsigned int q_ld, unsigned int x_ld,
                                   unsigned int num_var, unsigned int n_obs )
{
  // This kernel must be called with 256 threads per block.
  NUMTYPE __shared__ s_data[ 256 * 5 ];

  int index = blockIdx.y / num_var;

  const unsigned int var_k = blockIdx.x;
  const unsigned int var_l = blockIdx.y - index * num_var;

  const unsigned int s_prod = threadIdx.x + 4 * 256;
  const unsigned int q_idx  = blockIdx.y * q_ld + blockIdx.x;

  unsigned int var_i;
  unsigned int var_j;

  unsigned int col, t_idx;

  unsigned int s_iidx, s_jidx, s_kidx, s_lidx;
  unsigned int x_iidx, x_jidx, x_kidx, x_lidx;
  
  NUMTYPE l_prod, r_prod;

  unsigned int delta = 0;

  // Figure out the values for i and j for this matrix.
  while (index >= 0) {
    var_i = delta;
    var_j = index + delta;

    index -= (num_var - delta);

    delta++;
  }

  // Initialize the accumulators.
  s_data[s_prod] = 0.0f;

  // Read in the first groups of variable observations.
  for (col = 0; col < n_obs; col += 256) {
    // Read in values for variables i,j,k,l.
    s_iidx = threadIdx.x + 0 * 256; x_iidx = var_i * x_ld + threadIdx.x + col;
    s_jidx = threadIdx.x + 1 * 256; x_jidx = var_j * x_ld + threadIdx.x + col;
    s_kidx = threadIdx.x + 2 * 256; x_kidx = var_k * x_ld + threadIdx.x + col;
    s_lidx = threadIdx.x + 3 * 256; x_lidx = var_l * x_ld + threadIdx.x + col;

    s_data[s_iidx] = d_X[x_iidx];
    s_data[s_jidx] = d_X[x_jidx];
    s_data[s_kidx] = d_X[x_kidx];
    s_data[s_lidx] = d_X[x_lidx];

    // Calculate and accumulate the product of variables i,j,k,l. We compute the
    // product as ((i*j) * (k*l)) because i,j,k,l should be the same magnitude,
    // and this multiplication order should reduce rounding error.
    l_prod = s_data[s_iidx] * s_data[s_jidx];
    r_prod = s_data[s_kidx] * s_data[s_lidx];

    s_data[s_prod] += (l_prod * r_prod) / (NUMTYPE) n_obs;
  }
  __syncthreads();

  t_idx = threadIdx.x;

  // We now need to sum the accumulated products.
  if (t_idx < 128) { s_data[s_prod] += s_data[s_prod + 128]; }
  __syncthreads();

  if (t_idx <  64) { s_data[s_prod] += s_data[s_prod +  64]; }
  __syncthreads();

  if (t_idx <  32) {
    s_data[s_prod] += s_data[s_prod + 32];
    s_data[s_prod] += s_data[s_prod + 16];
    s_data[s_prod] += s_data[s_prod +  8];
    s_data[s_prod] += s_data[s_prod +  4];
    s_data[s_prod] += s_data[s_prod +  2];
    s_data[s_prod] += s_data[s_prod +  1];
  }

  // Record the final sum.
  if (t_idx == 0) {
    // Because of the whitening step, whenever the variable indices are equal to
    // each other, if all four are equal then the cumulant is equal to
    // E{Xi*Xi*Xi*Xi} - 3.0, otherwise, if the indices still form equal pairs,
    // the cumulant is equal to E{Xi*Xi*Xj*Xj} - 1.0. Otherwise, if at least two
    // of the indices differ, then the cumulant is simply equal to
    // E{Xi*Xj*Xk*Xl}. Before recording the calculated sum of products, we need
    // to apply any necessary DC offset.

    if (var_i == var_j && var_i == var_k && var_i == var_l) {
      d_Q[q_idx] = s_data[s_prod] - 3.0f; // i=j == k=l
    } else if (var_i == var_j && var_k == var_l) {
      d_Q[q_idx] = s_data[s_prod] - 1.0f; // i=j != k=l
    } else if (var_i == var_k && var_j == var_l) {
      d_Q[q_idx] = s_data[s_prod] - 1.0f; // i=k != j=l
    } else if (var_i == var_l && var_j == var_k) {
      d_Q[q_idx] = s_data[s_prod] - 1.0f; // i=l != j=k
    } else {
      d_Q[q_idx] = s_data[s_prod];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ jade_rightRot( NUMTYPE *d_Q, NUMTYPE *d_rot,
                               unsigned int q_ld, unsigned int rot_ld,
                               unsigned int sequence, unsigned int num_var )
{
  // This kernel must be called with (num_var - (num_var + 1) / 2) blocks
  // per cumulant matrix, for a total of:
  //    (num_var - (num_var + 1) / 2 - 1) * (num_var * (num_var + 1)) / 2
  // blocks.
  // This kernel must be called with `num_var'x2 threads per block.
  NUMTYPE extern __shared__ q_cols[];
  NUMTYPE extern __shared__ r_cols[];
  NUMTYPE new_val, cosine, sine;

  const unsigned int blks_per_mat = num_var - (num_var + 1) / 2;
  const unsigned int q_mat        = blockIdx.x / blks_per_mat;
  const unsigned int q_base       = q_mat * num_var * q_ld;

  unsigned int p, q;
  jade_getPQ( &p, &q, sequence, blockIdx.x % blks_per_mat, num_var ); 

  const unsigned int s_p = 0 * num_var + threadIdx.x;
  const unsigned int s_q = 1 * num_var + threadIdx.x;

  // Fetch the columns we're operating on. The threadIdx.y value will be either
  // 0 or 1. If it's 0, the thread fetches a value from column p. If it's 1, the
  // thread fetches a value from column q.
  const unsigned int s_idx = threadIdx.y * num_var + threadIdx.x;
  const unsigned int q_idx = q_base +
                             (1 - threadIdx.y) * (p * q_ld) +
                             (    threadIdx.y) * (q * q_ld) +
                             threadIdx.x;

  q_cols[s_idx] = d_Q[q_idx];

  // Fetch the cosine and sine values from the rotation matrix.
  cosine = d_rot[p + p * rot_ld]; // row p, column, p
  sine   = d_rot[q + p * rot_ld]; // row q, column, p
  __syncthreads();

  new_val = (1-threadIdx.y) * (cosine * q_cols[s_p] +   sine * q_cols[s_q]) +
            (  threadIdx.y) * ( -sine * q_cols[s_p] + cosine * q_cols[s_q]);

  // Write the new values back.
  d_Q[q_idx] = new_val;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __device__ jade_getPQ( unsigned int *p, unsigned int *q,
                            unsigned int sequence, unsigned int pair,
                            unsigned int num_var )
{
  const unsigned int m = (num_var + 1) / 2;
  unsigned int temp;

  // The algorithm in the above paper provides for one-based indexing, but we
  // use zero-based, so this implementation will differ slightly.
  if (sequence < m) {
    *q = m - sequence + pair;

    if      (*q <= 2*m - 2*sequence - 1) { *p = 2*m - 2*sequence - *q - 1; }
    else if (*q <= 2*m -   sequence - 2) { *p = 4*m - 2*sequence - *q - 2; }
    else                                 { *p = num_var - 1; }
  } else {
    *q = 4*m - num_var - sequence + pair - 1;

    if      (*q <  2*m -   sequence)     { *p = num_var - 1; }
    else if (*q <= 4*m - 2*sequence - 2) { *p = 4*m - 2*sequence - *q - 2; }
    else                                 { *p = 6*m - 2*sequence - *q - 3; }
  }

  // Make sure that element (p,p) is 'higher' than element (q,q).
  if (*p > *q) { temp = *p; *p = *q; *q = temp; }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ jade_anglesStepOne( NUMTYPE *d_vals, NUMTYPE const *d_Q,
                                    unsigned int q_ld,
                                    unsigned int seq, unsigned int num_var )
{
  // Called with one block per cumulant matrix and num_var/2 threads per block.
  unsigned int p, q;
  jade_getPQ( &p, &q, seq, threadIdx.x, num_var );

  const unsigned int q_mat  = blockIdx.x;
  const unsigned int q_base = q_mat * num_var * q_ld;

  const unsigned int q_ppidx = q_base + (p + p * q_ld); // row p, column p
  const unsigned int q_pqidx = q_base + (p + q * q_ld); // row p, column q
  const unsigned int q_qqidx = q_base + (q + q * q_ld); // row q, column q

  // Read in the elements (p,p), (p,q), and (q,q). Cumulant matrices are
  // symmetric, so element (p,q) equals element (q,p).
  const float q_pp = d_Q[q_ppidx];
  const float q_pq = d_Q[q_pqidx];
  const float q_qq = d_Q[q_qqidx];

  // Calculate the values from this cumulant matrix that contribute to the
  // rotation angle.
  const float diag_diff = q_pp - q_qq;
  const float off_diag  = 2.0f * q_pq;

  const float diag_sqr = diag_diff * diag_diff;
  const float off_sqr  = off_diag  * off_diag;
  const float cross    = off_diag  * diag_diff;

  // Record the computed values so that they can be summed by the next kernel.
  const unsigned int val_idx = threadIdx.x + blockIdx.x * blockDim.x;
  const unsigned int row_len = blockDim.x * gridDim.x;

  d_vals[ val_idx + row_len * 0 ] = diag_sqr;
  d_vals[ val_idx + row_len * 1 ] = off_sqr;
  d_vals[ val_idx + row_len * 2 ] = cross;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void __global__ jade_anglesStepTwo( NUMTYPE *d_rot, NUMTYPE *d_vals,
                                    unsigned int rot_ld,
                                    unsigned int seq, unsigned int num_var )
{
  // Called with one block of 3 * (num_var / 2) threads and shared memory for
  // 3*(num_var/2) elements.

  NUMTYPE extern __shared__ sums[];

  const unsigned int num_cm   = (num_var * (num_var + 1)) / 2;
  const unsigned int val_base = (threadIdx.x/(num_var/2)) *
                                (num_var/2) * num_cm;
  const unsigned int pair     = threadIdx.x % (num_var / 2);

  // Start summing values.
  const unsigned int sum_idx = threadIdx.x;
  unsigned int i;

  sums[sum_idx] = 0.0f;
  NUMTYPE val;
  for (i = 0; i < num_cm; i++) {
    val = d_vals[ val_base + pair + i * (num_var/2) ];
    sums[sum_idx] += val;
  }
  __syncthreads();

  // Compute the final values used in the angle computations.
  NUMTYPE sqr_diff, cross_sum, theta, cosine, sine;
  unsigned int p,q;

  if (threadIdx.x < (num_var/2)) {
    sqr_diff  = sums[sum_idx] - sums[sum_idx + (num_var/2)];
    cross_sum = sums[sum_idx + (num_var/2) * 2] * 2.0f;

    theta = 0.5f * atan2( cross_sum, sqr_diff +
                          sqrt(sqr_diff * sqr_diff + cross_sum * cross_sum) );
    cosine = cos(theta);
    sine   = sin(theta);

    jade_getPQ( &p, &q, seq, pair, num_var );

    d_rot[ p + p * rot_ld ] = cosine; // row p, column p
    d_rot[ p + q * rot_ld ] = -sine;  // row p, column q
    d_rot[ q + p * rot_ld ] = sine;   // row q, column p
    d_rot[ q + q * rot_ld ] = cosine; // row q, column q
  }
}
