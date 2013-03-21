#include "cmd_args/blink_source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( const char *name )
{
  printf( "Usage:\n"
"  %s [options] -e EDF_FILE -o OUT_FILE\n"
"\n"
"  DESCRIPTION:\n"
"\n"
"    Using the blink source detection code, this program processes EEG data\n"
"    given in the specified EDF_FILE and prints the extracted blink source\n"
"    to OUT_FILE as a single row of comma seperated values.\n"
"\n"
"    If no blinks are detected in the EEG data, no file is printed.\n"
"\n"
"    As this program makes use of independent component analysis, there are\n"
"    several options for ICA's configuration.\n"
"\n"
"  OPTIONS:\n"
"\n"
"    -h, --help\n"
"        Print this usage statement.\n"
"\n"
"    -c, --contrast FUNCTION\n"
"        Which contrast function to use (default 'tanh'). One of:\n"
"          tanh, cube, gauss\n"
"\n"
"    -e, --edf-file EDF_FILE\n"
"        The name of the EDF file from which to read EEG data.\n"
"\n"
"    -ep, --epsilon NUM\n"
"        Convergence criteria epsilon to use (default 0.0001).\n"
"\n"
"    -i, --implementation TYPE\n"
"        Which implementation to use (default 'fastica'). One of:\n"
"          fastica, jade\n"
"        If jade is specified, the contrast, epsilon and iteration options\n"
"        are unused.\n"
"\n"
"    -mi, --max_iterations NUM\n"
"        The maximum number of iterations to perform (default 400).\n"
"\n"
"    -o, --out-file OUT_FILE\n"
"        The name of the file to which to save the extracted source signal.\n"
"\n"
#ifdef ENABLE_GPU
"    -g, --gpu\n"
"        Use the GPU implementation of ICA.\n"
#endif
"\n"
,name );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int parseArgs( int *argc, char ***argv, CmdLineArgs *cmd_args )
{
  FILE *file;
  int i;

  // Setup default values.
  cmd_args->edf_file   = NULL;
  cmd_args->out_file   = NULL;
  cmd_args->contrast   = NONLIN_TANH;
  cmd_args->epsilon    = 0.0001;
  cmd_args->max_iter   = 400;
  cmd_args->implem     = ICA_FASTICA;
  cmd_args->use_gpu    = 0;

  // Define a macro to make parameter comparison cleaner in the code.
  #define PARAM_EQUALS( sn, ln ) (strcmp( (sn), (*argv)[i] ) == 0 ||\
                                  strcmp( (ln), (*argv)[i] ) == 0)

  for (i = 1; i < *argc;) {
    if (strncmp("-", (*argv)[i], 1) == 0) {
      // Argument is an option.
      if (PARAM_EQUALS( "-c", "--contrast" )) {
        if (strcmp( "tanh", (*argv)[i+1] ) == 0) {
          cmd_args->contrast = NONLIN_TANH;
        } else if (strcmp( "cube", (*argv)[i+1] ) == 0) {
          cmd_args->contrast = NONLIN_CUBE;
        } else if (strcmp( "gauss", (*argv)[i+1] ) == 0) {
          cmd_args->contrast = NONLIN_GAUSS;
        } else {
          fprintf(stderr, "Unknown contrast method, '%s'.", (*argv)[i+1]);
          return 0;
        }

        i += 2;
      } else if (PARAM_EQUALS( "-e", "--edf-file")) {
        cmd_args->edf_file = (*argv)[i+1];

        if ((file = fopen( cmd_args->edf_file, "r" )) == NULL) {
          fprintf(stderr,"Failed to open EDF file '%s'!\n",cmd_args->edf_file);
          return 0;
        }
        fclose(file);
        i += 2;
      } else if (PARAM_EQUALS("-ep", "--epsilon")) {
        cmd_args->epsilon = strtod( (*argv)[i+1], NULL );

        if (cmd_args->epsilon <= 0) {
          fprintf(stderr, "Epsilon value, %g, invalid. Must be > 0.",
                  cmd_args->epsilon);
          return 0;
        }

        i += 2;
      } else if (PARAM_EQUALS("-i", "--implementation")) {
        if (strcmp( "jade", (*argv)[i+1] ) == 0) {
          cmd_args->implem = ICA_JADE;
        } else if (strcmp( "fastica", (*argv)[i+1] ) != 0) {
          fprintf(stderr, "Implementation value, %s, invalid. "
                          "Must be one of 'jade' or 'fastica'.\n",
                          (*argv)[i+1]);
          return 0;
        }

        i += 2;
      } else if (PARAM_EQUALS( "-mi", "--max_iterations")) {
        cmd_args->max_iter = atoi( (*argv)[i+1] );

        if (cmd_args->max_iter <= 0) {
          fprintf(stderr,
                  "Invalid maximum number of iterations, %d, must be > 0.",
                  cmd_args->max_iter);
          return 0;
        }

        i += 2;
#ifdef ENABLE_GPU
      } else if (PARAM_EQUALS("-g", "--gpu")) {
        cmd_args->use_gpu = 1;
        i += 1;
#endif
      } else if (PARAM_EQUALS( "-o", "--out-file")) {
        cmd_args->out_file = (*argv)[i+1];

        if ((file = fopen( cmd_args->out_file, "w" )) == NULL) {
          fprintf(stderr,"Failed to open out-file '%s'!\n",cmd_args->out_file);
          return 0;
        }
        fclose(file);
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

  // Make sure that both an EDF and output file were specified.
  if (cmd_args->edf_file == NULL || cmd_args->out_file == NULL) {
    usage( (*argv)[0] );
    return 0;
  }

  // Update argc and argv to reflect the parameters that have already been
  // parsed.
  (*argc) -= (i - 1);

  (*argv)[i - 1] = (*argv)[0];
  (*argv) += (i - 1);

  return 1;
}
