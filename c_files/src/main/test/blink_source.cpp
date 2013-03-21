#include "matrix.h"
#include "ica/ica.h"
#include "numtype.h"
#include "xltek/edf.h"
#include "blink/detect.h"
#include "cmd_args/blink_source.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

/**
 * Name: main
 *
 * Description:
 * This program uses the blink source detection code to extract a blink source
 * from some EEG data. See the usage statement for details (run with the '-h'
 * option).
 */
int main( int argc, char **argv )
{
  //////////////////////////////////////////////////////////////////////////////
  // The variables.
  //////////////////////////////////////////////////////////////////////////////

  int i, edf_rc, *blinks, num_blinks, blink_source;

  // Configuration parameters.
  ICAParams ica_params;
  CmdLineArgs cmd_args;
  BlinkParams b_params;
  ICAThreadData ica_thr_data;

  // Input data stuff.
  edf_file_t *edf_file;
  edf_numtype_t edf_type = (ISDEF_USE_SINGLE ? EDF_FLOAT : EDF_DOUBLE );

  // Output data stuff.
  FILE *out_file;

  // Time measurement stuff.
  struct timeval start, stop, diff;
  double t_blink_det, t_ica, t_full;

  // Signal matrices.
  Matrix X, Wa, Aa, Sa;
  NUMTYPE *mu_Sa, *channels;

  // Multithreading stuff.
  pthread_attr_t attr;
  pthread_t ica_thread;

  //////////////////////////////////////////////////////////////////////////////
  // Do all the overhead setup (configure parameters, fetch data, etc.).
  //////////////////////////////////////////////////////////////////////////////

  // Parse the command line arguments.
  if (!parseArgs( &argc, &argv, &cmd_args )) {
    // Something went wrong.
    exit(1);
  }

  // Read in the specified EDF file in column major format because the ICA code
  // needs things that way.
  edf_file = edf_readFile( cmd_args.edf_file, EDF_COL_MAJOR );

  // Convert the EEG data from integer to floating point.
  edf_convert( edf_file, EDF_INT, edf_type );

  // Setup ICA parameters.
  ica_params.contrast = cmd_args.contrast;
  ica_params.epsilon  = cmd_args.epsilon;
  ica_params.max_iter = cmd_args.max_iter;
  ica_params.implem   = cmd_args.implem;
  ica_params.num_var  = edf_file->num_signals;
  ica_params.num_obs  = edf_file->num_samples;
  ica_params.use_gpu  = cmd_args.use_gpu;
  ica_params.gpu_device = 1;

  // Setup blink detection parameters.
  b_params.f_s = (NUMTYPE) edf_file->num_samples /
                  (NUMTYPE) strtol( edf_file->head.top.duration, NULL, 10 );
  b_params.t_1 = 15.0;
  b_params.t_cor = 0.75;

  // Initialize the signal matrices.
  if (ISDEF_USE_SINGLE) { X.elem = (NUMTYPE*) edf_file->f_samples; }
  else                  { X.elem = (NUMTYPE*) edf_file->d_samples; }
  X.rows = X.ld  = edf_file->num_signals;
  X.cols = X.lag = edf_file->num_samples;

  Wa.rows = Aa.rows = Wa.ld  = Aa.ld  = X.rows;
  Wa.cols = Aa.cols = Wa.lag = Aa.lag = X.rows;

  Sa.rows = Sa.ld  = X.rows;
  Sa.cols = Sa.lag = X.cols;

  Wa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Wa.ld * Wa.lag );
  Aa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Aa.ld * Aa.lag );
  Sa.elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Sa.ld * Sa.lag );

  mu_Sa = (NUMTYPE*) malloc( sizeof(NUMTYPE) * Sa.rows );

  // Initialize pthread stuff.
  pthread_attr_init( &attr );
  ica_thr_data.X = &X;  ica_thr_data.W = &Wa; ica_thr_data.mu_S = mu_Sa;
  ica_thr_data.S = &Sa; ica_thr_data.A = &Aa;
  ica_thr_data.ica_params = &ica_params;

  // Initialize blink detection stuff.
  channels = (NUMTYPE*) malloc( sizeof(NUMTYPE) * 4 * edf_file->num_samples );

  //////////////////////////////////////////////////////////////////////////////
  // Setup is complete. Begin processing.
  //////////////////////////////////////////////////////////////////////////////
  gettimeofday( &start, NULL );

  // Start the ICA thread and let it do it's thing. We'll join with it later.
  pthread_create( &ica_thread, &attr, icaMainThread, (void*) &ica_thr_data );
  pthread_attr_destroy( &attr );

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

    // Wait for the ICA thread to complete.
    pthread_join( ica_thread, NULL );

    // It's not a good practice, but freeing all the memory would be a pain, so
    // just exit.
    exit(1);
  }

  // Find the blinks in the EEG.
  blinks = blinkDetect( &num_blinks, channels, edf_file->num_samples, 4,
                        &b_params );

  // Record how long blink detection took.
  gettimeofday( &stop, NULL );
  timersub( &stop, &start, &diff );
  t_blink_det = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;

  // Wait for ICA to complete so we can find the blink source signal.
  pthread_join( ica_thread, NULL );

  // Record how long ICA took.
  gettimeofday( &stop, NULL );
  timersub( &stop, &start, &diff );
  t_ica = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;

  // Find the blink source.
  blink_source = blinkSource( &Sa, blinks, num_blinks, &b_params );

  // Record how long the whole algorithm took.
  gettimeofday( &stop, NULL );
  timersub( &stop, &start, &diff );
  t_full = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;

  //////////////////////////////////////////////////////////////////////////////
  // Save blink source to file and report times and source index.
  //////////////////////////////////////////////////////////////////////////////
  out_file = fopen( cmd_args.out_file, "w" );
  fprintf(out_file, "%g", Sa.elem[blink_source]);
  for (i = 1; i < Sa.cols; i++) {
    fprintf(out_file, ",%g", Sa.elem[i * Sa.rows + blink_source]);
  }
  fprintf(out_file, "\n");
  fclose(out_file);

  printf("Blink source component: #%d (0-based)\n", blink_source);
  printf("Time to detect blinks:   %g seconds\n", t_blink_det);
  printf("Time to perform ICA:     %g seconds\n", t_ica);
  printf("Time for full algorithm: %g seconds\n", t_full);

  //////////////////////////////////////////////////////////////////////////////
  // All done! Clean up and exit.
  //////////////////////////////////////////////////////////////////////////////
  free( Wa.elem );  free( Aa.elem ); free( Sa.elem ); free( mu_Sa );
  free( channels ); free( blinks );
  edf_freeFile( edf_file );

  return 0;
}
