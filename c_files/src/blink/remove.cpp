#include "matrix.h"
#include "numtype.h"
#include "xltek/edf.h"
#include "blink/aux.h"
#include "blink/detect.h"
#include "blink/remove.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void blinkRemoveFromEDF( const char *input_edf, const char *output_edf,
                         ICAParams *ica_params )
{
  int edf_rc;
  Matrix X, R;
  NUMTYPE *channels;

  BlinkParams b_params;

  edf_file_t *edf_file;
  edf_numtype_t edf_type = (ISDEF_USE_SINGLE ? EDF_FLOAT : EDF_DOUBLE);

  //////////////////////////////////////////////////////////////////////////////
  // Initialize everything.
  //////////////////////////////////////////////////////////////////////////////

  // Get the input EDF file.
  edf_file = edf_readFile( input_edf, EDF_COL_MAJOR );
  edf_convert( edf_file, EDF_INT, edf_type );

  // Finish up initializing ICA parameters now that we know the size of the
  // data.
  ica_params->num_var  = edf_file->num_signals;
  ica_params->num_obs  = edf_file->num_samples;

  // Setup the signal matrices.
  if (ISDEF_USE_SINGLE) { X.elem = (NUMTYPE*) edf_file->f_samples; }
  else                  { X.elem = (NUMTYPE*) edf_file->d_samples; }
  R.rows = R.ld  = X.rows = X.ld  = edf_file->num_signals;
  R.cols = R.lag = X.cols = X.lag = edf_file->num_samples;
  R.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * R.ld * R.lag );

  // Setup blink detection parameters.
  b_params.f_s = (NUMTYPE) edf_file->num_samples /
                 (NUMTYPE) strtol( edf_file->head.top.duration, NULL, 10 );
  b_params.t_1 = 15.0;
  b_params.t_cor = 0.75;
  channels = (NUMTYPE*) malloc( sizeof(NUMTYPE) * 4 * edf_file->num_samples );

  //////////////////////////////////////////////////////////////////////////////
  // Setup is complete. Remove the blinks.
  //////////////////////////////////////////////////////////////////////////////

  // Calculate the 'FP1 - F3', 'FP1 - F7', 'FP2 - F4', and 'FP2 - F8' channels.
  edf_rc  = edf_diffChannels( channels + edf_file->num_samples * 0,
                              "FP1", "F3", edf_file, edf_type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 1,
                              "FP1", "F7", edf_file, edf_type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 2,
                              "FP2", "F4", edf_file, edf_type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 3,
                              "FP2", "F8", edf_file, edf_type );

  // Make sure the channels were found.
  if (!edf_rc) {
    fprintf(stderr, "Failed to find FP1, FP2, F3, F4, F7, and F8 channels!\n");
    exit(1);
  }

  // Find and remove the blinks in the EEG.
  blinkRemove( &R, &X, channels, 4, NULL, 0, ica_params, &b_params );

  //////////////////////////////////////////////////////////////////////////////
  // Save the new EEG data to an EDF file.
  //////////////////////////////////////////////////////////////////////////////

  if (ISDEF_USE_SINGLE) { free(edf_file->f_samples);
                          edf_file->f_samples = (float*)  R.elem; }
  else                  { free(edf_file->d_samples);
                          edf_file->d_samples = (double*) R.elem; }
  edf_convert( edf_file, edf_type, EDF_INT ); // This will free(3) X.elem.
  edf_saveToFile( output_edf, edf_file );

  //////////////////////////////////////////////////////////////////////////////
  // Clean up and return.
  //////////////////////////////////////////////////////////////////////////////
  edf_freeFile( edf_file );
  free( channels );
  // X.elem was free(3)'d by the call to edf_convert, and R.elem by the call to
  // edf_freeFile().

  return;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int blinkRemove( Matrix *mat_R, const Matrix *mat_X,
                 const NUMTYPE *channels, int num_channels,
                 const int *keep, int num_keep,
                 ICAParams *ica_params, const BlinkParams *b_params )
{
  int i, j, k, restore, num_blinks, blink_source;
  int *blinks, *blinks_in_source;

  Matrix Wa, Aa, Sa;
  NUMTYPE *mu_Sa, *mu_X;

  pthread_attr_t attr;
  pthread_t ica_thread;
  ICAThreadData ica_thr_data;

  //////////////////////////////////////////////////////////////////////////////
  // Initialize everything.
  //////////////////////////////////////////////////////////////////////////////

  ica_params->num_var = mat_X->rows;
  ica_params->num_obs = mat_X->cols;

  // Setup the signal matrices.
  Wa.rows = Aa.rows = Wa.ld  = Aa.ld  = mat_X->rows;
  Wa.cols = Aa.cols = Wa.lag = Aa.lag = mat_X->rows;

  Sa.rows = Sa.ld  = mat_X->rows;
  Sa.cols = Sa.lag = mat_X->cols;

  Wa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Wa.ld * Wa.lag );
  Aa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Aa.ld * Aa.lag );
  Sa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Sa.ld * Sa.lag );

  mu_Sa = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Sa.rows );
  mu_X  = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Sa.rows );

  // Initialize pthread stuff.
  pthread_attr_init( &attr );
  ica_thr_data.X = mat_X;   ica_thr_data.W = &Wa; ica_thr_data.mu_S = mu_Sa;
  ica_thr_data.S = &Sa; ica_thr_data.A = &Aa;
  ica_thr_data.ica_params = ica_params;

  //////////////////////////////////////////////////////////////////////////////
  // Setup is complete. Begin processing.
  //////////////////////////////////////////////////////////////////////////////

  // Start the ICA thread doing its thing.
  pthread_create( &ica_thread, &attr, icaMainThread, (void*) &ica_thr_data );
  pthread_attr_destroy( &attr );

  // Find the blinks in the EEG.
  blinks = blinkDetect( &num_blinks, channels, mat_X->cols, 4, b_params );

  // Wait for ICA to complete so we can find the blink source signal.
  pthread_join( ica_thread, NULL );

  // Find the blink source.
  blink_source = blinkSource( &Sa, blinks, num_blinks, b_params );

  // Find the blinks in the blink source component.
  blinks_in_source = blinksInSource( &num_blinks, &Sa, blink_source, blinks,
                                     num_blinks, b_params );

  //////////////////////////////////////////////////////////////////////////////
  // Flatten the blink source for 0.4 seconds centered around the locations of
  // blinks.
  //////////////////////////////////////////////////////////////////////////////
  blinkFlatten( &Sa, blinks_in_source, num_blinks, blink_source, b_params );

  //////////////////////////////////////////////////////////////////////////////
  // Reconstruct the EEG with the modified blink source.
  //////////////////////////////////////////////////////////////////////////////
  GEMM( (*mat_R), Aa, Sa );
  GEMV( mu_X, Aa, mu_Sa );

  // Add the mean back in, and restore any rows we were told to 'keep'.
  for (i = 0; i < mat_R->cols; i++) {
    for (j = 0; j < mat_R->rows; j++) {
      // Check to see if we should restore the row.
      restore = 0;
      for (k = 0; k < num_keep; k++) {
        if (j == keep[k]) { restore = 1; break; }
      }

      if (restore) {
        mat_R->elem[ i * mat_R->ld + j ]  = mat_X->elem[ i * mat_X->ld + j ];
      } else {
        mat_R->elem[ i * mat_R->ld + j ] += mu_X[j];
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Clean up and return.
  //////////////////////////////////////////////////////////////////////////////
  free( Wa.elem ); free( Aa.elem ); free( Sa.elem );
  free( mu_Sa ); free( mu_X );
  free( blinks ); //free( blinks_in_source );

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void blinkFlatten( Matrix *mat_S, const int *blinks, int num_blinks, int row,
                   const BlinkParams *b_params )
{
  int i, j, j_low, j_high, window;
  NUMTYPE val;

  window = (int) (0.2 * b_params->f_s);

  for (i = 0; i < num_blinks; i++) {
    j_low  = blinks[i] - window;
    j_high = blinks[i] + window;

    if (j_low < 0) {
      // If the blink occurs near the beginning of the signal, make the source
      // assume the value directly after the blink.
      val = mat_S->elem[ j_high * mat_S->ld + row ];
      j_low = 0;
    } else if (j_high >= mat_S->cols) {
      // If the blink occurs near the end of the signal, make the source assume
      // the value directly before the blink.
      val = mat_S->elem[ j_low * mat_S->ld + row ];
      j_high = mat_S->cols - 1;
    } else {
      // Otherwise, make the source assume the average of the value directly
      // before and directly after the blink.
      val = 0.5 * (mat_S->elem[ j_low  * mat_S->ld + row ] +
                   mat_S->elem[ j_high * mat_S->ld + row ]);
    }

    for (j = j_low; j < j_high; j++) {
      mat_S->elem[ j * mat_S->ld + row ] = val;
    }
  }
}
