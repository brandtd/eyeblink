#ifndef FASTICA_CONTRAST_H
#define FASTICA_CONTRAST_H

#include "matrix.h"
#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This file contains prototypes for various contrast functions that all apply
 * the ICA learning rule:
 *
 *   w_next = E{z * g(w' * z)} - E{g`(w' * z)} * w
 *
 * where E{.} is the expected value, g`(.) is the first derivative of g(.), and
 * (.)' is the transpose of (.).
 *
 * Each function takes in a W and a Z matrix, and with W equal to the current
 * guess at the unmixing matrix (with each row an unmixing vector) and with Z
 * equal to the whitened set of observation data (rows as variables and columns
 * as observations), each function applies one iteration of the above learning
 * rule and returns the approximated next guess for the matrix W.
 *
 * The CPU functions return this as the matrix W_next. The GPU functions
 * overwrite the previous value for W with the new guess.
 *
 * If W_next and W are within a certain epsilon (ignoring sign), then the
 * matrices are said to have converged.
 *
 * For the GPU functions, the d_W and d_Z matrices are assumed to have their
 * elements stored on the device (GPU).
 *
 * Each function applies a different definition for g(x).
 */

// The general form of a contrast function for the CPU. Three matrix parameters
// are given. The first is where the next guess at the unmixing matrix will be
// stored. The second is where to find the current guess at the unmixing matrix,
// and the third is where to find the whitened set of observation data.
typedef void (*ContFunc)( Matrix*, Matrix*, Matrix* );

/**
 * Name: negent_tanh
 *
 * Description:
 * This function sets g(x) = tanh(x)
 *
 * Parameters:
 * @param W_next  where to store the next guess at the unmixing matrix W
 * @param W       the current guess at the unmixing matrix W
 * @param Z       the whitened set of observation data
 */
void negent_tanh( Matrix *W_next, Matrix *W, Matrix *Z );

/**
 * Name: negent_cube
 *
 * Description:
 * This function sets g(x) = x^3
 *
 * Parameters:
 * @param W_next  where to store the next guess at the unmixing matrix W
 * @param W       the current guess at the unmixing matrix W
 * @param Z       the whitened set of observation data
 */
void negent_cube( Matrix *W_next, Matrix *W, Matrix *Z );

/**
 * Name: negent_gauss
 *
 * Description:
 * This function sets g(x) = x * exp(-x^2 / 2)
 *
 * Parameters:
 * @param W_next  where to store the next guess at the unmixing matrix W
 * @param W       the current guess at the unmixing matrix W
 * @param Z       the whitened set of observation data
 */
void negent_gauss( Matrix *W_next, Matrix *W, Matrix *Z );

#ifdef __cplusplus
}
#endif

#ifdef ENABLE_GPU
// For the GPU, the first and second argument of the CPU function are combined.
typedef void (*GPUContFunc)( Matrix*, Matrix* );

/**
 * Name: gpu_negent_tanh
 *
 * Description:
 * This function sets g(x) = tanh(x)
 *
 * POST:
 * The d_W matrix is changed to contain the new guess for d_W with the learning
 * rule applied to its original value.
 *
 * Parameters:
 * @param d_W   the current guess at the unmixing matrix W
 * @param d_Z   the whitened set of observation data
 */
void gpu_negent_tanh( Matrix *d_W, Matrix *d_Z );

/**
 * Name: gpu_negent_cube
 *
 * Description:
 * This function sets g(x) = x^3
 *
 * POST:
 * The d_W matrix is changed to contain the new guess for d_W with the learning
 * rule applied to its original value.
 *
 * Parameters:
 * @param d_W   the current guess at the unmixing matrix W
 * @param d_Z   the whitened set of observation data
 */
void gpu_negent_cube( Matrix *d_W, Matrix *d_Z );

/**
 * Name: gpu_negent_gauss
 *
 * Description:
 * This function sets g(x) = x * exp(-x^2 / 2)
 *
 * POST:
 * The d_W matrix is changed to contain the new guess for d_W with the learning
 * rule applied to its original value.
 *
 * Parameters:
 * @param d_W   the current guess at the unmixing matrix W
 * @param d_Z   the whitened set of observation data
 */
void gpu_negent_gauss( Matrix *d_W, Matrix *d_Z );
#endif

#endif
