#include "ica/ica.h"
#include "ica/aux.h"
#include "ica/setup.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Global variables setup in our initialization function. Setup of these
 * variables is an overhead that we shouldn't have to incur for every run of
 * the fastica() computation, since the values for these variables is dependent
 * on the ICA configuration parameters and nothing else.
 */

// The number of variables in the observations, the number of cumulant matrices
// that we will generate, and the number of elements in a cumulant matrix.
static unsigned int _num_var = 0;
static unsigned int _num_cm = 0;
static unsigned int _num_elem = 0;

// Size (in bytes) of a cumulant matrix.
static size_t _mat_size = 0;

// Convenience variable used to help find mean values.
static NUMTYPE _scale = 0.0;

// Where we will store the cumulant matrices.
static NUMTYPE *_cm_mat = NULL;

// Minimum rotation angle. If we calculate an angle below this, we don't perform
// the rotation.
static NUMTYPE _threshold = 0.0;

// Temporary workspace matrices that will be used throughout.
static Matrix _t[6];
#define MAT_Z         _t[1] // Matrix for zero-mean, whitened observations.
#define MAT_TEMP      _t[2] // Matrix for temporary workspace matrix.
#define MAT_WHITEN    _t[3] // Matrix for whitening matrix.
#define MAT_DEWHITEN  _t[4] // Matrix for dewhitening matrix.
#define MAT_V         _t[5] // Matrix for rotation matrix.

