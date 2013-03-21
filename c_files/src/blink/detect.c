#include "wavelet/wavelets.h"
#include "blink/aux.h"
#include "blink/detect.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int *blinkDetect( int *num_blinks, const NUMTYPE *channels, int len_channel,
                  int num_channels, const BlinkParams *params )
{
  int i, j, chan, blink_loc, found, window;
  int **pos_blinks, *num_pos_blinks, *iters, *blinks;

  // Pull out the sample frequency to save some typing.
  NUMTYPE f_s   = params->f_s;

  //////////////////////////////////////////////////////////////////////////////
  // Find the locations of possible blinks within each channel seperately.
  //////////////////////////////////////////////////////////////////////////////
  pos_blinks     = (int**) malloc( sizeof(int*) * num_channels );
  num_pos_blinks = (int*)  malloc( sizeof(int) * num_channels );
  for (i = 0; i < num_channels; i++) {
    pos_blinks[i] = findPossibleBlinks( num_pos_blinks + i,
                                        channels + i * len_channel,
                                        len_channel, params );
  }

  // If any of the channels had no blinks detected, then just quit here so we
  // don't need to worry about trying to dereference NULL.
  for (i = 0; i < num_channels; i++) {
    if (pos_blinks[i] == NULL) {
      // Free all allocated memory. The free() function handles NULL cleanly.
      for (chan = 0; chan < num_channels; chan++) {
        free(pos_blinks[chan]);
      }
      free(pos_blinks);
      free(num_pos_blinks);

      // Return that no blinks were found.
      (*num_blinks) = 0;
      return NULL;
    }
  }

  // Figure out what 0.02 seconds is in samples.
  window = (int) (0.02 * f_s);

  //////////////////////////////////////////////////////////////////////////////
  // Look through the set of possible blink locations and find where things are
  // in agreement.
  //////////////////////////////////////////////////////////////////////////////
  iters = (int*) calloc( num_channels, sizeof(int) );
  (*num_blinks) = num_pos_blinks[0];
  for (i = 0; i < num_pos_blinks[0]; i++) {
    blink_loc = pos_blinks[0][i];

    for (chan = 1; chan < num_channels; chan++) {
      found = 0;
      for (j = iters[chan]; j < num_pos_blinks[chan]; j++) {
        if (abs(blink_loc - pos_blinks[chan][j]) <= window) {
          // A blink was found in channel 'chan' that matches. Update its
          // iterator so that we don't double count any blinks.
          iters[chan]++;
          found = 1;
        }
      }

      // If no match was found in the channel, then there's no need to search
      // the other channels, and we should mark the test blink for removal
      // later.
      if (!found) {
        pos_blinks[0][i] = -1;
        (*num_blinks)--;
        break;
      }
    }
  }

  // If any blinks are still left that matched, then we should create a new
  // array for those blinks and return that array.
  if ((*num_blinks) > 0) {
    blinks = (int*) malloc( sizeof(int) * (*num_blinks) );

    j = 0;
    for (i = 0; i < num_pos_blinks[0]; i++) {
      if (pos_blinks[0][i] >= 0) {
        blinks[j] = pos_blinks[0][i];
        j++;
      }
    }
  } else {
    (*num_blinks) = 0;
    blinks = NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Free up the memory allocated by this function and return.
  //////////////////////////////////////////////////////////////////////////////
  for (i = 0; i < num_channels; i++) {
    free(pos_blinks[i]);
  }
  free(pos_blinks);
  free(num_pos_blinks);
  free(iters);

  return blinks;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int blinkSource( const Matrix *mat_S, const int *blinks, int len_blinks,
                 const BlinkParams *params )
{
  int i, steps_1, len_template, eb_start, template_start, length, source;
  NUMTYPE *eb_template, *templ;

  eb_template = (NUMTYPE*) calloc( mat_S->cols, sizeof(NUMTYPE) );

  //////////////////////////////////////////////////////////////////////////////
  // Build the template.
  //////////////////////////////////////////////////////////////////////////////
  templ = getTemplate( &steps_1, &len_template, params );

  for (i = 0; i < len_blinks; i++) {
    eb_start = blinks[i] - steps_1;

    // If the blink occurs too close the the edge of the signal, we need to
    // adjust where we start copying the template over.
    if (eb_start < 0) { template_start = -eb_start; eb_start = 0; }
    else              { template_start = 0; }

    // We also need to make sure that we don't access beyond the end of the
    // signal.
    if ((eb_start + len_template) >= mat_S->cols) {
      length = len_template - (eb_start + len_template - mat_S->cols);
    } else {
      length = len_template - template_start;
    }

    // Copy the eyeblink template to the blink source template signal.
    memcpy( eb_template + eb_start, templ + template_start,
            sizeof(NUMTYPE) * length );
  }

  //////////////////////////////////////////////////////////////////////////////
  // Find the signal with maximum absolute correlation with the template signal.
  //////////////////////////////////////////////////////////////////////////////
  source = maxCorrelate( mat_S, eb_template );

  free( eb_template );
  free( templ );

  return source;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int *blinksInSource( int *num_blinks, const Matrix *mat_S, int blink_source,
                     const int *blinks, unsigned int len_blinks,
                     const BlinkParams *params )
{
  int i, j, above, num_indices, max_index, window, blinks_iter, found;
  int *indices, *retval;

  NUMTYPE max, f_s, thresh, *source;

  // Setup the threshold with an experimentally determined value that seems to
  // have good results.
  thresh = 10.0 / len_blinks;

  // Pull out the sampling frequency.
  f_s = params->f_s;

  // Extract the blink source signal from the source signal matrix.
  source = (NUMTYPE*) malloc( sizeof(NUMTYPE) * mat_S->cols );

  for (i = 0; i < mat_S->cols; i++) {
    source[i] = mat_S->elem[ mat_S->ld * i + blink_source ];
  }

  //////////////////////////////////////////////////////////////////////////////
  // Find the locations of blinks within the source signal.
  //////////////////////////////////////////////////////////////////////////////

  // Find the maximum absolute value of the source signal between crossings of
  // the threshold value.
  above = 0; num_indices = 0; max_index = 0; max = 0.0; indices = NULL;
  for (i = 1; i < mat_S->cols; i++) {
    // Check to see if we crossed the threshold.
    if (fabs(source[i]) > thresh && fabs(source[i-1]) <= thresh) {
      above = 1;

      // Reset the 'max' value so that we can find a new maximum.
      max = 0.0;
    } else if (fabs(source[i]) < thresh && fabs(source[i-1]) >= thresh) {
      above = 0;

      // We just found the falling edge of a pair. Record the index of the
      // maximum value that we found within the pair.
      num_indices++;
      indices = (int*) realloc( indices, sizeof(int) * num_indices );
      indices[num_indices-1] = max_index;
    }

    // If we're above the threshold, keep track of the maximum value that we
    // see.
    if (above) {
      if (max < fabs(source[i])) {
        max = fabs(source[i]);
        max_index = i;
      }
    }
  }

  // If no blinks were detected in the source, then just return now.
  if (num_indices == 0) {
    (*num_blinks) = 0;
    return NULL;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Eliminate blink locations that do not agree with those found in the EEG.
  //////////////////////////////////////////////////////////////////////////////

  // The maximums that we've found correspond to blink locations within the
  // blink source. Eliminate all locations that do not agree within 0.02 seconds
  // with the blinks detected in the original EEG.

  // Figure out what 0.02 seconds is in samples.
  window = (int) (0.02 * f_s);

  blinks_iter = 0;
  (*num_blinks) = num_indices;
  for (i = 0; i < num_indices; i++) {
    found = 0;

    for (j = blinks_iter; j < len_blinks; j++) {
      if (abs(indices[i] - blinks[j]) <= window) {
        // A blink was found that matches. Update the blinks interator so that
        // we don't double count any blinks.
        blinks_iter = j+1;
        found = 1;
        break;
      }
    }

    // If no match was found, then we should mark the test blink for removal
    // later.
    if (!found) {
      indices[i] = -1;
      (*num_blinks)--;
    }
  }

  // If there are any blinks remaining, make a list out of them and return it.
  if ((*num_blinks) == 0) {
    retval = NULL;
  } else {
    retval = (int*) malloc( sizeof(int) * (*num_blinks) );

    blinks_iter = 0;
    for (i = 0; i < num_indices; i++) {
      if (indices[i] != -1) {
        retval[blinks_iter] = indices[i];
        blinks_iter++;
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Free allocated memory and return.
  //////////////////////////////////////////////////////////////////////////////
  free(source); free(indices);
  return retval;
}
