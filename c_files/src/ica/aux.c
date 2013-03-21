#include "ica/aux.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void remmean( NUMTYPE *means, Matrix *Z, Matrix const *X )
{
  unsigned int row, col, i;

  // Find the mean of each row of the observation matrix.
  memset( means, 0, sizeof(NUMTYPE) * X->rows );
  for (col = 0; col < X->cols; col++) {
    for (row = 0; row < X->rows; row++) {
      means[row] += X->elem[col*X->ld + row];
    }
  }

  for (row = 0; row < X->rows; row++) {
    means[row] /= X->cols;
  }

  // Remove the row's mean from each element in the row.
  for (col = 0; col < X->cols; col++) {
    for (row = 0; row < X->rows; row++) {
      i = col*X->ld + row;
      Z->elem[i] = X->elem[i] - means[row];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void remmeanTranspose( NUMTYPE *means, Matrix *Z, Matrix const *X )
{
  unsigned int row, col, z_i, x_i;

  // Find the mean of each row of the observation matrix.
  memset( means, 0, sizeof(NUMTYPE) * X->rows );
  for (col = 0; col < X->cols; col++) {
    for (row = 0; row < X->rows; row++) {
      means[row] += X->elem[col*X->ld + row];
    }
  }

  for (row = 0; row < X->rows; row++) {
    means[row] /= X->cols;
  }

  // Remove the row's mean from each element in the row.
  for (col = 0; col < X->cols; col++) {
    for (row = 0; row < X->rows; row++) {
      x_i = col*X->ld + row;
      z_i = row*Z->ld + col;
      Z->elem[z_i] = X->elem[x_i] - means[row];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void whiten( Matrix *white_Z, Matrix *whiten, Matrix *dewhiten,
             Matrix const *Z, int transpose )
{
  computeWhiten( whiten, dewhiten, Z, transpose );

  // Whiten the zero-mean data using the whitening matrix.
  if (transpose) {
    GEMM_NT( *white_Z, *whiten, *Z );
  } else {
    GEMM( *white_Z, *whiten, *Z );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void computeWhiten( Matrix *whiten, Matrix *dewhiten, Matrix const *Z,
                    int transpose )
{
  unsigned int row, col, i;
  NUMTYPE eig_inv_sqr, eig_sqr, *eig_vals;

  if (transpose) {
    eig_vals = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->rows );
  } else {
    eig_vals = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Z->cols );
  }

  // To make observations white, we find the eigenvalue decomposition of the
  // zero-mean observations' covariance matrix. This lets us compute whitening
  // and dewhitening matrices. The whitening matrix will convert our zero-mean
  // observations into uncorrelated, unit variance variables. The dewhitening
  // matrix will be useful for calculating the inverse of the unmixing matrix
  // later on.
  //
  // The whitening and dewhitening matrices are equal to:
  //    White   = D^(-1/2) * E'
  //    Dewhite = E * D^(1/2)
  // where D is the diagonal matrix of eigenvalues and E' is the transpose of
  // the eigenvector matrix, both coming from the covariance matrix of the zero-
  // mean observations.
  if (transpose) {
    COVARIANCE_T( *dewhiten, *Z );
  } else {
    COVARIANCE( *dewhiten, *Z );
  }
  SYEV( *dewhiten, eig_vals );

  for (col = 0; col < whiten->cols; col++) {
    eig_inv_sqr = 1.0 / sqrt( eig_vals[col] );
    eig_sqr     = sqrt( eig_vals[col] );

    for (row = 0; row < whiten->rows; row++) {
      i = col * dewhiten->ld + row;
      whiten->elem[row*whiten->ld + col] = eig_inv_sqr * dewhiten->elem[i];
      dewhiten->elem[i]                  = eig_sqr     * dewhiten->elem[i];
    }
  }

  free( eig_vals );
}
