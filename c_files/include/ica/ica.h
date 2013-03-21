#ifndef ICA_H
#define ICA_H

#include "matrix.h"
#include "numtype.h"

// Default values used in the ICA algorithm.
#define DEF_IMPLEM      ICA_FASTICA
#define DEF_EPSILON     0.0001
#define DEF_CONTRAST    NONLIN_TANH
#define DEF_MAX_ITER    400
#define DEF_GPU_DEVICE  1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This enum is used to switch which nonlinearity is used to approximate
 * negentropy within the FastICA implementation of ICA.
 */
typedef enum ContrastType {
  NONLIN_TANH,
  NONLIN_CUBE,
  NONLIN_GAUSS
} ContrastType;

/**
 * This enum is used to switch between implementations of ICA (e.g. JADE and
 * FastICA).
 */
typedef enum ICA_TYPE {
  ICA_FASTICA,
  ICA_JADE,
} ICA_TYPE;

/**
 * A struct of this type must be passed to the ica() function. The meaning of
 * each value is described in the ica_init() function comment block.
 */
typedef struct ICAParams {
  ICA_TYPE     implem;
  NUMTYPE      epsilon;
  ContrastType contrast;
  unsigned int max_iter;
  unsigned int num_var;
  unsigned int num_obs;
  int          gpu_device;
  int          use_gpu;
} ICAParams;

/**
 * Data passed to a call to icaMainThread.
 */
typedef struct ICAThreadData {
  const Matrix *X;
  Matrix *W, *A, *S;
  NUMTYPE *mu_S;
  const ICAParams *ica_params;
} ICAThreadData;

/**
 * Name: icaMainThread
 *
 * Description:
 * This function is meant to provide an entry point for a pthread that will
 * perform ICA on the data contained in the ICAThreadData struct that should be
 * its parameter.
 *
 * Parameters:
 * @param data        should be of type ICAThreadData*; the data to process
 *
 * Returns:
 * @param void*       always NULL
 */
void *icaMainThread( void *data );

#ifdef __cplusplus
}
#endif

/**
 * Name: ica_init
 *
 * Description:
 * Initializes the ICA library, allocated memory and setting up global variables
 * needed to run the ICA computation. This function should be called before any
 * other ICA function, and whenever the values in the ICAParams struct change.
 *
 * This function takes in a pointer to an ICAParams struct containing some
 * configuration parameters for the function. If this function is not called,
 * the listed default values are used when the ica() function is first called.
 * The parameters, their purpose, and their default value are listed in the
 * following table:
 *
 *   property     |    default  | description
 *  --------------+-------------+-----------------------------------------------
 *    implem      | ICA_FASTICA | Which ICA implementation to use. If one of the
 *                |             | JADE implementations is specified, the
 *                |             | `epsilon', `contrast', and `max_iter'
 *                |             | parameters are unused.
 *  --------------+-------------+-----------------------------------------------
 *    epsilon     |      0.0001 | Convergence criteria. An iterative process is
 *                |             | used to find the unmixing matrix, and this
 *                |             | number defines how small a change must be in
 *                |             | the calculated matrix before it is called
 *                |             | 'converged'.
 *                |             | 
 *                |             | Convergence means that the cosine of the angle
 *                |             | between the previous unmixing vectors and the
 *                |             | current vectors is within 'epsilon' of +/- 1.
 *  --------------+-------------+-----------------------------------------------
 *    contrast    | NONLIN_TANH | The contrast/learning rule that is used to
 *                |             | find the mixing matrix. Valid values that use
 *                |             | negentropy estimation through a nonlinear
 *                |             | function are:
 *                |             |  NONLIN_TANH    using g(y) = tanh( y )
 *                |             |  NONLIN_CUBE    using g(y) = y^3
 *                |             |  NONLIN_GAUSS   using g(y) = y * exp(-y^2 / 2)
 *  --------------+-------------+-----------------------------------------------
 *    max_iter    |        1000 | The maximum number of iterations to perform
 *                |             | before giving up on achieving convergence.
 *  --------------+-------------+-----------------------------------------------
 *    num_var     |        rows | The number of variables to extract. This
 *                |             | defaults to the number of rows in the
 *                |             | observation matrix, which is currently the
 *                |             | only valid value for this parameter.
 *  --------------+-------------+-----------------------------------------------
 *    num_obs     |     columns | The number of observations in the observation
 *                |             | matrix. This defaults to the number of columns
 *                |             | in the matrix, which is currently the only
 *                |             | valid value for this parameter.
 *  --------------+-------------+-----------------------------------------------
 *    gpu_device  |           0 | Which GPU device to use. This only matters if
 *                |             | one of the GPU implementations was specified.
 *                |             | This value is passed to the cudaSetDevice()
 *                |             | function to set the device on which
 *                |             | calculations are performed. The chosen device
 *                |             | should be different from the device supporting
 *                |             | a display.
 *  --------------+-------------+-----------------------------------------------
 *
 *
 * Parameters:
 * @param params        configuration parameters for the ICA algorithm
 *
 * Returns:
 * @return int          zero if there was a problem, nonzero otherwise
 */
