
// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   Get CDROM Image  v00.12.02    Forever Young Software 1984-2016\n";


// Since CDROM's are "universal" to all platforms, if a value stored
//  in one of the following structures is more than a byte, the value
//  is stored twice.  The first being little_endian, the second, big_endian.
struct VOL_DATE {
  bit8u since_1900;
  bit8u month;
  bit8u day;
  bit8u hour;
  bit8u min;
  bit8u sec;
  bit8s gmt_off;
};

struct PVD {
  bit8u type;
  char  ident[5];
  bit8u ver;
  bit8u resv0;
  char  sys_ident[32];
  char  vol_ident[32];
  bit8u resv1[8];
  bit32u num_lbas;
  bit32u num_lbas_b;
  bit8u resv2[32];
  bit16u set_size;
  bit16u set_size_b;
  bit16u sequ_num;
  bit16u sequ_num_b;
  bit16u lba_size;
  bit16u lba_size_b;
  bit32u path_table_size;
  bit32u path_table_size_b;
  bit16u PathL_loc;    // little endian path location
  bit16u PathL_loc_b;
  bit16u PathLO_loc;
  bit16u PathLO_loc_b;
  bit16u PathM_loc;    // big endian path location
  bit16u PathM_loc_b;
  bit16u PathMO_loc;
  bit16u PathMO_loc_b;
  struct ROOT {
    bit8u len;
    bit8u e_attrib;
    bit32u  extent_loc;
    bit32u  extent_loc_b;
    bit32u  data_len;
    bit32u  data_len_b;
    struct VOL_DATE date;
    bit8u flags;
    bit8u unit_size;
    bit8u gap_size;
    bit16u sequ_num;
    bit16u sequ_num_b;
    bit8u fi_len;
    bit8u ident;
  } root;
  char set_ident[128];
  char pub_ident[128];
  char prep_ident[128];
  char app_ident[128];
  char copy_ident[37];
  char abs_ident[37];
  char bib_ident[37];
  struct VOL_DATE vol_date;
  bit8u date_resv[10];
  char mod_date[17];
  char exp_date[17];
  char val_date[17];
  bit8u struct_ver;
  bit8u resv3;
  bit8u app_use[512];
  bit8u resv4[653];
};


struct TOC_01 {
  char   toc_len[2];
  char   first_sess;
  char   last_sess;
  char   resv0;
  char   addr_control;
  char   first_trk_num;
  char   resv1;
  union {
    struct MSF {
      char  resv;
      char  min;
      char  sec;
      char  frame;
    } msf;
    char   lba[4];
  } abs_lba;
  char  resv2[804-12];
};

/*
// READ_TOC_EX structure(s) and #defines

#define CDROM_READ_TOC_EX_FORMAT_TOC      0x00
#define CDROM_READ_TOC_EX_FORMAT_SESSION  0x01
#define CDROM_READ_TOC_EX_FORMAT_FULL_TOC 0x02
#define CDROM_READ_TOC_EX_FORMAT_PMA      0x03
#define CDROM_READ_TOC_EX_FORMAT_ATIP     0x04
#define CDROM_READ_TOC_EX_FORMAT_CDTEXT   0x05

#define IOCTL_CDROM_BASE              FILE_DEVICE_CD_ROM
#define IOCTL_CDROM_READ_TOC_EX       CTL_CODE(IOCTL_CDROM_BASE, 0x0015, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _CDROM_READ_TOC_EX {
    UCHAR Format    : 4;
    UCHAR Reserved1 : 3; // future expansion
    UCHAR Msf       : 1;
    UCHAR SessionTrack;
    UCHAR Reserved2;     // future expansion
    UCHAR Reserved3;     // future expansion
} CDROM_READ_TOC_EX, *PCDROM_READ_TOC_EX;

typedef struct _TRACK_DATA {
    UCHAR Reserved;
    UCHAR Control : 4;
    UCHAR Adr : 4;
    UCHAR TrackNumber;
    UCHAR Reserved1;
    UCHAR Address[4];
} TRACK_DATA, *PTRACK_DATA;

typedef struct _CDROM_TOC_SESSION_DATA {
    // Header
    UCHAR Length[2];  // add two bytes for this field
    UCHAR FirstCompleteSession;
    UCHAR LastCompleteSession;
    // One track, representing the first track
    // of the last finished session
    TRACK_DATA TrackData[1];
} CDROM_TOC_SESSION_DATA, *PCDROM_TOC_SESSION_DATA;

// End READ_TOC_EX structure(s) and #defines
*/

// UDF stuff

struct DESC_TAG {
  bit16u id;
  bit16u ver;
  bit8u  crc;
  bit8u  resv;
  bit16u tagsernum;
  bit16u desccrc;
  bit16u desccrclen;
  bit32u tagloc;
};

struct EXTENT {
  bit32u length;
  bit32u location;
};

struct AVDP {
  struct DESC_TAG tag;
  struct EXTENT main_vds;
  struct EXTENT resv_vds;
  bit8u  resv[480];
};

struct DESC_LVD {
  struct DESC_TAG tag;
  bit32u seq_num;
  bit8u  char_set[64];
  bit8u  log_id[128];
  bit32u block_size;
  bit8u  domain_id[32];
  bit8u  content_use[16];
  bit32u map_table_len;
  bit32u partition_maps;
  bit8u  implementation_id[32];
  bit8u  implementation_use[128];
  struct EXTENT integrity_seq;
  bit8u  maps[1608];
};

struct LVD_MAP_1 {
  bit8u  type;
  bit8u  len;
  bit16u sequ_num;
  bit16u part_num;
};

struct LVD_MAP_2 {
  bit8u  type;
  bit8u  len;
  bit8u  id[62];
};

struct DESC_PART {
  struct DESC_TAG tag;
  bit32u sequ_num;
  bit16u flags;
  bit16u number;
  bit8u  contents[32];
  bit8u  content_use[128];
  bit32u access_type;
  bit32u start_lba;
  bit32u sectors;
  bit8u  implement[32];
  bit8u  implement_use[128];
  bit8u  resv[156];
};
