#ifndef XLTEK_ERD_S9
#define XLTEK_ERD_S9

#ifdef __cplusplus
extern "C" {
#endif

// Bit fields within the 'frequency byte' of a sample packet.
#define FREQ_ALL_CHANNELS  0x80
#define FREQ_1_50_CHANNELS 0x20
#define FREQ_1_20_CHANNELS 0x10
#define FREQ_1_10_CHANNELS 0x08
#define FREQ_1_5_CHANNELS  0x04
#define FREQ_1_4_CHANNELS  0x02
#define FREQ_1_2_CHANNELS  0x01

// Forward definition of erd file type.
struct xltek_erd_file_t;

/**
 * Struct defining the generic header for Xltek raw data, schema 9 files.
 */
typedef struct xltek_erd_s9_t {
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
} xltek_erd_s9_t;

void xltek_printErdHeaderS9( const xltek_erd_s9_t *head );
struct xltek_erd_file_t *xltek_openErdFileS9( const char *filename );
const char **xltek_getErdLabelsS9( struct xltek_erd_file_t *file );
int xltek_getNextSamplesS9( struct xltek_erd_file_t *file, double *samples );

#ifdef __cplusplus
}
#endif

#endif
