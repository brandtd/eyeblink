#include "cmd_args/ica.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( const char *name )
{
  printf( "Usage:\n"
"  %s [options] X_FILE [X_FILE ...]\n"
"\n"
"  DESCRIPTION:\n"
"\n"
"    Performs independent component analysis on the given observation matrix\n"
"    (given in X_FILE) and reports runtime of the algorithm.\n"
"\n"
"    Multiple matrix files may be specified, at which point one runtime will\n"
"    be reported for each generated observation matrix.\n"
"\n"
"    The matrix files are expected to be comma seperated value (CSV) files,\n"
"    given in column-major order (each line should be one column).\n"
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
"    -e, --epsilon NUM\n"
"        Convergence criteria epsilon to use (default 0.0001).\n"
"\n"
"    -i, --implementation TYPE\n"
"        Which implementation to use (default 'fastica'). One of:\n"
"          fastica, jade\n"
"\n"
"    -mi, --max_iterations NUM\n"
"        The maximum number of iterations to perform (default 400).\n"
"\n"
#ifdef ENABLE_GPU
"    -g, --gpu\n"
"        Run only the GPU implementation of ICA.\n"
"\n"
"    -k, --compare\n"
"        Compare the CPU and GPU implementations of ICA, printing runtime\n"
"        info for both.\n"
"\n"
#endif
"    -p, --print\n"
"        Print the resulting unmixing, mixing, and source signal matrices.\n"
#ifdef ENABLE_GPU
"        GPU matrices will be printed to Wgpu.csv, Agpu.csv, and Sgpu.csv,\n"
"        while CPU matrices will be printed to Wcpu.csv, etc.\n"
#endif
"\n"
,name );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int parseArgs( int *argc, char ***argv, CmdLineArgs *cmd_args )
{
  int i;

  // Setup default values.
  cmd_args->max_iter   = 400;
  cmd_args->epsilon    = 0.0001;
  cmd_args->contrast   = NONLIN_TANH;
  cmd_args->implem     = ICA_FASTICA;
  cmd_args->gpu_only   = 0;
  cmd_args->compare    = 0;
  cmd_args->print      = 0;

  // Define a macro to make parameter comparison cleaner in the code.
  #define PARAM_EQUALS( sn, ln ) (strcmp( (sn), (*argv)[i] ) == 0 ||\
                                  strcmp( (ln), (*argv)[i] ) == 0)

  for (i = 1; i < *argc;) {
    if (strncmp("-", (*argv)[i], 1) == 0) {
      // Argument is an option.
      if (PARAM_EQUALS( "-mi", "--max_iterations")) {
        cmd_args->max_iter = atoi( (*argv)[i+1] );

        if (cmd_args->max_iter <= 0) {
          fprintf(stderr,
                  "Invalid maximum number of iterations, %d, must be > 0.",
                  cmd_args->max_iter);
          return 0;
        }

        i += 2;
      } else if (PARAM_EQUALS("-c", "--constrast")) {
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
      } else if (PARAM_EQUALS("-e", "--epsilon")) {
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
#ifdef ENABLE_GPU
      } else if (PARAM_EQUALS("-g", "--gpu")) {
        cmd_args->gpu_only = 1;
        i += 1;
      } else if (PARAM_EQUALS("-k", "--compare")) {
        cmd_args->compare = 1;
        i += 1;
#endif
      } else if (PARAM_EQUALS("-p", "--print")) {
        cmd_args->print = 1;
        i += 1;
      } else {
        usage( (*argv)[0] );
        return 0;
      }
    } else {
      // We're done seeing options, the remaining arguments are the file names.
      break;
    }
  }

  // Update argc and argv to reflect the parameters that have already been
  // parsed.
  (*argc) -= (i - 1);

  (*argv)[i - 1] = (*argv)[0];
  (*argv) += (i - 1);

  return 1;
}
