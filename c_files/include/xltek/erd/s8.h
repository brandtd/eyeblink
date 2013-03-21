#ifndef XLTEK_ERD_S8
#define XLTEK_ERD_S8

#ifdef __cplusplus
extern "C" {
#endif

// Forward definition of erd file type.
struct xltek_erd_file_t;

/**
 * Struct defining the generic header for Xltek raw data, schema 8 files.
 */
typedef struct xltek_erd_s8_t {
  double         m_sample_freq;
  unsigned int   m_num_channels;
  unsigned int   m_deltabits;
  unsigned int   m_phys_chan[1024];
  unsigned int   m_headbox_type[4];
  unsigned int   m_headbox_sn[4];
  char           m_headbox_sw_version[4][10];
  char           m_dsp_hw_version[10];
  char           m_dsp_sw_version[10];
  unsigned int   m_discardbits;
  unsigned short m_shorted[1024];
  unsigned short m_frequency_factor[1024];
} xltek_erd_s8_t;

void xltek_printErdHeaderS8( const xltek_erd_s8_t *head );
struct xltek_erd_file_t *xltek_openErdFileS8( const char *filename );
const char **xltek_getErdLabelsS8( struct xltek_erd_file_t *file );

#ifdef __cplusplus
}
#endif

#endif
