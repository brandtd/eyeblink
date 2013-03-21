#include "xltek/edf.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
edf_file_t *edf_readFile( const char *filename, edf_major_t major )
{
  edf_file_t *file;
  char *head_bottom;
  int i, j, num_signals, bottom_size;
  FILE *file_p;

  file = (edf_file_t*) malloc( sizeof(edf_file_t) );
  file_p = fopen( filename, "r" );
  if (file_p == NULL) {
    fprintf( stderr, "Failed to open EDF file '%s'!\n", filename );
    free(file);
    return NULL;
  }

  file->d_samples = NULL; file->f_samples = NULL;

  // Read in the constant length part of the file header.
  fread( &(file->head.top), sizeof(edf_header_top_t), 1, file_p );

  // Figure out how many signals there are so that we can read in the rest of
  // the header.
  num_signals = file->num_signals = atoi( file->head.top.num_signals );

  // Allocate a block of memory to hold the bottom of the header.
  bottom_size = 16 + // labels
                80 + // transducers
                8  + // physical dimensions
                8  + // physical minimums
                8  + // physical maximums
                8  + // digital minimums
                8  + // digital maximums
                80 + // prefilter strings
                8  + // number of samples per record
                32;  // reserved space
  head_bottom = (char*) malloc( sizeof(char) * num_signals * bottom_size );
  fread( head_bottom, sizeof(char), num_signals * bottom_size, file_p );

  // Allocate memory for the variable sized fields.
  file->head.labels             = (char**) malloc(sizeof(char*) * num_signals);
  file->head.transducers        = (char**) malloc(sizeof(char*) * num_signals);
  file->head.phys_dim           = (char**) malloc(sizeof(char*) * num_signals);
  file->head.phys_min           = (char**) malloc(sizeof(char*) * num_signals);
  file->head.phys_max           = (char**) malloc(sizeof(char*) * num_signals);
  file->head.dig_min            = (char**) malloc(sizeof(char*) * num_signals);
  file->head.dig_max            = (char**) malloc(sizeof(char*) * num_signals);
  file->head.prefilter          = (char**) malloc(sizeof(char*) * num_signals);
  file->head.samples_per_record = (char**) malloc(sizeof(char*) * num_signals);
  file->head.reserved_1         = (char**) malloc(sizeof(char*) * num_signals);

  // Fill in the variable sized fields.
  for (i = 0; i < num_signals; i++) {
    file->head.labels[i]             = head_bottom + i * 16;
    file->head.transducers[i]        = head_bottom + i * 80 + num_signals * 16;
    file->head.phys_dim[i]           = head_bottom + i *  8 + num_signals * 96;
    file->head.phys_min[i]           = head_bottom + i *  8 + num_signals * 104;
    file->head.phys_max[i]           = head_bottom + i *  8 + num_signals * 112;
    file->head.dig_min[i]            = head_bottom + i *  8 + num_signals * 120;
    file->head.dig_max[i]            = head_bottom + i *  8 + num_signals * 128;
    file->head.prefilter[i]          = head_bottom + i * 80 + num_signals * 136;
    file->head.samples_per_record[i] = head_bottom + i *  8 + num_signals * 216;
    file->head.reserved_1[i]         = head_bottom + i * 32 + num_signals * 224;
  }

  file->num_samples = atoi( file->head.samples_per_record[0] );

  // Allocate space for and read in the sample data.
  file->i_samples = (short*) malloc( sizeof(short)*
                                     num_signals * file->num_samples );
  file->major = major;

  if (major == EDF_ROW_MAJOR) {
    for (i = 0; i < num_signals; i++) {
      fread( file->i_samples + i * file->num_samples, sizeof(short),
             file->num_samples, file_p );
    }
  } else {
    for (i = 0; i < num_signals; i++) {
      for (j = 0; j < file->num_samples; j++) {
        fread( file->i_samples + (j * file->num_signals + i),
               sizeof(short), 1, file_p );
      }
    }
  }

  // Everything's been read in, so we can close the opened file and return.
  fclose(file_p);

  return file;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int edf_saveToFile( const char *filename, const edf_file_t *file )
{
  int i, j, num_signals, num_samples;

  FILE *edf_file = fopen( filename, "w" );
  if (edf_file == NULL) {
    fprintf( stderr, "Failed to open EDF file '%s' for writing!\n", filename );
    return 0;
  }

  // Write the constant length part of the file header to the file.
  fwrite( &(file->head.top), sizeof(edf_header_top_t), 1, edf_file );

  // Figure out how many signals there are and then write the variable length
  // part of the header to file.
  num_signals = file->num_signals;

  #define WRITE( size, parameter ) \
    for (i = 0; i < num_signals; i++) {\
      fwrite( file->head.parameter[i],\
      sizeof(char), size, edf_file );\
    }

  WRITE( 16, labels )
  WRITE( 80, transducers )
  WRITE(  8, phys_dim )
  WRITE(  8, phys_min )
  WRITE(  8, phys_max )
  WRITE(  8, dig_min )
  WRITE(  8, dig_max )
  WRITE( 80, prefilter )
  WRITE(  8, samples_per_record )
  WRITE( 32, reserved_1 )

  // Now write the sample data itself. We assume that each signal has the same
  // number of samples.
  num_samples = file->num_samples;

  // If the data was given in row major format, our job is really easy. If it
  // was given in column major format, then we've got to do a bit of looping.
  if (file->major == EDF_ROW_MAJOR) {
    fwrite(file->i_samples, sizeof(short), num_samples * num_signals, edf_file);
  } else {
    for (j = 0; j < num_signals; j++) {
      for (i = 0; i < num_samples; i++) {
        fwrite( file->i_samples + (i * num_signals + j), sizeof(short), 1,
                edf_file );
      }
    }
  }

  // And we're done!
  fclose(edf_file);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void edf_freeFile( edf_file_t *file )
{
  // Free the memory block that's holding all the variable sized header info.
  free( file->head.labels[0] );

  // Free the variable sized fields.
  free( file->head.labels );
  free( file->head.transducers );
  free( file->head.phys_dim );
  free( file->head.phys_min );
  free( file->head.phys_max );
  free( file->head.dig_min );
  free( file->head.dig_max );
  free( file->head.prefilter );
  free( file->head.samples_per_record );
  free( file->head.reserved_1 );

  // Free the sample arrays. We're counting on the free function to handle NULL
  // cleanly.
  free( file->i_samples );
  free( file->d_samples );
  free( file->f_samples );

  // Free the file struct itself.
  free( file );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void edf_convert( edf_file_t *edf_file, edf_numtype_t from_t,
                  edf_numtype_t to_t )
{
  int i, j, k, dig_min, dig_max;
  double phys_min, phys_max, slope, intercept, val;
  char str[9];

  //////////////////////////////////////////////////////////////////////////////
  // Examine conversion type for correctness and initialization requirements.
  //////////////////////////////////////////////////////////////////////////////

  // Verify that we've been given valid type specifiers.
  if (from_t != EDF_INT && from_t != EDF_FLOAT && from_t != EDF_DOUBLE) {
    return;
  }
  if (to_t != EDF_INT && to_t != EDF_FLOAT && to_t != EDF_DOUBLE) {
    return;
  }

  // If we're supposed to convert from a type that does not yet exist, do
  // nothing.
  if (from_t == EDF_INT && edf_file->i_samples == NULL) {
    return;
  } else if (from_t == EDF_FLOAT && edf_file->f_samples == NULL) {
    return;
  } else if (from_t == EDF_DOUBLE && edf_file->d_samples == NULL) {
    return;
  }

  // If we're supposed to convert from a type to the same type, eureka! We're
  // already done!
  if (from_t == to_t) {
    return;
  }

  // If the samples have already been generated, assume the user wants them
  // regenerated.
  if (to_t == EDF_DOUBLE) {
    if (edf_file->d_samples) { free(edf_file->d_samples); }
    edf_file->d_samples = (double*) malloc( sizeof(double) *
                                            edf_file->num_signals *
                                            edf_file->num_samples );
  } else if (to_t == EDF_FLOAT) {
    if (edf_file->f_samples) { free(edf_file->f_samples); }
    edf_file->f_samples = (float*) malloc( sizeof(float) *
                                            edf_file->num_signals *
                                            edf_file->num_samples );
  } else { // Converting to integers.
    if (edf_file->i_samples) { free(edf_file->i_samples); }
    edf_file->i_samples = (short*) malloc( sizeof(short) *
                                            edf_file->num_signals *
                                            edf_file->num_samples );
  }

  // Make sure that any strings we copy over from the EDF header are NULL
  // terminated.
  str[8] = '\0';

  //////////////////////////////////////////////////////////////////////////////
  // Begin converting.
  //////////////////////////////////////////////////////////////////////////////

  // If we're converting from the physical domain, we need to recalculate the
  // physical maximum/minimum values.
  if (from_t != EDF_INT) {
    for (i = 0; i < edf_file->num_signals; i++) {
      phys_min = INFINITY; phys_max = -INFINITY;

      for (j = 0; j < edf_file->num_samples; j++) {
        if (edf_file->major == EDF_ROW_MAJOR) {
          k = i * edf_file->num_samples + j;
        } else { // Column major.
          k = j * edf_file->num_signals + i;
        }

        if (from_t == EDF_FLOAT) {
          val = (double) edf_file->f_samples[k];
        } else { // EDF_DOUBLE
          val = (double) edf_file->d_samples[k];
        }

        if (val < phys_min) { phys_min = val; }
        if (val > phys_max) { phys_max = val; }
      }

      // Record the new phys_min/phys_max values.
      snprintf( str, 9, "%f        ", phys_min );
      strncpy( edf_file->head.phys_min[i], str, 8 );

      snprintf( str, 9, "%f        ", phys_max );
      strncpy( edf_file->head.phys_max[i], str, 8 );
    }
  }

  // We assume a linear mapping between the digital and physical values. We also
  // calculate everything as a double, and then cast to float as the last step.
  for (i = 0; i < edf_file->num_signals; i++) {
    // Figure out the digital/physical mins/maxs of the signal.
    strncpy( str, edf_file->head.dig_min[i], 8 ); dig_min = atoi( str );
    strncpy( str, edf_file->head.dig_max[i], 8 ); dig_max = atoi( str );

    strncpy( str, edf_file->head.phys_min[i], 8 ); phys_min = atof( str );
    strncpy( str, edf_file->head.phys_max[i], 8 ); phys_max = atof( str );

    // When coverting between the two physical domains (float/double), the
    // mapping is direct. When converting to/from the digital domain, the
    // mapping is linear.
    switch (from_t) {
      case EDF_INT:
        slope     = (phys_max - phys_min) / (double) (dig_max - dig_min);
        intercept = phys_max - slope * (double) dig_max;
        break;

      case EDF_FLOAT:
        if (to_t == EDF_DOUBLE) {
          slope = 1.0; intercept = 0.0;
        } else {
          slope     =  (double) (dig_max - dig_min) / (phys_max - phys_min);
          intercept = dig_max - slope * (double) phys_max;
        }
        break;

      case EDF_DOUBLE:
      default:
        if (to_t == EDF_FLOAT) {
          slope = 1.0; intercept = 0.0;
        } else {
          slope     =  (double) (dig_max - dig_min) / (phys_max - phys_min);
          intercept = dig_max - slope * (double) phys_max;
        }
        break;
    }
    
    // Perform the conversion calculation, making sure to respect the storage
    // order of the sample matrices.
    for (j = 0; j < edf_file->num_samples; j++) {
      if (edf_file->major == EDF_ROW_MAJOR) {
        k = i * edf_file->num_samples + j;
      } else { // Column major.
        k = j * edf_file->num_signals + i;
      }

      switch (from_t) {
        case EDF_INT:
          val = slope * ((double) edf_file->i_samples[k]) + intercept; break;
        case EDF_FLOAT:
          val = slope * ((double) edf_file->f_samples[k]) + intercept; break;
        case EDF_DOUBLE:
        default:
          val = slope * ((double) edf_file->d_samples[k]) + intercept; break;
      }

      switch (to_t) {
        case EDF_INT:    edf_file->i_samples[k] = (short) val; break;
        case EDF_FLOAT:  edf_file->f_samples[k] = (float) val; break;
        case EDF_DOUBLE: edf_file->d_samples[k] = (double) val; break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int edf_diffChannels( void *diff, const char *left, const char *right,
                      edf_file_t *file, edf_numtype_t type )
{
  int i, l_i, r_i, chan_l = -1, chan_r = -1;
  size_t len_l, len_r;

  // Setup a couple convenience variables for accessing the 'diff' array.
  short  *i_diff = (short*) diff;
  float  *f_diff = (float*) diff;
  double *d_diff = (double*) diff;

  len_l = strlen(left);
  len_r = strlen(right);

  if (len_l > 8 || len_r > 8 || len_l == 0 || len_r == 0) {
    // EDF labels max out at 8 characters, so if the string lengths are invalid
    // we can return early.
    return 0;
  }

  // First, find the two channels' indices.
  for (i = 0; i < file->num_signals; i++) {
    if (strncmp( left, file->head.labels[i], len_l ) == 0) {
      chan_l = i;
    } else if (strncmp( right, file->head.labels[i], len_r ) == 0) {
      chan_r = i;
    }

    if (chan_l != -1 && chan_r != -1) {
      break;
    }
  }

  // If we didn't manage to find unique indices for both channels, we should
  // return an error here.
  if (chan_l == -1 || chan_r == -1) {
    if (chan_l == -1) {
      fprintf(stderr, "Could not find channel '%s' in EDF file!\n", left );
    } else {
      fprintf(stderr, "Could not find channel '%s' in EDF file!\n", right );
    }
    return 0;
  }

  // Make sure the appropriate sample types have been calculated.
  if (type == EDF_DOUBLE && file->d_samples == NULL) {
    edf_convert( file, EDF_INT, type );
  } else if (type == EDF_FLOAT && file->f_samples == NULL) {
    edf_convert( file, EDF_INT, type );
  }

  // Compute the difference between the two channels.
  for (i = 0; i < file->num_samples; i++) {
    if (file->major == EDF_ROW_MAJOR) {
      l_i = chan_l * file->num_samples + i;
      r_i = chan_r * file->num_samples + i;
    } else { // Column major.
      l_i = i * file->num_signals + chan_l;
      r_i = i * file->num_signals + chan_r;
    }

    switch (type) {
      case EDF_INT:    i_diff[i] = file->i_samples[l_i] - file->i_samples[r_i];
        break;
      case EDF_FLOAT:  f_diff[i] = file->f_samples[l_i] - file->f_samples[r_i];
        break;
      case EDF_DOUBLE: d_diff[i] = file->d_samples[l_i] - file->d_samples[r_i];
        break;
    }
  }

  // All done!
  return 1;
}
