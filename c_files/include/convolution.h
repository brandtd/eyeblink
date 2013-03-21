#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: conv_mirrorDown
 *
 * Description:
 * Convolves the input signal and filter while simultaneously down-sampling the
 * result. Result is down-sampled by dropping every even index starting at zero.
 *
 * PRE:
 * This function assumes that enough memory has been allocated for the output
 * array. The output length will always be equal to:
 *    len_output = floor( (len_input + len_filter - 1) / 2 )
 *
 * The output, input, and filter parameters must not overlap.
 *
 * This function also assumes that the input is longer than the filter.
 *
 * Parameters:
 * @param output        OUTPUT where to store the results
 * @param input         INPUT  the input signal vector
 * @param len_input     INPUT  the length of the input vector
 * @param filter        INPUT  the filter vector
 * @param len_filter    INPUT  the length of the filter vector
 */
void conv_mirrorDown( NUMTYPE *output,
                      NUMTYPE const *input,  unsigned int len_input,
                      NUMTYPE const *filter, unsigned int len_filter );

/**
 * Name: conv_mirrorUp
 *
 * Description:
 * Upsamples the input signal and then convolves that result and filter. The
 * upsampling is performed by generating a new signal:
 *           { X(k/2), k even
 *    Y(k) = {                , 1 <= k <= 2*length(X)
 *           { 0,      k odd
 * where X is the original input signal and Y is the upsampled signal.
 *
 * PRE:
 * This function assumes that enough memory has been allocated for the output
 * array. The output length will always be equal to:
 *    len_output = 2*len_input + len_filter - 1
 *
 * The output, input, and filter parameters must not overlap.
 *
 * This function also assumes that the input is longer than the filter.
 *
 * Parameters:
 * @param output        OUTPUT where to store the results
 * @param input         INPUT  the input signal vector
 * @param len_input     INPUT  the length of the input vector
 * @param filter        INPUT  the filter vector
 * @param len_filter    INPUT  the length of the filter vector
 */
void conv_mirrorUp( NUMTYPE *output,
                    NUMTYPE const *input,  unsigned int len_input,
                    NUMTYPE const *filter, unsigned int len_filter );

#ifdef __cplusplus
}
#endif

#endif
