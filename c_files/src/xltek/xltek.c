#include "xltek/xltek.h"
#include "xltek/guids.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_printHeader( const xltek_head_t *head )
{
  const char *file_type;
  char uuid_str[37];
  time_t create_time = (time_t) head->m_creation_time;

  //////////////////////////////////////////////////////////////////////////////
  // Figure out which type of file this is.
  //////////////////////////////////////////////////////////////////////////////
  if        (xltek_uuidEqual( head->m_file_guid, UUID_EEG )) {
    file_type = "Patient Information (.eeg)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_ERD )) {
    file_type = "Raw Data (.erd)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_ETC )) {
    file_type = "Table of Contents (.etc)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_ENT )) {
    file_type = "Notes (.ent)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_SNC )) {
    file_type = "Synchronization File (.snc)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_STC )) {
    file_type = "Segments Table of Contents (.stc)";
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_VTC_MPEG1 )) {
    // The VTC files follow a different header format.
    file_type = "Video Table of Contents (MPEG-1) (.vtc)";
    printf( "File type: %s\n", file_type );
    printf( "This file type does not follow the standard used by other xltek\n"
            "files, so the rest of the header that's been read in won't make\n"
            "sense.\n" );
    return;
  } else if (xltek_uuidEqual( head->m_file_guid, UUID_VTC_MPEG4 )) {
    // The VTC files follow a different header format.
    file_type = "Video Table of Contents (MPEG-4) (.vtc)";
    printf( "File type: %s\n", file_type );
    printf( "This file type does not follow the standard used by other xltek\n"
            "files, so the rest of the header that's been read in won't make\n"
            "sense.\n" );
    return;
  } else {
    // If this is an unknown file type, print a warning message and return since
    // we don't know if the rest of the data will mean anything.
    printf( "Unknown file type.\n" );

    xltek_uuidToString( head->m_file_guid, uuid_str );

    printf( "GUID: %s\n", uuid_str );
    printf( "Note: byte order is network byte order (big-endian)\n" );
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Print header information.
  //////////////////////////////////////////////////////////////////////////////
  printf("File type:      %s\n", file_type);
  printf("file schema:    %d\n", head->m_file_schema);
  printf("base schema:    %d\n", head->m_base_schema);
  printf("creation time:  %s",   ctime(&(create_time)));
  printf("patient ID:     %d\n", head->m_patient_id);
  printf("study ID:       %d\n", head->m_study_id);
  printf("last name:      %s\n", head->m_pat_last_name);
  printf("first name:     %s\n", head->m_pat_first_name);
  printf("middle name:    %s\n", head->m_pat_middle_name);
  printf("id:             %s\n", head->m_pat_id);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_uuidToString( const uuid_t uuid, char *str )
{
  sprintf(str,
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      uuid[3], uuid[2], uuid[1], uuid[0],
      uuid[5], uuid[4],
      uuid[7], uuid[6],
      uuid[8], uuid[9],
      uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15] );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int xltek_uuidEqual( const uuid_t u1, const uuid_t u2 )
{
  int i;
  for (i = 0; i < 16; i++) {
    if (u1[i] != u2[i]) {
      return 0;
    }
  }
  return 1;
}
