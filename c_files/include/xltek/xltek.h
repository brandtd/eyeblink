#ifndef XLTEK_H
#define XLTEK_H

#include "xltek/erd.h"
#include "xltek/x_types.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// General functions.
////////////////////////////////////////////////////////////////////////////////

/**
 * Name: xltek_printHeader
 *
 * Description:
 * Prints the general file header in a human readable format.
 *
 * Parameters:
 * @param head      the header to print
 */
void xltek_printHeader( const xltek_head_t *head );

////////////////////////////////////////////////////////////////////////////////
// UUID specific functions.
////////////////////////////////////////////////////////////////////////////////

/**
 * Name: xltek_uuidToString
 *
 * Description:
 * Converts the given UUID into a character string. The given string must have
 * space for at least 37 characters (36 printable characters and the NULL
 * character).
 *
 * Parameters:
 * @param uuid      the UUID to format
 * @param str       where to put the formatted string
 */
void xltek_uuidToString( const uuid_t uuid, char *str );

/**
 * Name: xltek_uuidEqual
 *
 * Description:
 * Compares the given two UUIDs, returning nonzero if the UUIDs are equal and
 * zero otherwise.
 *
 * Parameters:
 * @param u1        UUID to compare
 * @param u2        UUID to compare
 *
 * Returns:
 * @return int      nonzero if UUIDs are equal, zero otherwise
 */
int xltek_uuidEqual( const uuid_t u1, const uuid_t u2 );

#ifdef __cplusplus
}
#endif

#endif
