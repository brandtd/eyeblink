#ifndef BLINK_AUX_H
#define BLINK_AUX_H

#include "matrix.h"
#include "numtype.h"
#include "blink/params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: findPossibleBlinks
 *
 * Description:
 * Finds possible blink locations within a single channel. The blink location
 * array returned by this function is malloc(3)'d and must be free(3)'d by the
 * caller.
 *
 * Returns NULL if no blinks were found.
 *
 * Parameters:
 * @param num_blinks        where to store how many blinks were detected
 * @param channel           the channel to process
 * @param len_channel       how many samples are in the channel
 * @param params            configuration parameters
 *
 * Returns:
 * @return int*             the locations of blinks within the channel
 */
int *findPossibleBlinks( int *num_blinks, const NUMTYPE *channel,
                         int len_channel, const BlinkParams *params );

/**
 * Name: envelope
 *
 * Description:
 * Envelopes the absolute value of a a given signal from above. This function
 * works by finding the points in the input signal where the slope of its
 * absolute value goes from positive to non-positive and then linearly
 * interpolates between those points.
 *
 * The given signal is modified in place.
 *
 * Parameters:
 * @param signal      the signal to envelope
 * @param length      the length of the signal
 */
void envelope( NUMTYPE *signal, unsigned int length );

/**
 * Name: correlate
 *
 * Description:
 * Calculates the correlation coefficient of two signals. The signals are
 * assumed to be the same length.
 * 
 * Parameters:
 * @param sig_1       one of the signals to correlate
 * @param sig_2       one of the signals to correlate
 * @param length      the length of the signals
 *
 * Returns:
 * @return NUMTYPE    the correlation coefficient of the two signals
 */
NUMTYPE correlate( const NUMTYPE *sig_1, const NUMTYPE *sig_2,
                   unsigned int length );

/**
 * Name: maxCorrelate
 *
 * Description:
 * Finds the row in `mat_S' with the highest absolute correlation with the
 * given `signal', 'absolute correlation' meaning the absolute value of the
 * correlation (ignores sign).
 *
 * PRE:
 * The `mat_S' matrix is assumed to be stored in column-major order.
 *
 * The `signal' parameter must have at least the same number of elements as the
 * matrix has columns.
 *
 * Parameters:
 * @param mat_S             the matrix to compute against
 * @param signal            the signal to correlate with
 *
 * Returns:
 * @return int              the row in the matrix with the largest correlation
 */
int maxCorrelate( const Matrix *mat_S, const NUMTYPE *signal );

/**
 * Name: getTemplate
 *
 * Description:
 * Creates an eyeblink template. The returned template has been malloc(3)'d and
 * must be free(3)'d by the caller.
 *
 * The `steps_1' parameter stores the length of the first part of the template.
 * The 'peak' of the template can be found 'steps_1' indices into the returned
 * template.
 *
 * Parameters:
 * @param steps_1           where to store the length of the first part
 * @param len_template      where to store the length of the resulting template
 * @param params            configuration parameters
 *
 * Returns:
 * @return NUMTYPE*         the template
 */
NUMTYPE *getTemplate( int *steps_1, int *len_template,
                      const BlinkParams *params );

/**
 * Name: movingAverage
 *
 * Description:
 * Finds the moving average of the input signal over a window of 512 elements.
 *
 * PRE:
 * This function assumes that enough memory has been allocated for the output
 * array. The output length must be as long as the input signal length. This
 * function also assumes that the input is more than 512 elements long.
 *
 * The output, input, and filter parameters must not overlap.
 *
 * Parameters:
 * @param output        OUTPUT where to store the results
 * @param input         INPUT  the input signal vector
 * @param len_input     INPUT  the length of the input vector
 */
void movingAverage( NUMTYPE *output, const NUMTYPE *input,
                    unsigned int len_input );

/**
 * Name: blinkFlatten
 *
 * Description:
 * Removes blinks from a blink source component by flattening the blink
 * component for 0.2 seconds before and after the blink location.
 *
 * Parameters:
 * @param mat_S             the source component matrix
 * @param blinks            the locations of blinks within the source component
 * @param num_blinks        the number of blinks
 * @param row               the blink source component row (0-based)
 * @param b_params          blink detection parameters
 */
void blinkFlatten( Matrix *mat_S, const int *blinks, int num_blinks, int row,
                   const BlinkParams *b_params );

#ifdef __cplusplus
}
#endif

#endif
