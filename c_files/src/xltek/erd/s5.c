#include "xltek/erd/s5.h"
#include "xltek/erd/headbox_types.h"
#include "xltek/erd/channel_labels.h"

#include "xltek/xltek.h"
#include "xltek/x_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_printErdHeaderS5( const xltek_erd_s5_t *head )
{
  unsigned int i, boxtype[4] = { head->m_headbox_type[0], 
                                 head->m_headbox_type[1], 
                                 head->m_headbox_type[2], 
                                 head->m_headbox_type[3] };
  for (i = 0; i < 4; i++) {
    if (boxtype[i] >= _num_headbox_types) {
      boxtype[i] = 0;
    }
  }

  printf("ERD schema 5.\n"
         "Sample frequency:           %g\n"
         "Number of channels:         %d\n"
         "Bits of delta per channel:  %d\n"
         "Storage order:              --skipped--\n"
         "\n"
         "Headbox:  type        |  serial  | software version\n"
         "          ------------+----------+-----------------\n"
         "          %d %s | %08X | %s\n"
         "          %d %s | %08X | %s\n"
         "          %d %s | %08X | %s\n"
         "          %d %s | %08X | %s\n"
         "\n"
         "DSP hardware version:       %s\n"
         "DSP software version:       %s\n"
         "Discarded least sig. bits:  %d\n",
         head->m_sample_freq,
         head->m_num_channels,
         head->m_deltabits,
         head->m_headbox_type[0], _headbox_types[boxtype[0]],
            head->m_headbox_sn[0], head->m_headbox_sw_version[0],
         head->m_headbox_type[1], _headbox_types[boxtype[1]],
            head->m_headbox_sn[1], head->m_headbox_sw_version[1],
         head->m_headbox_type[2], _headbox_types[boxtype[2]],
            head->m_headbox_sn[2], head->m_headbox_sw_version[2],
         head->m_headbox_type[3], _headbox_types[boxtype[3]],
            head->m_headbox_sn[3], head->m_headbox_sw_version[3],
         head->m_dsp_hw_version,
         head->m_dsp_sw_version,
         head->m_discardbits );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
xltek_erd_file_t *xltek_openErdFileS5( const char *filename )
{
  xltek_erd_file_t *erd_file;
  FILE *file = fopen( filename, "r" );

  if (file == NULL) {
    fprintf( stderr, "Failed to open file '%s'!\n", filename );
    return NULL;
  }

  // Allocate space for the details about the file.
  erd_file = (xltek_erd_file_t*) malloc( sizeof(xltek_erd_file_t) );

  // Read in the generic header.
  fread(&(erd_file->gen_header), sizeof(xltek_head_t), 1, file);

  // Verify that this is a schema 5 ERD file.
  if (!xltek_uuidEqual( erd_file->gen_header.m_file_guid, UUID_ERD ) ||
      erd_file->gen_header.m_file_schema != 5) {
    fprintf( stderr, "Given file '%s' is not a schema 5 Raw Data file!\n",
             filename );
    free( erd_file );
    return NULL;
  }

  // Now that we know we've got the correct file, save its details, read in the
  // ERD header, and return so that samples can begin to be read in.
  erd_file->filename = strdup( filename );
  erd_file->file = file;
  erd_file->schema = erd_file->gen_header.m_file_schema;

  fread(&(erd_file->erd_head), sizeof(xltek_erd_s5_t), 1, file);

  return erd_file;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char **xltek_getErdLabelsS5( xltek_erd_file_t *file )
{
  int i;
  char const **labels;

  if (file->schema != 5) {
    fprintf( stderr, "Given file '%s' is not a schema 5 Raw Data file!\n",
             file->filename );
    return NULL;
  }

  // Allocate space for the labels.
  labels = (char const**) malloc(sizeof(char)*file->erd_head.s5.m_num_channels);

  // Define a couple macros to make value verification easier on my words-per-
  // minute and to make the code way more concise.
  #define VALID_INDEX_IN( labels_array ) \
          ((file->erd_head.s5.m_phys_chan[i] < \
            sizeof((labels_array)) / sizeof(char*)) &&\
          (file->erd_head.s5.m_phys_chan[i] >= 0))

  #define CASE_STATEMENT(c,la) \
    case (c):\
      if (VALID_INDEX_IN((la))) {\
        labels[i] = (la)[file->erd_head.s5.m_phys_chan[i]];\
      } else {\
        fprintf( stderr, "Invalid physical channel index (%d) in '%s'!\n", \
                 file->erd_head.s5.m_phys_chan[i], file->filename ); \
        free(labels); \
        return NULL; \
      }\
      break

  for (i = 0; i < file->erd_head.s5.m_num_channels; i++) {
    switch (file->erd_head.s5.m_headbox_type[0]) {
      CASE_STATEMENT( 1, labels_eeg32 );
      CASE_STATEMENT( 3, labels_eeg128 );
      CASE_STATEMENT( 4, labels_amb28 );
      CASE_STATEMENT( 5, labels_hyppo );
      CASE_STATEMENT( 6, labels_emu36 );
      CASE_STATEMENT( 8, labels_mobee24 );
      CASE_STATEMENT( 9, labels_mobee32 );
      default:
        fprintf( stderr, "Given file\n"
                         "  %s\n"
                         "used an unknown headbox type (%d)!\n",
                 file->filename, file->erd_head.s5.m_headbox_type[0] );
        free(labels);
        return NULL;
    }
  }

  return labels;
}