int ica_init( ICAParams const *params );

/**
 * Name: ica_shutdown
 *
 * Description:
 * Cleans up and shuts down the ICA library, freeing allocated memory, etc.
 */
void ica_shutdown();

/**
 * Name: ica
 *
 * Description:
 * Performs independent component analysis on the given observations X. Each
 * row of X is assumed to be an observation vector. In other words, each row
 * is a random variable and each column is an observation of that variable.
 *
 * Four values are returned by this function:
 *  W     - the inverse mixing matrix (i.e., unmixing matrix)
 *  A     - the mixing matrix
 *  S     - the calculated source signals (zero-mean)
 *  mu_S  - the calculated source signal means
 *
 * This function works by assuming that the observations, X, are a linear
 * combination of some unknown source signals, S, represented by the equation:
 *
 *    X = A * S
 *
 * The goal of this function is to find the transform, W, that will yield:
 *
 *    W * X = S
 *
 * Thus, W = A^-1
 *
 * To make finding W easier, X is first transformed into a zero-mean set of
 * observations, resulting in a new matrix, represented by:
 *    _               _
 *    X + mu_X = A * (S + mu_S)
 *       _     _
 * Where X and S are both zero mean. We can then ignore both mu_X and mu_S
 * until we have calculated W, at which point it is simple to add them back in.
 *                                                        _
 * The S matrix returned by this function is actually the S matrix reference
 * above. The original observations, X, may be reconstructed by calculating:
 *             _
 *    X = A * (S + mu_S)
 *
 * Parameters:
 * @param W           OUTPUT  where the resulting W matrix will be stored
 * @param A           OUTPUT  where the resulting A matrix will be stored
 * @param S           OUTPUT  where the resulting S matrix will be stored
 * @param mu_S        OUTPUT  where the resulting mu_S vector will be stored
 * @param X           INPUT   the observation matrix
 *
 * Returns:
 * @return unsigned int   how many iterations/sweeps the algorithm took
 */
unsigned int ica( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                  Matrix const *X );

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: fastica
 *
 * Description:
 * FastICA implementation of Independent Component Analysis. See description
 * for ica() for parameter and return value details.
 *
 * Parameters:
 * @param W           OUTPUT  where the resulting W matrix will be stored
 * @param A           OUTPUT  where the resulting A matrix will be stored
 * @param S           OUTPUT  where the resulting S matrix will be stored
 * @param mu_S        OUTPUT  where the resulting mu_S vector will be stored
 * @param X           INPUT   the observation matrix
 *
 * Returns:
 * @return unsigned int   how many iterations the algorithm took
 */
unsigned int fastica( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                      Matrix const *X );

/**
 * Name: jade 
 *
 * Description:
 * JADE implementation of Independent Component Analysis. See description
 * for ica() for parameter and return value details.
 *
 * Parameters:
 * @param W           OUTPUT  where the resulting W matrix will be stored
 * @param A           OUTPUT  where the resulting A matrix will be stored
 * @param S           OUTPUT  where the resulting S matrix will be stored
 * @param mu_S        OUTPUT  where the resulting mu_S vector will be stored
 * @param X           INPUT   the observation matrix
 *
 * Returns:
 * @return unsigned int   how many Jacobi sweeps the algorithm took
 */
unsigned int jade( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                   Matrix const *X );

#ifdef __cplusplus
}
#endif

#ifdef ENABLE_GPU
/**
 * Name: fastica_gpu
 *
 * Description:
 * Same as fastica(), but operations are performed on the GPU.
 *
 * Parameters:
 * @param W           OUTPUT  where the resulting W matrix will be stored
 * @param A           OUTPUT  where the resulting A matrix will be stored
 * @param S           OUTPUT  where the resulting S matrix will be stored
 * @param mu_S        OUTPUT  where the resulting mu_S vector will be stored
 * @param X           INPUT   the observation matrix
 */
unsigned int fastica_gpu( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                          Matrix const *X );

/**
 * Name: fastica_gpu
 *
 * Description:
 * Same as jade(), but operations are performed on the GPU.
 *
 * Parameters:
 * @param W           OUTPUT  where the resulting W matrix will be stored
 * @param A           OUTPUT  where the resulting A matrix will be stored
 * @param S           OUTPUT  where the resulting S matrix will be stored
 * @param mu_S        OUTPUT  where the resulting mu_S vector will be stored
 * @param X           INPUT   the observation matrix
 */
unsigned int jade_gpu( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                       Matrix const *X );
#endif

#endif
