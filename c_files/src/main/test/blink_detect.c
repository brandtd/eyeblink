#include "numtype.h"
#include "xltek/edf.h"
#include "blink/detect.h"
#include "cmd_args/blink_detect.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/**
 * Name: main
 *
 * Description:
 * This program verifies the correct operation of the blink detection code and
 * prints runtime information about the program. See the usage statement for
 * details (run with the '-h' option).
 */
int main( int argc, char **argv )
{
  int i, j, false_det, missed_det, window, edf_rc, *blinks, num_blinks;
  int found;

  CmdLineArgs cmd_args;
  BlinkParams b_params;

  edf_file_t *edf_file;
  edf_numtype_t type;

  NUMTYPE *channels;

  //////////////////////////////////////////////////////////////////////////////
  // Parse command line arguments.
  //////////////////////////////////////////////////////////////////////////////
  if (!parseArgs( &argc, &argv, &cmd_args )) {
    // Something bad happened.
    exit(1);
  }

  // Read in the specified EDF file in row major format because the blink
  // detection almost all operate on single rows.
  edf_file = edf_readFile( cmd_args.edf_file, EDF_ROW_MAJOR );

  //////////////////////////////////////////////////////////////////////////////
  // Calculate the 'FP1 - F3', 'FP1 - F7', 'FP2 - F4', and 'FP2 - F8' channels.
  //////////////////////////////////////////////////////////////////////////////

  // Make sure the samples get converted to the correct type.
  if (ISDEF_USE_SINGLE) {
    type = EDF_FLOAT;
  } else {
    type = EDF_DOUBLE;
  }

  // Allocate space for four channels' worth of EEG data.
  channels = (NUMTYPE*) malloc(sizeof(NUMTYPE)* 4 * edf_file->num_samples);

  edf_rc  = edf_diffChannels( channels + edf_file->num_samples * 0,
                              "FP1", "F3", edf_file, type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 1,
                              "FP1", "F7", edf_file, type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 2,
                              "FP2", "F4", edf_file, type );
  edf_rc &= edf_diffChannels( channels + edf_file->num_samples * 3,
                              "FP2", "F8", edf_file, type );

  // Make sure that all channels could be found.
  if (!edf_rc) {
    free(channels);
    exit(1);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Find the blinks in the EEG.
  //////////////////////////////////////////////////////////////////////////////

  // Figure out the parameters to pass to the detection algorithm.
  b_params.f_s = (NUMTYPE) edf_file->num_samples /
                  (NUMTYPE) strtol( edf_file->head.top.duration, NULL, 10 );
  b_params.t_1 = 15.0;
  b_params.t_cor = 0.75;

  // Perform the actual detection.
  blinks = blinkDetect( &num_blinks, channels, edf_file->num_samples, 4,
                        &b_params );

  //////////////////////////////////////////////////////////////////////////////
  // Print the false detections.
  //////////////////////////////////////////////////////////////////////////////

  printf( "\nExpected %d blink(s), found %d blink(s).\n\n", cmd_args.num_blinks,
          num_blinks );

  // Figure out what 0.02 seconds is in samples.
  window = (int) (0.02 * b_params.f_s);

  // Print the false detections.
  printf( "False detections at:  " );
  false_det = 0;
  for (i = 0; i < num_blinks; i++) {
    found = 0;

    // See if the found blink matches any of the expected blinks.
    for (j = 0; j < cmd_args.num_blinks; j++) {
      if (abs(blinks[i] - cmd_args.blinks[j]) <= window) { found = 1; break; }
    }

    // If it does not match any expected blink, record it as a false detection.
    if (!found) { printf(" %d", blinks[i]); false_det++; }
  }

  if (false_det == 0) { printf(" --\n"); }
  else                { printf("\n"); }

  printf( "False detection count: %d\n\n", false_det );

  //////////////////////////////////////////////////////////////////////////////
  // Print the missed detections.
  //////////////////////////////////////////////////////////////////////////////

  printf( "Missed detections at:  " );
  missed_det = 0;
  for (i = 0; i < cmd_args.num_blinks; i++) {
    found = 0;

    // See if the expected blink matches any of the found blinks.
    for (j = 0; j < num_blinks; j++) {
      if (abs(cmd_args.blinks[i] - blinks[j]) <= window) { found = 1; break; }
    }

    // If it does not match any found blinks, record it as a missed detection.
    if (!found) { printf(" %d", cmd_args.blinks[i]); missed_det++; }
  }

  if (missed_det == 0) { printf(" --\n"); }
  else                 { printf("\n"); }

  printf( "Missed detection count: %d\n\n", missed_det );

  //////////////////////////////////////////////////////////////////////////////
  // All done! Clean up and exit.
  //////////////////////////////////////////////////////////////////////////////

  edf_freeFile( edf_file );
  free(channels);
  free(blinks);
  return 0;
}
