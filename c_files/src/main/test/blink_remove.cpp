#include "blink/remove.h"
#include "cmd_args/blink_remove.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/**
 * Name: main
 *
 * Description:
 * This program uses the blink source detection code to remove blinks from EEG
 * data. See the usage statement for details (run with the '-h' option).
 */
int main( int argc, char **argv ){

  CmdLineArgs cmd_args;
  ICAParams ica_params;

  struct timeval start, stop, diff;
  double t_process;

  if (!parseArgs( &argc, &argv, &cmd_args )) {
    // Something went wrong.
    exit(1);
  }

  // Build the ICA parameters.
  ica_params.contrast = cmd_args.contrast;
  ica_params.epsilon  = cmd_args.epsilon;
  ica_params.max_iter = cmd_args.max_iter;
  ica_params.implem   = cmd_args.implem;
  ica_params.use_gpu  = cmd_args.use_gpu;
  ica_params.gpu_device = 1;

  // Perform the actual work.
  gettimeofday( &start, NULL );
    blinkRemoveFromEDF( cmd_args.in_file, cmd_args.out_file, &ica_params );
  gettimeofday( &stop, NULL );
  timersub( &stop, &start, &diff );

  t_process = (double) (diff.tv_sec) + (double) (diff.tv_usec) * 0.000001;
  printf( "\n" );
  printf( "Time to process (including file i/o): %1.4f seconds\n", t_process );
  printf( "\n" );

  // Shutdown ICA.
  ica_shutdown();

  return 0;
}
