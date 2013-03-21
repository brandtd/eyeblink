#include "cmd_args/runtime.h"

#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void usage( const char *name )
{
  printf(
"USAGE:\n"
"  %s OPTIONS\n"
"\n"
"Gathers data relating the size of an observation matrix with the runtime of\n"
"each implementation of ICA. Only worst case runtimes are explored. Runtime\n"
"measurements do not include any setup time an implementation may require,\n"
"and are gathered by attempting to separate a set of gaussian sources.\n"
"\n"
"This program will report the average and standard deviation of the runtime\n"
"of each implementation, for several observation matrix sizes, over the given\n"
"number of samples. If the number of samples is not specified, it defaults\n"
"to 20.\n"
"\n"
"OPTIONS:\n"
"  -d, --demo\n"
"     Show sample output.\n"
"\n"
"  -s, --samples NUM\n"
"     Specify the number of samples to obtain for calculating the average and\n"
"     standard deviation.\n"
"\n"
, name );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
unsigned int parseArgs( int *argc, char ***argv, CmdLineArgs *cmd_args )
{
  int i;

  // Setup default values.
  cmd_args->samples = 20;
  cmd_args->demo    = 0;

  // Define a macro to make parameter comparison cleaner in the code.
  #define PARAM_EQUALS( sn, ln ) (strcmp( (sn), (*argv)[i] ) == 0 ||\
                                  strcmp( (ln), (*argv)[i] ) == 0)

  for (i = 1; i < *argc; i++) {
    if (PARAM_EQUALS("-h", "--help")) {
      usage( (*argv)[0] );
      return 0;
    } else if (PARAM_EQUALS( "-d", "--demo" )) {
      cmd_args->demo = 1;
      i++;
    } else if (PARAM_EQUALS( "-s", "--samples" )) {
      cmd_args->samples = atoi( (*argv)[i+1] );
      if (cmd_args->samples <= 0) {
        usage( (*argv)[0] );
        return 0;
      }
      i += 2;
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
