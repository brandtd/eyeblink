#include "ica/ica.h"
#include "ica/setup.h"
#include "ica/fastica/contrast.h"

#include <string.h>

/**
 * A global copy of the ICA configuration parameters that we're using. We keep
 * a copy so that we can check for changes so that we know when we need to
 * reinitialize the library.
 */
static ICAParams _ica_params;
static int _initialized = 0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int ica_init( ICAParams const *params )
{
  // Check to see if we've already initialized. If we have, only reinitialize
  // if the ICA params just given differ from the ones we've already got.
  if (_initialized) {
    if ((_ica_params.implem     != params->implem)     ||
        (_ica_params.epsilon    != params->epsilon)    ||
        (_ica_params.contrast   != params->contrast)   ||
        (_ica_params.max_iter   != params->max_iter)   ||
        (_ica_params.num_var    != params->num_var)    ||
        (_ica_params.num_obs    != params->num_obs)    ||
        (_ica_params.gpu_device != params->gpu_device) ||
        (_ica_params.use_gpu    != params->use_gpu)) {
      ica_shutdown();
    } else {
      return _initialized;
    }
  }

  // Backup the parameters so that we know how to properly shutdown.
  memcpy( &_ica_params, params, sizeof(ICAParams) );

  //////////////////////////////////////////////////////////////////////////////
  // Initialize the appropriate library.
  //////////////////////////////////////////////////////////////////////////////
  switch (_ica_params.implem) {
    case ICA_JADE:
      if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
        _initialized = jade_gpuInit( &_ica_params );
#endif
      } else {
        _initialized = jade_init( &_ica_params );
      }
      break;

    case ICA_FASTICA:
    default:
      if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
        _initialized = fastica_gpuInit( &_ica_params );
#endif
      } else {
        _initialized = fastica_init( &_ica_params );
      }
      break;
  }

  return _initialized;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ica_shutdown()
{
  if (_initialized) {
    ////////////////////////////////////////////////////////////////////////////
    // Shutdown the appropriate library.
    ////////////////////////////////////////////////////////////////////////////
    switch (_ica_params.implem) {
      case ICA_JADE:
        if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
          jade_gpuShutdown();
#endif
        } else {
          jade_shutdown();
        }
        break;

      case ICA_FASTICA:
      default:
        if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
          fastica_gpuShutdown();
#endif
        } else {
          fastica_shutdown();
        }
        break;
    }

    _initialized = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int ica( Matrix *W, Matrix *A, Matrix *S, NUMTYPE *mu_S,
                  Matrix const *X )
{
  ICAParams def_params;

  //////////////////////////////////////////////////////////////////////////////
  // Verify that we've been initialized and with the correct settings.
  //////////////////////////////////////////////////////////////////////////////
  if (!_initialized) {
    // If we haven't already been initialized, setup the default values.
    def_params.implem     = DEF_IMPLEM;
    def_params.epsilon    = DEF_EPSILON;
    def_params.contrast   = DEF_CONTRAST;
    def_params.max_iter   = DEF_MAX_ITER;
    def_params.num_var    = X->rows;
    def_params.num_obs    = X->cols;
    def_params.gpu_device = DEF_GPU_DEVICE;
    def_params.use_gpu    = 0;

    ica_init( &def_params );
  }

  // Make sure that the parameters we initialized with match the size of the
  // X matrix on which we're about to operate.
  if (_ica_params.num_var != X->rows || _ica_params.num_obs != X->cols) {
    // We need to reinitialize for a new observation matrix size.
    memcpy( &def_params, &_ica_params, sizeof(ICAParams) );
    def_params.num_var = X->rows; def_params.num_obs = X->cols;

    ica_init( &def_params );
  }

  //////////////////////////////////////////////////////////////////////////////
  // Launch the appropriate function.
  //////////////////////////////////////////////////////////////////////////////
  switch (_ica_params.implem) {
    case ICA_JADE:
      if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
        return jade_gpu( W, A, S, mu_S, X );
#endif
      } else {
        return jade( W, A, S, mu_S, X );
      }

    case ICA_FASTICA:
    default:
      if (_ica_params.use_gpu) {
#ifdef ENABLE_GPU
        return fastica_gpu( W, A, S, mu_S, X );
#endif
      } else {
        return fastica( W, A, S, mu_S, X );
      }
  }

  // If we get here, something bad happened.
  return 0;
}
