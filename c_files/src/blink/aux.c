#include "blink/aux.h"
#include "wavelet/wavelets.h"
#include "convolution.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
NUMTYPE *getTemplate( int *steps_1, int *len_template,
                      const BlinkParams *params )
{
  int i, steps_2;
  NUMTYPE *templ, slope, intercept, f_s = params->f_s;

  (*steps_1) = (int) (round(0.08 * f_s));   // The length of the template parts.
  steps_2    = (int) (round(0.06 * f_s));
  (*len_template) = 2 * (*steps_1) + steps_2 - 1;

  templ = (NUMTYPE*) malloc( sizeof(NUMTYPE) * (*len_template) );

  // Generate the eyeblink template to correlate with the eyeblink locations.
  slope = -1.0 / (NUMTYPE) ((*steps_1) - 1);
  intercept = 0.0;
  for (i = 0; i < (*steps_1); i++) {
    templ[i] = slope * (NUMTYPE) i + intercept;
  }

  slope = 0.6 / (NUMTYPE) ((*steps_1) - 1);
  intercept = -1.0;
  for (i = 0; i < (*steps_1); i++) {
    templ[i + (*steps_1)] = slope * (NUMTYPE) i + intercept;
  }

  slope = 0.2 / (NUMTYPE) (steps_2 - 1);
  intercept = -0.4;
  for (i = 1; i < steps_2; i++) {
    templ[i - 1 + (*steps_1)*2] = slope * (NUMTYPE) i + intercept;
  }

  return templ;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int *findPossibleBlinks( int *num_blinks, const NUMTYPE *channel,
                         int len_channel, const BlinkParams *params )
{
  unsigned int i, j, max_level, len_coef, lengths[8];
  int cor_length, temp_start, len_template;
  NUMTYPE *coefs, *chan_a, *chan_d, *chan_r, *thresh, *templ;
  NUMTYPE min;

  int above, min_index, num_indices, *indices, steps_1, index, *blinks;
  NUMTYPE *cors;

  // Pull out the parameters that we'll be using.
  NUMTYPE t_1   = params->t_1;
  NUMTYPE t_cor = params->t_cor;

  // Make sure that we are capable of taking the 5th, 6th, and 7th level
  // decompositions.
  max_level = wavedecMaxLevel( len_channel, COIF3.len_filter );
  if (max_level < 7) {
    fprintf( stderr, "Channel too short for processing!\n" );
    (*num_blinks) = 0;
    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Allocate memory.
  //////////////////////////////////////////////////////////////////////////////

  len_coef = wavedecResultLength( len_channel, COIF3.len_filter, 7 );
  coefs    = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_coef );
  chan_a   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_channel );
  chan_d   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_channel );
  chan_r   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_channel );
  thresh   = (NUMTYPE*) malloc( sizeof(NUMTYPE) * len_channel );

  //////////////////////////////////////////////////////////////////////////////
  // Extract the appropriate approximation and detail signals.
  //////////////////////////////////////////////////////////////////////////////
  wavedec( coefs, lengths, channel, len_channel, COIF3, 7 );
  wrcoef( chan_a, coefs, lengths, 8, RECON_APPROX, COIF3, 3 );

  // Sum the enveloped 5th, 6th, and 7th level details.
  for (i = 0; i < len_channel; i++) { chan_d[i] = 0.0; }
  for (i = 5; i <= 7; i++) {
    wrcoef( chan_r, coefs, lengths, 8, RECON_DETAIL, COIF3, i );
    envelope( chan_r, len_channel );
    for (j = 0; j < len_channel; j++) {
      chan_d[j] += chan_r[j];
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Generate the first guess at eyeblink locations.
  //////////////////////////////////////////////////////////////////////////////

  // Compute the moving average threshold.
  movingAverage( thresh, chan_d, len_channel );
  for (i = 0; i < len_channel; i++) { thresh[i] += t_1; }

  // Find the minimums of the original channel between rising/falling edge pairs
  // of the enveloped channel details.
  above = 0; num_indices = 0; min = INFINITY; min_index = 0; indices = NULL;
  for (i = 1; i < len_channel; i++) {
    // Check to see if we crossed the threshold.
    if (!above && chan_d[i] > thresh[i] && chan_d[i-1] <= thresh[i-1]) {
      above = 1;

      // Reset the 'min' value so that we can find a new minimum.
      min = INFINITY;
    } else if (above && chan_d[i] < thresh[i] && chan_d[i-1] >= thresh[i-1]) {
      above = 0;

      // We just found the falling edge of a pair. Record the index of the
      // minimum value that we found within the pair.
      num_indices++;
      indices = (int*) realloc( indices, sizeof(int) * num_indices );
      indices[num_indices-1] = min_index;
    }

    // If we're above the threshold, keep track of the minimum value that we
    // see.
    if (above) {
      if (min > chan_a[i]) {
        min = chan_a[i];
        min_index = i;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Refine our first guess, getting our final guess at eyeblink locations.
  //////////////////////////////////////////////////////////////////////////////
  cors = (NUMTYPE*) malloc( sizeof(NUMTYPE) * num_indices );

  // Generate the eyeblink template to correlate with the eyeblink locations.
  templ = getTemplate( &steps_1, &len_template, params );

  // Calculate the correlations of the original channel around the minimum
  // points.
  (*num_blinks) = num_indices;
  for (i = 0; i < num_indices; i++) {
    index = indices[i] - steps_1; temp_start = 0; cor_length = len_template;

    // Make sure the correlation function will not access beyond the bounds
    // of our arrays.
    if (index < 0) {
      temp_start = -index;
      cor_length = len_template + index;
      index = 0;
    } else if (index + len_template >= len_channel) {
      cor_length = len_template - (index + len_template - len_channel);
    }

    cors[i] = correlate( chan_a + index, templ + temp_start, cor_length );

    // Mark for elimination the indices with correlations below the threshold.
    if (cors[i] < t_cor) {
      indices[i] = -1;
      (*num_blinks)--;
    }
  }

  // If we still have blinks after all this, allocate memory for them.
  if ((*num_blinks) == 0) {
    blinks = NULL;
  } else {
    blinks = (int*) malloc( sizeof(int) * (*num_blinks) );
    index = 0;
    for (i = 0; i < num_indices; i++) {
      if (indices[i] > 0) {
        blinks[index] = indices[i];
        index++;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Clean up and return.
  //////////////////////////////////////////////////////////////////////////////
  free( indices ); free( cors ); free( coefs );
  free( chan_a ); free( chan_d ); free( chan_r );
  free( thresh ); free( templ );

  return blinks;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void envelope( NUMTYPE *signal, unsigned int length )
{
  int i, j, *turn_points, num_points;
  NUMTYPE slope, intercept;

  //////////////////////////////////////////////////////////////////////////////
  // Find the signal's turning points.
  //////////////////////////////////////////////////////////////////////////////
  num_points = 1;
  turn_points = (int*) malloc( sizeof(int) * num_points );
  turn_points[0] = 0;

  signal[0] = fabs(signal[0]);
  signal[1] = fabs(signal[1]);

  for (i = 2; i < length; i++) {
    signal[i] = fabs(signal[i]);

    if ((signal[i-1] - signal[i-2] > 0) && (signal[i] - signal[i-1] <= 0)) {
      num_points++;
      turn_points = (int*) realloc( turn_points, sizeof(int) * num_points );
      turn_points[num_points-1] = i - 1;
    }
  }

  // If there were no turning points, then there's no need to interpolate, and
  // we can return right here.
  if (num_points == 1) {
    free( turn_points );
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Interpolate between the turning points.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 1; i < num_points; i++) {
    slope = (signal[ turn_points[i] ] - signal[ turn_points[i-1] ]) /
            (NUMTYPE) (turn_points[i] - turn_points[i-1]);
    intercept = signal[turn_points[i]] - slope * (NUMTYPE) turn_points[i];

    for (j = turn_points[i-1]; j < turn_points[i]; j++) {
      signal[j] = slope * (NUMTYPE) j + intercept;
    }
  }

  // Interpolate between the final turning point and the end of the signal.
  i = num_points; // Not necessary, but just so it's clear.
  slope = (signal[ length-1 ] - signal[ turn_points[i-1] ]) /
          (NUMTYPE) (length-1 - turn_points[i-1]);
  intercept = signal[length-1] - slope * (NUMTYPE) (length-1);

  for (j = turn_points[i-1]; j < length; j++) {
    signal[j] = slope * (NUMTYPE) j + intercept;
  }

  // Free allocated memory.
  free( turn_points );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
NUMTYPE correlate( const NUMTYPE *sig_x, const NUMTYPE *sig_y,
                   unsigned int length )
{
  int i;
  NUMTYPE mean_x = 0.0, mean_y = 0.0, cov = 0.0, std_x = 0.0, std_y = 0.0;

  // Calculate each signal's mean.
  for (i = 0; i < length; i++) {
    mean_x += sig_x[i]; mean_y += sig_y[i];
  }
  mean_x /= (NUMTYPE) length; mean_y /= (NUMTYPE) length;

  // With means calculated, find the covariance and standard deviation of the
  // signals.
  for (i = 0; i < length; i++) {
    cov   += (sig_x[i] - mean_x) * (sig_y[i] - mean_y);
    std_x += (sig_x[i] - mean_x) * (sig_x[i] - mean_x);
    std_y += (sig_y[i] - mean_y) * (sig_y[i] - mean_y);
  }
  cov /= (NUMTYPE) length; std_x /= (NUMTYPE) length; std_y /= (NUMTYPE) length;
  std_x = sqrt( std_x ); std_y = sqrt( std_y );

  // Finish up the calculation for the correlation.
  return cov / (std_x * std_y);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int maxCorrelate( const Matrix *mat_S, const NUMTYPE *signal )
{
  int i, j, k, s_i;
  NUMTYPE *means, *stds, *covs, cor, max_cor;

  means = (NUMTYPE*) malloc( sizeof(NUMTYPE) * (mat_S->rows + 1) );
  stds  = (NUMTYPE*) malloc( sizeof(NUMTYPE) * (mat_S->rows + 1) );
  covs  = (NUMTYPE*) malloc( sizeof(NUMTYPE) * mat_S->rows );

  //////////////////////////////////////////////////////////////////////////////
  // Initialize everything.
  //////////////////////////////////////////////////////////////////////////////
  s_i = mat_S->rows; // Index for the 'signal' means and standard deviations.
  for (i = 0; i < mat_S->rows; i++) {
    means[i] = 0.0;
    stds[i]  = 0.0;
    covs[i]  = 0.0;
  }
  means[s_i] = stds[s_i] = 0.0;

  //////////////////////////////////////////////////////////////////////////////
  // Find the mean of every signal.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < mat_S->cols; i++) {
    for (j = 0; j < mat_S->rows; j++) {
      means[j] += mat_S->elem[ i * mat_S->ld + j ];
    }
    means[s_i] += signal[i];
  }

  for (i = 0; i < mat_S->rows + 1; i++) {
    means[i] /= (NUMTYPE) mat_S->cols;
  }

  //////////////////////////////////////////////////////////////////////////////
  // With means calculated, find the covariance of every row in `S' with the
  // given signal, and find every signal's standard deviation.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < mat_S->cols; i++) {
    for (j = 0; j < mat_S->rows; j++) {
      k = i * mat_S->ld + j;
      covs[j] += (mat_S->elem[k] - means[j]) * (signal[i] - means[s_i]);
      stds[j] += (mat_S->elem[k] - means[j]) * (mat_S->elem[k] - means[j]);
    }
    stds[s_i] += (signal[i] - means[s_i]) * (signal[i] - means[s_i]);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Find the maximum correlation value.
  //////////////////////////////////////////////////////////////////////////////
  max_cor = 0.0; k = 0;
  for (i = 0; i < mat_S->rows; i++) {
    cor = fabs(covs[i] / (stds[i] * stds[s_i]));

    if (cor > max_cor) {
      max_cor = cor;
      k = i;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Clean up and return.
  //////////////////////////////////////////////////////////////////////////////
  free( means ); free( stds ); free( covs );
  return k;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void movingAverage( NUMTYPE *output, const NUMTYPE *input,
                    unsigned int len_input )
{
  const unsigned int len_filter = 512;
  unsigned int i, j, k;
  NUMTYPE sum;

  // We are calculating a movinge average over 512 samples, with the edges
  // mirrored.

  // Handle the left edge of the input signal where we must pad the signal.
  for (i = 0; i < len_filter/2; i++) {
    sum = 0.0;
    for (j = 0; j < i + len_filter/2; j++) {
      sum += input[j] / (NUMTYPE) len_filter;
    }

    for (j = 1; j <= len_filter/2 - i; j++) {
      sum += input[j] / (NUMTYPE) len_filter;
    }
    *(output++) = sum;
  }

  // Handle the points that don't require padding of the input.
  for (; i < len_input - len_filter/2 + 1; i++) {
    sum = 0.0;
    for (j = 0; j < len_filter; j++) {
      sum += input[i - len_filter/2 + j] / (NUMTYPE) len_filter;
    }
    *(output++) = sum;
  }

  // Handle the right edge where we must once again pad the signal.
  for (; i < len_input; i++) {
    sum = 0.0;
    for (j = i - len_filter/2; j < len_input; j++) {
      sum += input[j] / (NUMTYPE) len_filter;
    }

    k = len_filter - len_input + i - len_filter/2;
    for (j = len_input - k; j < len_input-1; j++) {
      sum += input[j] / (NUMTYPE) len_filter;
    }
    *(output++) = sum;
  }
}
