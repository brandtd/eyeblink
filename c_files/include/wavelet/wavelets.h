#ifndef WAVELETS_H
#define WAVELETS_H

#include "numtype.h"
#include "wavelet/wavelet_filters.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enum to be used when specifying detail or approximation reconstruction.
 */
typedef enum ReconType {
  RECON_DETAIL,
  RECON_APPROX
} ReconType;

/**
 * Name: wavedecMaxLevel
 *
 * Description:
 * Determines the maximum wavelet deconstruction level that can be achieved with
 * a call to wavedec() using the given signal and filter lengths.
 *
 * Parameters:
 * @param len_signal    the length of the signal vector
 * @param len_filter    the length of the filter vector
 *
 * Returns:
 * @return unsigned int   the maximum deconstruction level
 */
unsigned int wavedecMaxLevel( unsigned int len_signal,
                              unsigned int len_filter );

/**
 * Name: wavedecResultLength
 *
 * Description:
 * Calculates and returns the length the coefficient vector that will needed by
 * a call to the wavedec() function with the given wavelet, signal length, and
 * deconstruction level.
 *
 * Parameters:
 * @param len_signal    the length of the signal vector
 * @param len_filter    the length of the filter vector
 * @param max_level     the maximum deconstruction level
 *
 * Returns:
 * @return unsigned int   the required length of the coefficient vector
 */
unsigned int wavedecResultLength( unsigned int len_signal,
                                  unsigned int len_filter,
                                  unsigned int max_level );

/**
 * Name: wavedec
 *
 * Description:
 * This function is meant to perform similar to MATLAB's wavedec() function.
 *
 * This function performs a one-dimensional deconstruction of the given signal
 * vector using the specified wavelet. The coefficient vectors for each level
 * of the deconstruction are stored in the given coefs output vector, while
 * the length of each coefficient vector is returned in the length output
 * vector.
 *
 * For example, if C is the output coefficient vector and L is the output length
 * vector, then, for a three level deconstruction, the resulting C and L vectors
 * would look like:
 *
 *         +-----+-----+---------+-----------------+
 *    C -> | cA3 | cD3 |   cD2   |       cD1       |
 *         +-----+-----+---------+-----------------+
 *
 *    L -> [ length(cA3), length(cD3), length(cD2), length(cD1) ]
 *
 * Where cDx represent detail coefficients at level x, and cAy represents
 * approximation coefficients at level y.
 *
 * PRE:
 * The output vectors are assumed to have the required amount of space available
 * for writing to. To ensure this, it is recommended to call the
 * wavedecMaxLevel() and wavedecResultLength() functions to find the required
 * lengths of the vectors before using this function.
 *
 * The 'lengths' vector must be at least 'level + 1' elements long.
 *
 * Parameters:
 * @param coefs       OUTPUT  where to store the resulting coefficients
 * @param lengths     OUTPUT  where to store the lengths of the coef. vectors
 * @param signal      INPUT   the input signal vector
 * @param len_signal  INPUT   the length of the signal vector
 * @param wavelet     INPUT   which wavelet to use to deconstruct the signal
 * @param level       INPUT   the deconstruction level to shoot for
 */
void wavedec( NUMTYPE *coefs,  unsigned int *lengths,
              NUMTYPE const *signal, unsigned int len_signal,
              Wavelet wavelet, unsigned int level );

/**
 * Name: wrcoef
 *
 * Description:
 * This function is meant to perform similar to MATLAB's wrcoef() function.
 *
 * This function reconstructs the coefficients from a single branch of a one-
 * dimensional wavelet deconstruction given by the input 'coefs' and 'lengths'
 * vectors. The input 'coefs' and 'lengths' should be the same format as those
 * created by the wavedec() function.
 *
 * The type parameter defines whether to reconstruct detail or approximation
 * coefficients, and the level parameter specifies which level to reconstruct.
 *
 * If an approximation reconstruction is used, then the level parameter may
 * equal zero, in which case the original signal is reconstructed. The level
 * parameter must always satisfy:
 *    level <= len_lengths - 1
 *
 * PRE:
 * The level parameter must be less than the length of the 'lengths' parameter
 * and must be greater than zero, unless an approximation reconstruction is
 * desired, in which case, the level parameter must be greater than or equal to
 * zero.
 *
 * The 'result' output is assumed to have the required amount of space available
 * for writing to. The length of the result vector must be at least the size of
 * the original signal vector.
 *
 * Parameters:
 * @param result      OUTPUT  where to store the resulting signal
 * @param coefs       INPUT   where to find the coefficient vectors
 * @param lengths     INPUT   where to find the coefficient vector lengths
 * @param len_lengths INPUT   the length of the 'lengths' vector
 * @param type        INPUT   which type of coefficients to reconstruct
 * @param wavelet     INPUT   which wavelet to use
 * @param level       INPUT   which level of coefficients to reconstruct
 */
void wrcoef( NUMTYPE *result,
             NUMTYPE const *coefs, unsigned int const *lengths,
             unsigned int len_lengths, ReconType type, Wavelet wavelet,
             unsigned int level );

/**
 * Name: idwtResultLength
 *
 * Description:
 * Calculates and returns the length of the signal vector that results from
 * performing the inverse discrete wavelet transform using vectors of the given
 * lengths.
 *
 * Parameters:
 * @param len_coef    the length of the coefficient vector
 * @param len_filter  the length of the filter vector
 *
 * Returns:
 * @return unsigned int   the resulting signal vector length
 */
extern inline unsigned int idwtResultLength( unsigned int len_coef,
                                             unsigned int len_filter );

/**
 * Name: idwt
 *
 * Description:
 * This function is meant to perform similarly to MATLAB's idwt() function.
 *
 * This function performs a single-level inverse discrete wavelet transform
 * on the given coefficients using the specified wavelet.
 *
 * PRE:
 * The result output is assumed to have the required amount of space available
 * for writing to. The required length of the result vector is given by the
 * idwtResultLength() function.
 *
 * Parameters:
 * @param result        OUTPUT    where to store the resulting signal
 * @param coef_approx   INPUT     where to find the approximation coefficients
 * @param coef_detail   INPUT     where to find the detail coefficients
 * @param coef_length   INPUT     the length of the coefficient vectors
 * @param wavelet       INPUT     which wavelet to use to reconstruct
 */
void idwt( NUMTYPE *result, NUMTYPE const *coef_approx,
           NUMTYPE const *coef_detail,
           unsigned int coef_length, Wavelet wavelet );

#ifdef __cplusplus
}
#endif

#endif