// Storage for the means of observed variables.
static NUMTYPE *_mu_X = NULL;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int jade_init( ICAParams *params )
{
  int i;

  _num_var  = params->num_var;
  _num_cm   = (_num_var * (_num_var + 1)) / 2;
  _num_elem = _num_var * _num_var;

  _mat_size = sizeof(NUMTYPE) * _num_elem;

  _scale = 1.0 / (NUMTYPE) params->num_obs;

  _threshold = (1.0 / sqrt((NUMTYPE) params->num_obs)) / 100.0;

  //////////////////////////////////////////////////////////////////////////////
  // Allocate memory and initialize matrices.
  //////////////////////////////////////////////////////////////////////////////
  _cm_mat = (NUMTYPE*) malloc( _mat_size * _num_cm );
  _mu_X   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * _num_var );

  // _t[0] will be used to iterate through the cumulant matrices in _cm_mat.
  _t[0].elem = _cm_mat;
  _t[0].rows = _t[0].cols = _t[0].ld = _t[0].lag = _num_var;

  // _t[1] will store the zero-meaned, whitened observations, _t[2] will be used
  // to help calculate the cumulant matrices.
  for (i = 1; i <= 2; i++) {
    _t[i].elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) *
                                    _num_var * params->num_obs );
    _t[i].rows = _t[i].ld  = _num_var;
    _t[i].cols = _t[i].lag = params->num_obs;
  }

  // _t[3] will store the whitening matrix, _t[4] will hold the dewhitening
  // matrix, and _t[5] will hold the rotation matrix.
  for (i = 3; i <= 5; i++) {
    _t[i].elem = (NUMTYPE*) malloc( _mat_size );
    _t[i].rows = _t[i].cols = _t[i].ld = _t[i].lag = _num_var;
  }

  // Return that everything went OK.
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void jade_shutdown()
{
  int i;

  //////////////////////////////////////////////////////////////////////////////
  // Free all allocated memory and set everything to NULL/0.
  //////////////////////////////////////////////////////////////////////////////
  free( _cm_mat ); _cm_mat = NULL;

  free( _mu_X ); _mu_X = NULL;

  for (i = 1; i < 6; i++) {
    free( _t[i].elem );
    _t[i].elem = NULL;
    _t[i].ld = _t[i].lag = _t[i].rows = _t[i].cols = 0;
  }

  _num_var = _num_cm = _num_elem = _mat_size = 0;
  _scale = _threshold = 0.0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int jade( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                   Matrix const *X )
{
  // Indexing variables.
  unsigned int i, var_i, var_j, var, var2, row, col, sweeps, modified;
  unsigned int p_i, q_i, pp_i, qq_i, pq_i, p, q, cm;

  // Variables used in the calculation of the Jacobi rotation.
  NUMTYPE on_diag, off_diag, GG_x, GG_z, GG_y, theta, cosine, sine, tmp1, tmp2;
  NUMTYPE cos_sqr, sin_sqr;

  //////////////////////////////////////////////////////////////////////////////
  // Initialize the rotation matrix to the identity matrix.
  //////////////////////////////////////////////////////////////////////////////
  memset( MAT_V.elem, 0, _mat_size );
  for (i = 0; i < _num_var; i++) {
    MAT_V.elem[i * _num_var + i] = 1.0;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Make the observations zero-mean, remembering the observation means, so that
  // we can calculate the source signal means later, and using the S matrix
  // parameter to temporarily hold the zero-mean observations.
  //////////////////////////////////////////////////////////////////////////////

  remmean( _mu_X, S, X );

  //////////////////////////////////////////////////////////////////////////////
  // Make the zero-mean observations white.
  //////////////////////////////////////////////////////////////////////////////

  whiten( &(MAT_Z), &(MAT_WHITEN), &(MAT_DEWHITEN), S, 0 );
  // MAT_Z        -> the zero-mean, white observations
  // MAT_WHITEN   -> the whitening matrix
  // MAT_DEWHITEN -> the dewhitening matrix

  //////////////////////////////////////////////////////////////////////////////
  // Form the cumulant matrices.
  //////////////////////////////////////////////////////////////////////////////
  _t[0].elem = _cm_mat;
  for (var = 0; var < _num_var; var++) {
    // Generate the cumulant matrix of the form Qiikl.
    // i <- var, k <- row, l <- col
    for (col = 0; col < X->cols; col++) {
      for (row = 0; row < X->rows; row++) {
        i     = col * _num_var + row;
        var_i = col * _num_var + var;

        MAT_TEMP.elem[i] = (_scale *
                            (MAT_Z.elem[var_i] *
                             MAT_Z.elem[var_i])) *
                           MAT_Z.elem[i];
      }
    }

    GEMM_NT( _t[0], MAT_TEMP, MAT_Z );

    for (i = 0; i < _num_var; i++) {
      var_i = i * _num_var + i;

      if (i == var) {
        _t[0].elem[var_i] -= 3.0;
      } else {
        _t[0].elem[var_i] -= 1.0;
      }
    }

    _t[0].elem += _num_elem;

    // Generate cumulant matrices of the form Qijkl.
    // i <- var, j <- var2, k <- row, l <- col
    for (var2 = 0; var2 < var; var2++) {
      for (col = 0; col < X->cols; col++) {
        for (row = 0; row < X->rows; row++) {
          i     = col * _num_var + row;
          var_i = col * _num_var + var;
          var_j = col * _num_var + var2;

          MAT_TEMP.elem[i] = (_scale *
                              (MAT_Z.elem[var_i] *
                               MAT_Z.elem[var_j])) *
                             MAT_Z.elem[i];
        }
      }

      GEMM_NT( _t[0], MAT_TEMP, MAT_Z );

      _t[0].elem[ var * _num_var + var2 ] -= 1.0;
      _t[0].elem[ var2 * _num_var + var ] -= 1.0;

      _t[0].elem += _num_elem;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Begin performing Jacobi sweeps in an attempt to diagonalize all cumulant
  // matrices simulaneously.
  //////////////////////////////////////////////////////////////////////////////
  sweeps   = 0;
  modified = 1;
  while (sweeps < 100 && modified) {
    modified = 0;
    sweeps++;

    // Attempt to zero out all off diagonal elements. The cumulant matrices are
    // all symmetric, so we only need to work on either the upper or lower
    // triangle of the matrices. We arbitrarily choose the upper.
    for (p = 0; p < _num_var - 1; p++) {
      for (q = p + 1; q < _num_var; q++) {
        // The core of this double loop attempts to minimize the p,q'th element
        // in each cumulant matrix. It takes a lot of algebra to explain why
        // this calculation is valid--too much to put into these comments.

        // Find the sum of the difference between diagonal elements and the sum
        // of the off diagonal elements (the matrices are symmetric, so we just
        // multiply the upper off diagonal by two).
        GG_x = 0.0f;
        GG_y = 0.0f;
        GG_z = 0.0f;
        for (cm = 0; cm < _num_cm; cm++) {
          pp_i = cm * _num_elem + p * _num_var + p; // row p, column p
          qq_i = cm * _num_elem + q * _num_var + q; // row q, column q
          pq_i = cm * _num_elem + q * _num_var + p; // row p, column q

          on_diag  = _cm_mat[pp_i] - _cm_mat[qq_i];
          off_diag = _cm_mat[pq_i] * 2.0;
          GG_x += on_diag  * on_diag;
          GG_y += on_diag  * off_diag;
          GG_z += off_diag * off_diag;
        }

        on_diag  = GG_x - GG_z;
        off_diag = GG_y * 2.0;

        // Find the angle of rotation to be performed. It is possible to find
        // this angle using only multiply/divides, but experiments showed that
        // timing was roughly the same, so we use atan2() because it's cleaner
        // code.
        theta = 0.5 * atan2( off_diag, on_diag +
                             sqrt(on_diag * on_diag + off_diag * off_diag));

        // Only perform a rotation if it is 'statistically relavent'.
        if (fabs( theta ) > _threshold) {
          modified = 1;
          cosine   = cos( theta );
          sine     = sin( theta );

          //////////////////////////////////////////////////////////////////////
          // We only need to update the upper triangle of the cumulant matrices.
          //////////////////////////////////////////////////////////////////////

          // For each cumulant matrix...
          for (i = 0; i < _num_cm; i++) {
            // Update the p,q'th, p,p'th, and q,q'th elements.
            pp_i = p * _num_var + p + _num_elem * i; // row p, column p
            qq_i = q * _num_var + q + _num_elem * i; // row q, column q
            pq_i = q * _num_var + p + _num_elem * i; // row p, column q

            cos_sqr = cosine * cosine;
            sin_sqr =   sine *   sine;

            tmp1 = _cm_mat[pp_i];
            tmp2 =  2.0 * cosine * sine * _cm_mat[pq_i];

            _cm_mat[pq_i] = (cos_sqr - sin_sqr) * _cm_mat[pq_i] +
                           cosine * sine * (_cm_mat[qq_i] - _cm_mat[pp_i]);
            _cm_mat[pp_i] = cos_sqr * tmp1 + sin_sqr * _cm_mat[qq_i] + tmp2;
            _cm_mat[qq_i] = sin_sqr * tmp1 + cos_sqr * _cm_mat[qq_i] - tmp2;

            // Update the elements in columns p and q that are above the p'th
            // row.
            for (row = 0; row < p; row++) {
              p_i = p * _num_var + row + _num_elem * i;
              q_i = q * _num_var + row + _num_elem * i;   // Update (example):
                                                        // * * x * * x *
              tmp1 = cosine * _cm_mat[p_i];              // * * x * * x *
              tmp2 =  -sine * _cm_mat[p_i];              // * * + * * + *
                                                        // * * * * * * *
              _cm_mat[p_i] = tmp1 +   sine * _cm_mat[q_i];// * * * * * * *
              _cm_mat[q_i] = tmp2 + cosine * _cm_mat[q_i];// * * * * * + *
            }                                           // * * * * * * *

            // Update the elements in the p'th row from just to the right of the
            // diagonal to the q'th column, and update the elements in the q'th
            // column from just above the diagonal up to the p'th row.
            for (col = p + 1; col < q; col++) {
              p_i = col * _num_var +  p  + _num_elem * i;
              q_i = q   * _num_var + col + _num_elem * i; // Update (example):
                                                        // * * + * * + *
              tmp1 = cosine * _cm_mat[p_i];              // * * + * * + *
              tmp2 =  -sine * _cm_mat[p_i];              // * * + x x + *
                                                        // * * * * * x *
              _cm_mat[p_i] = tmp1 +   sine * _cm_mat[q_i];// * * * * * x *
              _cm_mat[q_i] = tmp2 + cosine * _cm_mat[q_i];// * * * * * + *
            }                                           // * * * * * * *

            // In each cumulant matrix, update the elements in rows p and q that
            // are to the right of the q'th column.
            for (col = q + 1; col < _num_var; col++) {
              p_i = col * _num_var + p + _num_elem * i;
              q_i = col * _num_var + q + _num_elem * i;   // Update (example):
                                                        // * * + * * + *
              tmp1 = cosine * _cm_mat[p_i];              // * * + * * + *
              tmp2 =  -sine * _cm_mat[p_i];              // * * + + + + x
                                                        // * * * * * + *
              _cm_mat[p_i] = tmp1 +   sine * _cm_mat[q_i];// * * * * * + *
              _cm_mat[q_i] = tmp2 + cosine * _cm_mat[q_i];// * * * * * + x
            }                                           // * * * * * * *
          }

          //////////////////////////////////////////////////////////////////////
          // Update the rotation matrix.
          //////////////////////////////////////////////////////////////////////

          for (row = 0; row < _num_var; row++) {
            p_i = p * _num_var + row; // row 'row', column p of V matrix
            q_i = q * _num_var + row; // row 'row', column q of V matrix

            tmp1            = cosine * MAT_V.elem[p_i] + sine * MAT_V.elem[q_i];
            MAT_V.elem[q_i] = cosine * MAT_V.elem[q_i] - sine * MAT_V.elem[p_i];
            MAT_V.elem[p_i] = tmp1;
          }
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // We now have a best guess at an unmixing matrix. We need to finish up our
  // computations by computing the source signals, the source signal mixing
  // matrix, the unmixing matrix that will unmix the original, nonwhitened
  // observations, and the means of the source signals.
  //////////////////////////////////////////////////////////////////////////////

  // Finish computations for A, W, and S.
  GEMM_TN( *S, MAT_V, MAT_Z );      // S = V' * Z
  GEMM_TN( *W, MAT_V, MAT_WHITEN ); // W = V' * whiten_matrix
  GEMM( *A, MAT_DEWHITEN, MAT_V );  // A = dewhiten * V

  // Recreate the source signal means.
  GEMV( mu_S, *W, _mu_X );

  //////////////////////////////////////////////////////////////////////////////
  // We're done!
  //////////////////////////////////////////////////////////////////////////////

  return sweeps;
}
