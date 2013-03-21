#include "xltek/erd.h"
#include "xltek/erd/erd.h"

#include "xltek/guids.h"
#include "xltek/xltek.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_printErdHeader( const xltek_erd_head_t *head, int schema )
{
  switch (schema) {
    case 5: xltek_printErdHeaderS5( &(head->s5) ); break;
    case 6: xltek_printErdHeaderS6( &(head->s6) ); break;
    case 7: xltek_printErdHeaderS7( &(head->s7) ); break;
    case 8: xltek_printErdHeaderS8( &(head->s8) ); break;
    case 9: xltek_printErdHeaderS9( &(head->s9) ); break;
    default:
      printf( "Unknown ERD schema (%d).\n", schema );
      return;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
xltek_erd_file_t *xltek_openErdFile( const char *filename )
{
  xltek_head_t header;

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf( stderr, "Failed to open file '%s'!\n", filename );
    return NULL;
  }

  // Read in the generic Xltek header so we can verify the file is a Raw Data
  // file and then close the file, since we pass the filename off to another
  // function for the actual parsing.
  fread( &header, sizeof(xltek_head_t), 1, file );
  fclose(file);

  if (!xltek_uuidEqual( header.m_file_guid, UUID_ERD )) {
    fprintf( stderr, "Given file '%s' is not a Raw Data file!\n", filename );
    return NULL;
  }

  // From the generic header we need to find out which schema was used for this
  // file so that we can pass off the rest of the file parsing to the approriate
  // function.
  switch (header.m_file_schema) {
    case 5: return xltek_openErdFileS5( filename );
    case 6: return xltek_openErdFileS6( filename );
    case 7: return xltek_openErdFileS7( filename );
    case 8: return xltek_openErdFileS8( filename );
    case 9: return xltek_openErdFileS9( filename );
    default:
      fprintf( stderr, "ERD file uses unknown schema (%d)!\n",
               header.m_file_schema );
      return NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void xltek_closeErdFile( xltek_erd_file_t *file )
{
  if (file == NULL) {
    return;
  }

  // Free allocated memory and close the file.
  fclose( file->file );
  free( file->filename );
  if (file->prev_samples) {
    free(file->prev_samples);
  }

  // Set pointers to NULL and clear variables.
  file->filename = NULL;
  file->file = NULL;
  file->prev_samples = NULL;
  file->schema = 0;
  file->freq_factor = 0;
  file->num_shorted = 0;

  // Free the memory for this file.
  free(file);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
const char **xltek_getErdLabels( xltek_erd_file_t *file )
{
  if (file == NULL || file->schema < 5 || file->schema > 9) {
    fprintf( stderr, "Cannot fetch ERD labels from invalid ERD file!\n" );
    return NULL;
  }

  switch (file->schema) {
    case 5: return xltek_getErdLabelsS5( file );
    case 6: return xltek_getErdLabelsS6( file );
    case 7: return xltek_getErdLabelsS7( file );
    case 8: return xltek_getErdLabelsS8( file );
    case 9: return xltek_getErdLabelsS9( file );
  }

  // We should never get here.
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int xltek_getErdNumChannels( xltek_erd_file_t *file )
{
  if (file == NULL || file->schema < 5 || file->schema > 9) {
    fprintf( stderr, "Cannot fetch num channels from invalid ERD file!\n" );
    return 0;
  }

  switch (file->schema) {
    case 5: return file->erd_head.s5.m_num_channels;
    case 6: return file->erd_head.s6.m_num_channels;
    case 7: return file->erd_head.s7.m_num_channels;
    case 8: return file->erd_head.s8.m_num_channels;
    case 9: return file->erd_head.s9.m_num_channels;
  }

  // We should never get here.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
double xltek_getErdSampleFreq( xltek_erd_file_t *file )
{
  if (file == NULL || file->schema < 5 || file->schema > 9) {
    fprintf( stderr, "Cannot fetch sample frequency from invalid ERD file!\n" );
    return 0.0;
  }

  switch (file->schema) {
    case 5: return file->erd_head.s5.m_sample_freq;
    case 6: return file->erd_head.s6.m_sample_freq;
    case 7: return file->erd_head.s7.m_sample_freq;
    case 8: return file->erd_head.s8.m_sample_freq;
    case 9: return file->erd_head.s9.m_sample_freq;
  }

  // We should never get here.
  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int xltek_getNextSamples( xltek_erd_file_t *file, double *samples )
{
  if (file == NULL || file->schema < 9 || file->schema > 9) {
    fprintf( stderr, "Cannot fetch samples from invalid ERD file!\n" );
    return 0;
  }

  switch (file->schema) {
    case 9: return xltek_getNextSamplesS9( file, samples );
  }

  // We should never get here.
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
double xltek_convertSample( int sample, int headbox_type, int channel,
                            int discard_bits, const char *sw_version )
{
  static double const conv_8711  = 8711.0 / (2097152.0 - 0.5);
  static double const conv_5e7   = (5000000.0 / (1024.0 - 0.5)) / 64.0;
  static double const conv_2neg6 = 1.0 / 64.0;
  static double const conv_hyppo = (8711.0/(2097152.0-0.5))/(159.8/249.5);
  static double const conv_1e8   = (10000000.0 / (1024.0 - 0.5)) / 64.0;
  static double const conv_2e8   = (20000000.0 / 65536.0);
  static double const conv_108e6 = (10800000.0 / 65536.0) / 64.0;

  double as_doub;
  double two_power = pow(2.0, (double) discard_bits);

  switch (headbox_type) {
    // EEG32
    case 1:   as_doub = (double) sample * conv_8711 * two_power;
              break;

    // EEG128
    case 3:   as_doub = (double) sample * conv_8711 * two_power;
              break;

    // AMB28
    case 4:   if (channel <= 23) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else {
                as_doub = (double) sample * conv_5e7 * two_power;
              }
              break;

    // HYPPO
    case 5:   if (channel <= 25) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 31) {
                as_doub = (double) sample * conv_hyppo * two_power;
              } else if (channel <= 39 && atof(sw_version) < 3.4) {
                as_doub = (double) sample * conv_1e8 * two_power;
              } else if (channel <= 39 && atof(sw_version)) {
                as_doub = (double) sample * conv_2e8 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // EMU36
    case 6:   if (channel <= 31) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else {
                as_doub = (double) sample * conv_5e7 * two_power;
              }
              break;

    // MOBEE24
    case 8:   if (channel <= 24) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // MOBEE32
    case 9:   if (channel <= 32) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // Connex
    case 14:  if (channel <= 37) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 47) {
                as_doub = (double) sample * conv_108e6 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;
                
    // Trex
    case 15:  if (channel <= 27) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 31) {
                as_doub = (double) sample * conv_1e8 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // EMU40
    case 17:  if (channel <= 39) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 43) {
                as_doub = (double) sample * conv_108e6 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // EEG32U
    case 19:  as_doub = (double) sample * conv_8711 * two_power;
              break;

    // NeuroLink IP
    case 21:  if (channel <= 127) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else {
                as_doub = (double) sample * conv_2neg6 * two_power;
              }
              break;

    // Netlink
    case 22:  if (channel <= 31) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 39) {
                as_doub = (double) sample * conv_108e6 * two_power;
              } else if (channel <= 41) {
                as_doub = (double) sample * conv_2neg6 * two_power;
              } else {
                as_doub = (double) sample * conv_108e6 * two_power;
              }
              break;

    // Traveler
    case 23:  if (channel <= 31) {
                as_doub = (double) sample * conv_8711 * two_power;
              } else if (channel <= 35) {
                as_doub = (double) sample * conv_108e6 * two_power;
              } else if (channel <= 37) {
                as_doub = (double) sample * conv_2neg6 * two_power;
              } else {
                as_doub = (double) sample * conv_108e6 * two_power;
              }
              break;

    default:
      fprintf( stderr, "Unknown headbox type (%d)!\n", headbox_type );
      as_doub = 0.0;
  }

  return as_doub;
}
