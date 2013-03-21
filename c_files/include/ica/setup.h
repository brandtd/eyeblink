#ifndef ICA_SETUP_H
#define ICA_SETUP_H

#include "matrix.h"
#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: fastica_init
 *
 * Description:
 * Initializes the CPU implementation of fastica. This function should only be
 * called by the ica_init() function.
 *
 * Parameters:
 * @param params        configuration parameters for the ICA algorithm
 *
 * Returns:
 * @return int          zero if a problem occurs, nonzero otherwise
 */
int fastica_init( ICAParams *params );

/**
 * Name: jade_init
 *
 * Description:
 * Initializes the CPU implementation of JADE. This function should only be
 * called by the ica_init() function.
 *
 * Parameters:
 * @param params        configuration parameters for the ICA algorithm
 *
 * Returns:
 * @return int          zero if a problem occurs, nonzero otherwise
 */
int jade_init( ICAParams *params );

/**
 * Name: fastica_shutdown
 *
 * Description:
 * Cleans up and shuts down the CPU implementation of the fastica library,
 * freeing allocated memory, etc. This function should only be called by the
 * ica_shutdown() function.
 */
void fastica_shutdown();

/**
 * Name: jade_shutdown
 *
 * Description:
 * Cleans up and shuts down the CPU implementation of the fastica library,
 * freeing allocated memory, etc. This function should only be called by the
 * ica_shutdown() function.
 */
void jade_shutdown();

#ifdef __cplusplus
}
#endif

#ifdef ENABLE_GPU
/**
 * Name: fastica_gpuInit
 *
 * Description:
 * Initializes the GPU implementation of fastica. This function should only be
 * called by the ica_init() function.
 *
 * Parameters:
 * @param params        configuration parameters for the ICA algorithm
 *
 * Returns:
 * @return int          zero if a problem occurs, nonzero otherwise
 */
int fastica_gpuInit( ICAParams *params );

/**
 * Name: jade_gpuInit
 *
 * Description:
 * Initializes the GPU implementation of JADE. This function should only be
 * called by the ica_init() function.
 *
 * Parameters:
 * @param params        configuration parameters for the ICA algorithm
 *
 * Returns:
 * @return int          zero if a problem occurs, nonzero otherwise
 */
int jade_gpuInit( ICAParams *params );

/**
 * Name: fastica_gpuShutdown
 *
 * Description:
 * Cleans up and shuts down the GPU implementation of the fastica library,
 * freeing allocated memory, etc. This function should only be called by the
 * ica_shutdown() function.
 */
void fastica_gpuShutdown();

/**
 * Name: jade_gpuShutdown
 *
 * Description:
 * Cleans up and shuts down the GPU implementation of the fastica library,
 * freeing allocated memory, etc. This function should only be called by the
 * ica_shutdown() function.
 */
void jade_gpuShutdown();
#endif

#endif
