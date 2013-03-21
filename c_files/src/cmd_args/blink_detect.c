#include "cmd_args/blink_detect.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( const char *name )
{
  printf( "Usage:\n"
"  %s [-h] -e EDF_FILE [-b BLINK [-b BLINK [...]]]\n"
"\n"
"  DESCRIPTION:\n"
"\n"
"    Verifies the operation of the blink detection code by reading in EEG\n"
"    data from the specified EDF_FILE and verifying that the blink detection\n"
"    code reports blinks within 0.02 seconds of the BLINK locations specified\n"
"    when calling the program.\n"
"\n"
"  OPTIONS:\n"
"\n"
"    -h, --help\n"
"        Print this usage statement.\n"
"\n"
"    -e, --edf-file EDF_FILE\n"
"        The name of the EDF file from which to read EEG data.\n"
"\n"
"    -b, --blink NUM\n"
"        The location in samples of blinks within the EEG data. For example,\n"
"        '-b 3214' states that a blink should be detected as occuring within\n"
"        0.02 seconds of sample 3214 (1-based). The value of '0.02 seconds'\n"
"        in samples is determined by examining the sampling frequency given\n"
"        by the EDF file.\n"
"\n"
,name );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int parseArgs( int *argc, char ***argv, CmdLineArgs *cmd_args )
{
  FILE *edf_file;
  int i;

  // Setup default values.
  cmd_args->edf_file   = NULL;
  cmd_args->blinks     = NULL;
  cmd_args->num_blinks = 0;

  // Define a macro to make parameter comparison cleaner in the code.
  #define PARAM_EQUALS( sn, ln ) (strcmp( (sn), (*argv)[i] ) == 0 ||\
                                  strcmp( (ln), (*argv)[i] ) == 0)

  for (i = 1; i < *argc;) {
    if (strncmp("-", (*argv)[i], 1) == 0) {
      // Argument is an option.
      if (PARAM_EQUALS( "-e", "--edf-file")) {
        cmd_args->edf_file = (*argv)[i+1];

        if ((edf_file = fopen( cmd_args->edf_file, "r" )) == NULL) {
          fprintf(stderr,"Failed to open EDF file '%s'!\n",cmd_args->edf_file);
          return 0;
        }
        fclose(edf_file);
        i += 2;
      } else if (PARAM_EQUALS("-b", "--blink")) {
        cmd_args->num_blinks++;
        cmd_args->blinks = realloc( cmd_args->blinks,
                                    sizeof(int) * cmd_args->num_blinks );

        cmd_args->blinks[cmd_args->num_blinks - 1] = atoi( (*argv)[i+1] ) - 1;

        if (cmd_args->blinks[cmd_args->num_blinks - 1] <= 0) {
          fprintf(stderr, "Blink indices must be greater than 0!\n");
          return 0;
        }

        i += 2;
      } else {
        usage( (*argv)[0] );
        return 0;
      }
    } else {
      usage( (*argv)[0] );
      return 0;
    }
  }

  // Update argc and argv to reflect the parameters that have already been
  // parsed.
  (*argc) -= (i - 1);

  (*argv)[i - 1] = (*argv)[0];
  (*argv) += (i - 1);

  return 1;
}
