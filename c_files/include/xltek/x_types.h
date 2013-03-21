#ifndef XLTEK_TYPES_H
#define XLTEK_TYPES_H

#include "xltek/guids.h"

#include <time.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Generic file header, common to .eeg, .erd, .ent, .etc, .snc, .stc
////////////////////////////////////////////////////////////////////////////////

/**
 * Struct defining the generic Xltek header. NOTE: base schema 1 is assumed.
 * In base schema 0, the m_base_schema and m_file_schema fields are combined.
 */
typedef struct xltek_head_t {
  uuid_t         m_file_guid;
  unsigned short m_file_schema;
  unsigned short m_base_schema;
  unsigned int   m_creation_time;
  unsigned int   m_patient_id;
  unsigned int   m_study_id;
  char           m_pat_last_name[80];
  char           m_pat_first_name[80];
  char           m_pat_middle_name[80];
  char           m_pat_id[80];
} xltek_head_t;

////////////////////////////////////////////////////////////////////////////////
// Raw data header, used in .erd
////////////////////////////////////////////////////////////////////////////////

#include "erd/erd.h"

/**
 * Union of the raw data headers.
 */
typedef union xltek_erd_head_t {
  xltek_erd_s5_t s5;
  xltek_erd_s6_t s6;
  xltek_erd_s7_t s7;
  xltek_erd_s8_t s8;
  xltek_erd_s9_t s9;
} xltek_erd_head_t;

////////////////////////////////////////////////////////////////////////////////
// Note file records, used in .ent
////////////////////////////////////////////////////////////////////////////////

typedef struct xltek_etc_record_t {
  unsigned int type;
  unsigned int length;
  unsigned int prev_length;
  unsigned int id;
} xltek_etc_record_t;

////////////////////////////////////////////////////////////////////////////////
// Table of contents entry, used in .etc
////////////////////////////////////////////////////////////////////////////////

/**
 * Struct defining a TOC entry, schema 2.
 */
typedef struct xltek_toc_entry_s2_t {
  unsigned int offset;
  unsigned int timestamp;
  unsigned int sample_num;
  unsigned int sample_span;
} xltek_toc_entry_s2_t;

/**
 * Struct defining a TOC entry, schema 3.
 */
typedef struct xltek_toc_entry_s3_t {
  unsigned int offset;
  unsigned int samplestamp;
  unsigned int sample_num;
  unsigned int sample_span;
} xltek_toc_entry_s3_t;

////////////////////////////////////////////////////////////////////////////////
// Synchronization file entry, used in .snc
////////////////////////////////////////////////////////////////////////////////

/**
 * Struct defining an SNC entry, schema 0.
 */
typedef struct xltek_snc_entry_s0_t {
  unsigned int sampleStamp;
  long unsigned int sampleTime;
} xltek_snc_entry_s0_t;

////////////////////////////////////////////////////////////////////////////////
// Segment table of contents header, used in .stc
////////////////////////////////////////////////////////////////////////////////

/**
 * Struct defining a generic STC file header, schema 1.
 */
typedef struct xltek_stc_head_s1_t {
  unsigned int m_next_segment;
  unsigned int m_final;
  unsigned int m_padding[12];
} xltek_stc_head_s1_t;

/**
 * Struct defining a generic STC file entry, schema 1.
 */
typedef struct xltek_stc_entry_s1_t {
  char segment_name[256];
  unsigned int start_stamp;
  unsigned int end_stamp;
  unsigned int sample_num;
  unsigned int sample_span;
} xltek_stc_entry_s1_t;

////////////////////////////////////////////////////////////////////////////////
// Implementation specific data structures. Users of the Xltek library functions
// should treat everything below this point as an opaque type, similar to the
// FILE* type.
////////////////////////////////////////////////////////////////////////////////

/**
 * Struct containing implementation details about a Raw Data (.erd) file.
 */
typedef struct xltek_erd_file_t {
  char *filename;
  FILE *file;

  unsigned int schema;

  xltek_head_t     gen_header;
  xltek_erd_head_t erd_head;

  unsigned int freq_factor;
  unsigned int num_shorted;

  int *prev_samples;
} xltek_erd_file_t;

#ifdef __cplusplus
}
#endif

#endif
