#include <math.h>
#include <stdio.h>

#include "convolution.h"

// TODO: I really feel like I must be missing something simple with how
//       'convoluted' I've made these convolution functions.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void conv_mirrorDown( NUMTYPE *output,
                      NUMTYPE const *input,  unsigned int len_input,
                      NUMTYPE const *filter, unsigned int len_filter )
{
  unsigned int i, j, k;
  NUMTYPE sum;

  // Handle the left edge of the input signal where we must pad the signal.
  for (i = 1; i < len_filter - 1; i += 2) {
    sum = 0.0;
    for (j = 0; j <= i; j++) {
      sum += input[j] * filter[i-j];
    }

    for (j = i+1; j < len_filter; j++) {
      sum += input[j-i] * filter[j];
    }
    *(output++) = sum;
  }

  // Handle the points that don't require padding of the input. The 'i' index is
  // already where we need it so we don't re-initialize it since we'd then have
  // to figure out the correct number to use because of the down-sampling.
  for (; i < len_input; i += 2) {
    sum = 0.0;
    for (j = 0; j < len_filter; j++) {
      sum += input[i-j] * filter[j];
    }
    *(output++) = sum;
  }

  // Handle the right edge of the input signal where we need to pad again.
  for (; i < len_input + len_filter - 1; i += 2) {
    k = 2 * len_input - 2 - i;

    sum = 0.0;
    for (j = k; j < len_input; j++) {
      sum += input[j] * filter[j-k];
    }

    for (j = 1 - len_filter + i; j < len_input - 1; j++) {
      sum += input[j] * filter[i-j];
    }
    *(output++) = sum;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void conv_mirrorUp( NUMTYPE *output,
                    NUMTYPE const *input,  unsigned int len_input,
                    NUMTYPE const *filter, unsigned int len_filter )
{
  unsigned int i, j, k;
  NUMTYPE sum[2];

  sum[0] = 0.0;
  for (i = 1; i < len_filter; i += 2) {
    sum[0] += input[(i - 1) / 2] * filter[i];
  }
  *(output++) = sum[0];

  // Handle the left edge of the input signal where we must pad the signal.
  for (i = 0; i < (len_filter - 1) / 2; i++) {
    sum[0] = 0.0;
    sum[1] = 0.0;
    for (j = 0; j <= i; j++) {
      sum[0] += input[j] * filter[2*(i-j)];
      sum[1] += input[j] * filter[2*(i-j) + 1];
    }

    for (j = i+1; j < len_filter/2; j++) {
      sum[0] += input[j-i-1] * filter[2*j];
      sum[1] += input[j-i-1] * filter[2*j + 1];
    }

    // If the filter length is odd, we need an extra accumulate step for the
    // first sum.
    if (len_filter & 1) {
      sum[0] += input[j-i-1] * filter[2*j];
    }

    *(output++) = sum[0];
    *(output++) = sum[1];
  }

  // Handle the points that don't require padding of the input. The 'i' index is
  // already where we need it so we don't re-initialize it since we'd then have
  // to figure out the correct number to use because of the down-sampling.
  for (; i < len_input; i++) {
    sum[0] = 0.0;
    sum[1] = 0.0;

    for (j = 0; j < len_filter / 2; j++) {
      sum[0] += input[i-j] * filter[2*j];
      sum[1] += input[i-j] * filter[2*j + 1];
    }

    if (len_filter & 1) {
      sum[0] += input[i-j] * filter[2*j];
    }

    *(output++) = sum[0];
    *(output++) = sum[1];
  }

  // Handle the right edge of the input signal where we need to pad again.
  for (; i < len_input + (len_filter / 2) - 1; i++) {
    k = 2 * len_input - 2 - i;

    sum[0] = 0.0;
    sum[1] = 0.0;

    for (j = k; j < len_input; j++) {
      sum[0] += input[j] * filter[2*(j-k)];
      sum[1] += input[j] * filter[2*(j-k) + 1];
    }

    for (j = 1 - (len_filter/2) + i; j < len_input - 1; j++) {
      sum[0] += input[j] * filter[2*(i-j)];
      sum[1] += input[j] * filter[2*(i-j) + 1];
    }

    if (len_filter & 1) {
      j = 1 - (len_filter/2) + i - 1;
      sum[0] += input[j] * filter[2*(i-j)];
    }

    *(output++) = sum[0];
    *(output++) = sum[1];
  }

  // If the filter length is odd, we need one more accumulate step.
  if (len_filter & 1) {
    k = 2 * len_input - 2 - i;
    sum[0] = 0.0;

    for (j = k; j < len_input; j++) {
      sum[0] += input[j] * filter[2*(j-k)];
    }

    *(output++) = sum[0];
  }
}
