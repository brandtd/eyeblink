#ifndef BLINK_DETECT_H
#define BLINK_DETECT_H

#include "matrix.h"
#include "numtype.h"
#include "blink/params.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: blinkDetect
 *
 * Description:
 * Performs blink detection on the given set of EEG channels, returning a list
 * of sample indices where blinks have been determined to be located. Please see
 * the MATLAB blink_detect code for an explanation of the algorithm and
 * configuration parameters.
 *
 * The returned list is malloc(3)'d and must be free(3)'d by the caller.
 *
 * If no blinks are detected, NULL is returned.
 *
 * The channels parameter should be a matrix of channel data, with each row
 * representing a different channel and each column representing a sample of the
 * channels. The channel matrix should be stored in row-major order.
 *
 * Parameters:
 * @param num_blinks        where to store how many blinks were detected
 * @param channels          where to find the EEG channels
 * @param len_channel       how many samples are in each channel
 * @param num_channels      how many channels were given
 * @param params            configuration parameters
 *
 * Returns:
 * @return int*             the locations of blinks within the channels
 */
int *blinkDetect( int *num_blinks, const NUMTYPE *channels, int len_channel,
                  int num_channels, const BlinkParams *params );

/**
 * Name: blinkSource
 * 
 * Description:
 * Indentifies the source signal that is most likely to be the eyeblink source.
 *
 * The given `mat_S' parameter should be the extracted source signal matrix and
 * the `blinks' parameter should be the eyeblink indices as returned by
 * blinkDetect().
 *
 * In the source signal matrix, each row is treated as a signal, each column as
 * an observation of the signals.
 *
 * Parameters:
 * @param mat_S             the source signal matrix
 * @param blinks            the estimated locations of blinks
 * @param len_blinks        the length of the blinks array
 * @param params            configuration parameters
 *
 * Returns:
 * @return int              the row index in the S matrix of the eyeblink source
 *                          (0-based)
 */
int blinkSource( const Matrix *mat_S, const int *blinks, int len_blinks,
                 const BlinkParams *params );

/**
 * Name: blinksInSource
 *
 * Description:
 * Finds the locations of blinks in the given source signal by thresholding the
 * signal against a value that is dependent on the number of expected blinks.
 *
 * The given `blinks' parameter should be the same as the value returned by the
 * blinkDetect() function.
 *
 * The returned list is malloc(3)'d and must be free(3)'d by the caller.
 *
 * Parameters:
 * @param num_blinks        where to store how many blinks were detected
 * @param mat_S             the source signal matrix
 * @param blink_source      the source signal row index
 * @param blinks            the locations of blinks according to blinkDetect
 * @param len_blinks        the length of the blinks array
 * @param params            configuration parameters
 *
 * Returns:
 * @return int*             the locations of blinks within the source signal
 */
int *blinksInSource( int *num_blinks, const Matrix *mat_S, int blink_source,
                     const int *blinks, unsigned int len_blinks,
                     const BlinkParams *params );

#ifdef __cplusplus
}
#endif

#endif
