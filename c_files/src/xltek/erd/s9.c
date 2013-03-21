#include "xltek/erd/s9.h"
#include "xltek/erd/headbox_types.h"
#include "xltek/erd/channel_labels.h"

#include "xltek/xltek.h"
#include "xltek/x_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_printErdHeaderS9( const xltek_erd_s9_t *head )
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

  printf("ERD schema 9.\n"
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
xltek_erd_file_t *xltek_openErdFileS9( const char *filename )
{
  int i;
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

  // Verify that this is a schema 9 ERD file.
  if (!xltek_uuidEqual( erd_file->gen_header.m_file_guid, UUID_ERD ) ||
      erd_file->gen_header.m_file_schema != 9) {
    fprintf( stderr, "Given file '%s' is not a schema 9 Raw Data file!\n",
             filename );
    free( erd_file );
    return NULL;
  }

  // Now that we know we've got the correct file, save its details, read in the
  // ERD header, and return so that samples can begin to be read in.
  erd_file->filename = strdup( filename );
  erd_file->file = file;
  erd_file->schema = erd_file->gen_header.m_file_schema;

  fread(&(erd_file->erd_head), sizeof(xltek_erd_s9_t), 1, file);

  // Check to see if any of the input channels is being sampled at a different
  // frequency than the headbox frequency. At the same time, count the number of
  // shorted channels.
  erd_file->freq_factor = 0;
  erd_file->num_shorted = 0;
  for (i = 0; i < erd_file->erd_head.s9.m_num_channels; i++) {
    if (erd_file->erd_head.s9.m_frequency_factor[i] != 0x7FFF) {
      erd_file->freq_factor = 1;
    }

    if (erd_file->erd_head.s9.m_shorted[i] != 0) {
      erd_file->num_shorted++;
    }
  }

  // Allocate space for previous sample values.
  erd_file->prev_samples = (int*) malloc( sizeof(int) *
                                        erd_file->erd_head.s9.m_num_channels );

  return erd_file;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char **xltek_getErdLabelsS9( xltek_erd_file_t *file )
{
  int i;
  char const **labels;

  if (file->schema != 9) {
    fprintf( stderr, "Given file '%s' is not a schema 9 Raw Data file!\n",
             file->filename );
    return NULL;
  }

  // Allocate space for the labels.
  labels = (char const**)malloc(sizeof(char*)*file->erd_head.s9.m_num_channels);

  // Define a couple macros to make value verification easier on my words-per-
  // minute and to make the code way more concise.
  #define VALID_INDEX_IN( labels_array ) \
          ((file->erd_head.s9.m_phys_chan[i] < \
            sizeof((labels_array)) / sizeof(char*)) &&\
          (file->erd_head.s9.m_phys_chan[i] >= 0))

  #define CASE_STATEMENT(c,la) \
    case (c):\
      if (VALID_INDEX_IN((la))) {\
        labels[i] = (la)[file->erd_head.s9.m_phys_chan[i]];\
      } else {\
        fprintf( stderr, "Invalid physical channel index (%d) in '%s'!\n", \
                 file->erd_head.s9.m_phys_chan[i], file->filename ); \
        free(labels); \
        return NULL; \
      }\
      break

  for (i = 0; i < file->erd_head.s9.m_num_channels; i++) {
    switch (file->erd_head.s9.m_headbox_type[0]) {
      CASE_STATEMENT( 1, labels_eeg32 );
      CASE_STATEMENT( 3, labels_eeg128 );
      CASE_STATEMENT( 4, labels_amb28 );
      CASE_STATEMENT( 5, labels_hyppo );
      CASE_STATEMENT( 6, labels_emu36 );
      CASE_STATEMENT( 8, labels_mobee24 );
      CASE_STATEMENT( 9, labels_mobee32 );
      CASE_STATEMENT( 14, labels_connex );
      CASE_STATEMENT( 15, labels_trex );
      CASE_STATEMENT( 17, labels_emu40 );
      CASE_STATEMENT( 19, labels_eeg32u );
      CASE_STATEMENT( 21, labels_neurolink );
      CASE_STATEMENT( 22, labels_netlink );
      CASE_STATEMENT( 23, labels_traveler );
      default:
        fprintf( stderr, "Given file\n"
                         "  %s\n"
                         "used an unknown headbox type (%d)!\n",
                 file->filename, file->erd_head.s9.m_headbox_type[0] );
        free(labels);
        return NULL;
    }
  }

  return labels;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int xltek_getNextSamplesS9( xltek_erd_file_t *file, double *samples )
{
  unsigned int i, mask_bytes, num_channels;
  unsigned char event_byte, freq_byte, *delta_mask;

  char  thin_delta;
  short wide_delta;
  int  *abs_deltas;

  if (file->schema != 9) {
    fprintf( stderr, "Given file '%s' is not a schema 9 Raw Data file!\n",
             file->filename );
    return 0;
  }

  // Make typing easier by copying the number of channels to a local variable.
  num_channels = file->erd_head.s9.m_num_channels;

  // Read in the event byte.
  if (fread( &event_byte, sizeof(char), 1, file->file ) != 1) {
    // We've reached the end of the file.
    for (i = 0; i < num_channels; i++) {
      samples[i] = 0.0;
    }

    return 0;
  }

  // If necessary, read in the frequency byte. This is currently ignored.
  if (file->freq_factor) {
    fread( &freq_byte, sizeof(char), 1, file->file );
  }

  // TODO: stop ignoring the frequency byte.
  freq_byte = FREQ_ALL_CHANNELS;

  // Read in the delta mask.
  mask_bytes = (num_channels + 7) / 8;
  delta_mask = (unsigned char*) malloc( sizeof(char) * mask_bytes );

  for (i = 0; i < mask_bytes; i++) {
    fread(delta_mask + i, sizeof(char), 1, file->file);
  }

  // Read in and apply deltas.
  abs_deltas = (int*) calloc( num_channels, sizeof(int) );
  for (i = 0; i < num_channels; i++) {
    if (delta_mask[i / 8] & (0x01 << (i % 8))) {
      // Wide delta (delta is 16 bits).
      fread( &wide_delta, sizeof(wide_delta), 1, file->file );

      if (wide_delta != (short) 0xFFFF) {
        // Add the delta value to the previous channel value.
        file->prev_samples[i] += (int) wide_delta;
      } else {
        abs_deltas[i] = 1;
      }
    } else {
      // Thin delta (delta is  8 bits).
      fread( &thin_delta, sizeof(thin_delta), 1, file->file );
      file->prev_samples[i] += (int) thin_delta;
    }
  }

  // Find all the 'absolute deltas'. For these, we must read in the full value
  // of the channel.
  for (i = 0; i < num_channels; i++) {
    if (abs_deltas[i]) {
      fread( file->prev_samples + i, sizeof(int), 1, file->file );
    }
  }

  // With all the sample values updated, convert those samples into doubles.
  for (i = 0; i < num_channels; i++) {
    samples[i] = xltek_convertSample(file->prev_samples[i],
                                     file->erd_head.s9.m_headbox_type[0],
                                     file->erd_head.s9.m_phys_chan[i],
                                     file->erd_head.s9.m_discardbits,
                                     file->erd_head.s9.m_headbox_sw_version[0]);
  }

  free( delta_mask );
  free( abs_deltas );

  return 1;
}
