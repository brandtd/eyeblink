#ifndef RUNTIME_H
#define RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct to store command line arguments. See the usage statement for an
 * explanation.
 */
typedef struct CmdLineArgs {
  int demo;
  int samples;
} CmdLineArgs;

/**
 * Name: usage
 *
 * Description:
 * Prints a usage statement for this program.
 *
 * Parameters:
 * @param name      argv[0]
 */
void usage( const char *name );

/**
 * Name: parseArgs
 *
 * Description:
 * Parses command line arguments, first setting up default values and then
 * reading the given parameters and modifying those arguments to match the
 * parameters.
 *
 * This function should be passed the argc and argv variables given to main(),
 * and it will modify them to remove the command line parameters switches,
 * living just the file names.
 *
 * If the '-h' or '--help' parameter is given, then this function will return
 * 0, otherwise, a nonzero value is returned.
 *
 * Parameters:
 * @param argc      the 'argc' parameter given to main()
 * @param argv      the 'argv' parameter given to main()
 * @param cmd_args  where to store the command line argument values
 *
 * Returns:
 * @return unsigned int   0 if '-h' or '--help' is given, nonzero otherwise
 */
unsigned int parseArgs( int *argc, char ***argv, CmdLineArgs *cmd_args );

#ifdef __cplusplus
}
#endif

#endif
