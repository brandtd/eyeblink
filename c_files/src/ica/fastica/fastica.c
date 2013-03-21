#include "ica/ica.h"
#include "ica/aux.h"
#include "ica/setup.h"
#include "ica/fastica/contrast.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

/**
 * Global variables setup in our initialization function. Setup of these
 * variables is an overhead that we shouldn't have to incur for every run of
 * the fastica() computation, since the values for these variables is dependent
 * on the ICA configuration parameters and nothing else.
 */
static Matrix   _white_Z, _tW[6];       // Workspace matrices.
static NUMTYPE *_eig_vals = NULL;       // Where we store computed eigen values.
static ContFunc _contrast = NULL;       // The contrast function we apply.
static NUMTYPE  _epsilon = 0.0;         // Convergence epsilon.
static int _max_iter = 0;               // Max number of iterations to perform.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int fastica_init( ICAParams *params )
{
  int i;

  //////////////////////////////////////////////////////////////////////////////
  // Initialize configuration parameters.
  //////////////////////////////////////////////////////////////////////////////
  _epsilon  = params->epsilon;
  _max_iter = params->max_iter;

  switch (params->contrast) {
    case NONLIN_CUBE:
      _contrast = negent_cube;
      break;
    case NONLIN_GAUSS:
      _contrast = negent_gauss;
      break;
    case NONLIN_TANH:
    default:
      _contrast = negent_tanh;
      break;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Allocate memory.
  //////////////////////////////////////////////////////////////////////////////
  _eig_vals     = (NUMTYPE*) malloc( sizeof(NUMTYPE) * params->num_var );
  _white_Z.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * params->num_var *
                                     params->num_obs );
  _white_Z.cols = _white_Z.lag = params->num_obs;
  _white_Z.rows = _white_Z.ld  = params->num_var;

  // The _tW matrices are all temporary matrices used as scratch space in our
  // calculations, and _tW[0] is used as another name for the W parameter.
  // NOTE: the setup of _tW[0] must be down within the fastica() function.
  //    _tW[0] = *W;
  for (i = 1; i < sizeof(_tW) / sizeof(Matrix); i++) {
    _tW[i].elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) *
                                     params->num_var * params->num_var );
    _tW[i].rows = _tW[i].cols = _tW[i].ld = _tW[i].lag = params->num_var;
  }

  // All done. Return that things went OK.
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void fastica_shutdown()
{
  int i;

  //////////////////////////////////////////////////////////////////////////////
  // Free allocated memory and set everything to NULL/0.
  //////////////////////////////////////////////////////////////////////////////
  free( _eig_vals );
  _eig_vals = NULL;

  free( _white_Z.elem );
  _white_Z.elem = NULL;
  _white_Z.cols = _white_Z.lag = _white_Z.rows = _white_Z.ld = 0;

  for (i = 1; i < sizeof(_tW) / sizeof(Matrix); i++) {
    free( _tW[i].elem );
    _tW[i].elem = NULL;
    _tW[i].rows = _tW[i].cols = _tW[i].ld = _tW[i].lag = 0;
  }

  _contrast = NULL;
  _epsilon = 0.0;
  _max_iter = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int fastica( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                      Matrix const *X )
{
  int num_iter, prev_i, new_i, row, col, i;
  NUMTYPE min;

  // Setup the renaming of the `W' parameter.
  _tW[0] = *W;

  //////////////////////////////////////////////////////////////////////////////
  // Make the observations zero-mean, temporarily recommissioning the mu_S
  // array to store the observation means, so that we can calculate the source
  // signal means later, and using the S matrix parameter to temporarily hold
  // the zero-mean observations.
  //////////////////////////////////////////////////////////////////////////////

  remmean( mu_S, S, X );

  //////////////////////////////////////////////////////////////////////////////
  // Make the zero-mean observations white.
  //////////////////////////////////////////////////////////////////////////////

  whiten( &_white_Z, &_tW[2], &_tW[3], S, 0 );
  // _tW[2] <--   whitening matrix
  // _tW[3] <-- dewhitening matrix

  //////////////////////////////////////////////////////////////////////////////
  // Initialize our guess at the unmixing matrix to the identity matrix.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < _tW[0].rows * _tW[0].cols; i++) {
    _tW[0].elem[i] = 0.0;
  }
  for (i = 0; i < _tW[0].rows; i++) {
    _tW[0].elem[i + i*_tW[0].rows] = 1.0;
  }

  //////////////////////////////////////////////////////////////////////////////
  // With the observations now zero-mean and whitened, begin iterating.
  //////////////////////////////////////////////////////////////////////////////
  num_iter = 0;
  do {
    // To save us from having to copy the previous unmixing matrix guess, we
    // alternate between _tW[0] and _tW[1] holding the new/previous guess,
    // starting with _tW[0] holding the previous guess, and _tW[1] holding the
    // new guess.
    new_i  = (num_iter + 1) & 0x01;
    prev_i = (num_iter    ) & 0x01;
    num_iter++;

    ////////////////////////////////////////////////////////////////////////////
    // Apply the contrast rule to _tW[prev_i], storing the result in _tW[4].
    ////////////////////////////////////////////////////////////////////////////
    _contrast( &_tW[4], &_tW[prev_i], &_white_Z );

    ////////////////////////////////////////////////////////////////////////////
    // Orthogonalize the updated unmixing matrix.
    ////////////////////////////////////////////////////////////////////////////

    GEMM_NT( _tW[new_i], _tW[4], _tW[4] );
    SYEV( _tW[new_i], _eig_vals );

    for (i = 0; i < W->rows; i++) {
      // We need to take the square root of the absolute value here, because it
      // is possible, thanks to floating point error, than an eigenvalue has
      // become negative. If we don't use the absolute value, we will get NaN
      // infecting our data.

      // TODO: if an eigenvector becomes negative, should be just quit there and
      //       say that extraction of source signals is not possible?
      _eig_vals[i] = 1.0 / sqrt( fabs( _eig_vals[i]) );
    }

    // A = D ^ (-1/2) * E'
    for (col = 0; col < W->cols; col++) {
      for (row = 0; row < W->rows; row++) {
        i = col * W->rows + row;
        A->elem[i] = _eig_vals[row] * _tW[new_i].elem[row*W->rows + col];
      }
    }
    GEMM( _tW[5], _tW[new_i], *A );     // _tW[5] = E * D^(-1/2) * E'
    GEMM( _tW[new_i], _tW[5], _tW[4] );

    ////////////////////////////////////////////////////////////////////////////
    // Determine if the rows of the unmixing matrix have changed significantly
    // by examining the angle between the previous rows and the current rows.
    ////////////////////////////////////////////////////////////////////////////

    // We find the angle by using the dot product and the rule that the dot
    // product of two vectors is equal to the cosine of the angle between the
    // vectors.
    GEMM_NT( _tW[4], _tW[new_i], _tW[prev_i] );
    min = 1.0;

    for (i = 0; i < W->rows; i++) {
      if (min > fabs(_tW[4].elem[i*W->rows + i])) {
        min = fabs(_tW[4].elem[i*W->rows + i]);
      }
    }

  // Keep iterating until we meet the convergence criteria, or hit the maximum
  // number of iterations.
  } while ((1.0 - min) > _epsilon && num_iter < _max_iter);

  //////////////////////////////////////////////////////////////////////////////
  // We now have a best guess at an unmixing matrix. We need to finish up our
  // computations by computing the source signals, the source signal mixing
  // matrix, the unmixing matrix that will unmix the original, nonwhitened
  // observations, and the means of the source signals.
  //////////////////////////////////////////////////////////////////////////////

  // _tW[0] is just another name for *W. We need to use the newest result of the
  // iteration process as an operand in the computation of the final W matrix.
  // To do this, we need to make sure the newest result comes from a matrix
  // that is not *W.
  if (new_i == 0) {
    memcpy( _tW[1].elem, _tW[0].elem, sizeof(NUMTYPE) * W->rows * W->cols );
    new_i = 1;
  }

  // _tW[1] holds the best guess at an unmixing matrix after our iterations.
  // _tW[2] holds the whitening matrix.
  // _tW[3] holds the dewhitening matrix.

  // Finish the computations for A, W, and S.
  GEMM_NT( *A, _tW[3], _tW[1] );
  GEMM( *W, _tW[1], _tW[2] );
  GEMM( *S, _tW[1], _white_Z );

  // Compute the mean values of the signal vectors by unmixing the mean values
  // of the observation vectors.
  GEMV( _eig_vals, *W, mu_S );
  memcpy( mu_S, _eig_vals, sizeof(NUMTYPE) * S->rows );

  //////////////////////////////////////////////////////////////////////////////
  // And that's it!
  //////////////////////////////////////////////////////////////////////////////

  return num_iter;
}
