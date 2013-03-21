#ifndef XLTEK_ERD_H
#define XLTEK_ERD_H

#include "xltek/x_types.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// ERD file specific functions.
////////////////////////////////////////////////////////////////////////////////

/**
 * Name: xltek_printErdHeader
 *
 * Description:
 * Prints the ERD file header in a human readable format.
 *
 * Parameters:
 * @param head      the header to print
 * @param schema    the file schema as given by the general file header
 */
void xltek_printErdHeader( const xltek_erd_head_t *head, int schema );

/**
 * Name: xltek_openErdFile
 *
 * Description:
 * Opens the file whose name is given by the `filename' parameter. If the
 * file cannot be opened, or turns out to be invalid (e.g., is not an ERD file),
 * NULL is returned.
 *
 * Files opened in this way must be closed using the xltek_closeErdFile()
 * function.
 *
 * Parameters:
 * @param filename              name of the file to open
 *
 * Returns:
 * @return xltek_erd_file_t     an opaque type, similar to FILE* that should
 *                              be passed to the other ERD file functions
 */
xltek_erd_file_t *xltek_openErdFile( const char *filename );

/**
 * Name: xltek_closeErdFile
 *
 * Description:
 * Closes the given file. Calling this function ensures that memory allocated
 * for the given file is freed and that the underlying file descriptor is
 * closed.
 *
 * After this function is called, the given file is closed and should not be
 * used again.
 *
 * Parameters:
 * @param file      the file to close
 */
void xltek_closeErdFile( xltek_erd_file_t *file );

/**
 * Name: xltek_getErdLabels
 *
 * Description:
 * Returns the labels associated with sample channels for the given ERD file.
 * The label strings themselves are statically allocated strings and should not
 * be freed, but this function does alloc(3) memory for the returned array, and
 * that array must be free(3)'d.
 *
 * The returned array is arranged such that array[i] points to the channel label
 * for sample[i].
 *
 * Parameters:
 * @param file      the file for which we want channel labels
 *
 * Returns:
 * @return const char**   a freshly alloc(3)'d array pointing to channel labels
 */
char const **xltek_getErdLabels( xltek_erd_file_t *file );

/**
 * Name: xltek_getErdNumChannels
 *
 * Description:
 * Returns how many channels are recorded within the given file.
 *
 * Parameters:
 * @param file      the file to look at
 *
 * Returns:
 * @return int      how many channels are encoded within the file
 */
int xltek_getErdNumChannels( xltek_erd_file_t *file );

/**
 * Name: xltek_getErdSampleFreq
 *
 * Description:
 * Returns the sample frequency of an ERD file.
 *
 * Parameters:
 * @param file      the file to look at
 *
 * Returns:
 * @return double   the sample frequency
 */
double xltek_getErdSampleFreq( xltek_erd_file_t *file );

/**
 * Name: xltek_getNextSamples
 *
 * Description:
 * Decodes the next set of samples from the given ERD file. The samples are
 * returned within the given `samples' array. The array must have enough space
 * for each channel.
 *
 * The required amount of space can be found by querying the number of channels
 * encoded within the ERD file using the xltek_getErdNumChannels() function.
 *
 * Parameters:
 * @param file      the file to decode
 * @param samples   where to store the extracted samples
 *
 * Returns:
 * @return int      0 if no samples existed to decode (EOF was reached) or there
 *                  was a problem, nonzero otherwise
 */
int xltek_getNextSamples( xltek_erd_file_t *file, double *samples );

/**
 * Name: xltek_convertSample
 *
 * Description:
 * This function should not be called by users of this library.
 *
 * This function decodes a sample value from its signed int representation to
 * its double representation according to the conversion function defined by the
 * channel, headbox type, headbox software version, and the number of 'discard
 * bits' all of which are specified in the ERD file header.
 *
 * Parameters:
 * @param sample          the sample to convert
 * @param headbox_type    the headbox type given in the ERD header
 * @param channel         which channel the sample comes from
 * @param discard_bits    the m_discardbits field from the ERD header
 * @param sw_version      the headbox software version string given in the ERD
 *                        header
 */
double xltek_convertSample( int sample, int headbox_type, int channel,
                            int discard_bits, const char *sw_version );

#ifdef __cplusplus
}
#endif

#endif
