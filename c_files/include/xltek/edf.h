#ifndef EDF_H
#define EDF_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Structs defining an EDF header.
//
// These structs uses the same storage format as the EDF format itself, i.e.,
// they store all information as ASCII strings rather than any native format.
////////////////////////////////////////////////////////////////////////////////
typedef struct edf_header_top_t { // Define the fixed size part of the struct
  char version[8];                // so that reading/writing this stuff in is a
  char patient_id[80];            // whole lot easier.
  char recording_id[80];
  char startdate[8];
  char starttime[8];
  char num_bytes[8];
  char reserved_0[44];
  char num_records[8];
  char duration[8];
  char num_signals[4];
} edf_header_top_t;

typedef struct edf_header_t {
  edf_header_top_t top;

  // For the following fields, there must be one string per signal.
  char **labels;
  char **transducers;
  char **phys_dim;
  char **phys_min;
  char **phys_max;
  char **dig_min;
  char **dig_max;
  char **prefilter;
  char **samples_per_record;
  char **reserved_1;
} edf_header_t;

/**
 * Enum used to tell the edf functions how to store/access sample values. The
 * EDF functions access sample data as if it were an nxT matrix storing `T'
 * samples of `n' signals.
 *
 * In row major format, contiguous elements are in the same row. In column
 * major format, contiguous elements are in the same column.
 */
typedef enum edf_major_t {
  EDF_ROW_MAJOR,
  EDF_COL_MAJOR
} edf_major_t;

/**
 * Enum used to tell the edf functions what type of sample values to use.
 */
typedef enum edf_numtype_t {
  EDF_INT,
  EDF_FLOAT,
  EDF_DOUBLE
} edf_numtype_t;

typedef struct edf_file_t {
  edf_header_t head;

  int num_signals;
  int num_samples;

  edf_major_t major;

  short  *i_samples;
  double *d_samples;
  float  *f_samples;
} edf_file_t;

/**
 * Name: edf_readFile
 *
 * Description:
 * Reads in the information stored in the EDF file specified by the given
 * filename. This function reads the entire file into memory. That memory needs
 * to be released using the edf_freeFile() function.
 *
 * This function intializes the `i_samples' field of the EDF file, but does
 * not compute the `d_samples' or `f_samples' fields.
 *
 * Parameters:
 * @param filename      the file to read
 * @param major         in what order to store the read-in sample data
 *
 * Returns:
 * @return edf_file_t*  the read file, NULL if there was a problem
 */
edf_file_t *edf_readFile( const char *filename, edf_major_t major );

/**
 * Name: edf_saveToFile
 *
 * Opens the file specified by the given filename, writes the given file data
 * to the file and closes the file.
 *
 * Parameters:
 * @param filename      the file to write to
 * @param file          the file data to write to the file
 *
 * Returns:
 * @return int          0 if any errors are detected, nonzero othewise
 */
int edf_saveToFile( const char *filename, const edf_file_t *file );

/**
 * Name: edf_freeFile
 *
 * Description:
 * Frees memory allocated by the call to edf_readFile that created the given
 * file.
 *
 * Once this function has been called, the given file should no longer be used.
 *
 * Parameters:
 * @param file          the file whose memory we want to free
 */
void edf_freeFile( edf_file_t *file );

/**
 * Name: edf_convert
 *
 * Description:
 * Converts the samples of the EDF file from type `from_t' into type `to_t'.
 * Depending on type fields' values, either the `f_samples' or `d_samples'
 * field of the edf_file_t struct will be intialized, or must already be
 * initialized.
 *
 * If `to_t' specifies a type field that has already been initialized, the field
 * will be free'd and recomputed. For example, if the EDF_FLOAT type is
 * specified for `to_t' and the `f_samples' field already exists, the
 * `f_samples' field will be free'd and recomputed.
 *
 * If the `from_t' type does not yet exist (e.g., if EDF_FLOAT is specified
 * but the `f_samples' field has not been set), then this function will do
 * nothing.
 *
 * If the `to_t' specifies EDF_INT, the phys_max and phys_min fields will be
 * recalculated.
 *
 * Parameters:
 * @param edf_file      the file to modify
 * @param from_t        the source type
 * @param to_t          the destination type
 */
void edf_convert( edf_file_t *edf_file, edf_numtype_t from_t,
                  edf_numtype_t to_t );

/**
 * Name: edf_diffChannels
 *
 * Description:
 * Searches the given EDF file for the specified channels and computes their
 * difference given by subtracting 'right' from 'left'.
 *
 * For example, to compute the `FP1 - F3' channel, this function could be
 * called like:
 *
 *    edf_diffChannels( fp1_f3, "FP1", "F3", edf_file, type );
 *
 * Channel labels are compared only upto the number of characters in the
 * parameter string. This is to gaurd against the EDF file containing the label,
 * e.g., "FP1     ". This does mean, though, if one of the labels is, for
 * example, "FP", then this will match "FP1" and "FP2".
 *
 * If the 'type' parameter specifies a type that has not yet been calculated,
 * e.g., if type is EDF_DOUBLE and the double sample values have not yet been
 * computed, then the appropriate edf_convert... function is first called to
 * generate those values.
 *
 * PRE:
 * The 'diff' parameter is expected to point to valid memory with enough space.
 * The memory must contain space for `num_samples' elements.
 *
 * Parameters:
 * @param diff          where to store the resulting difference
 * @param left          the left channel to search for
 * @param right         the right channel to search for
 * @param file          the file to read from
 * @param type          whether to diff the int, float, or double channels
 *
 * Returns:
 * @return int          0 if the channel could not be found, nonzero otherwise
 */
int edf_diffChannels( void *diff, const char *left, const char *right,
                      edf_file_t *file, edf_numtype_t type );

#ifdef __cplusplus
}
#endif

#endif
