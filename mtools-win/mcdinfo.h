
// set it to 1 (align on byte)
#pragma pack (1)

char strtstr[] = "\nMTOOLS   CDROM Info  v00.12.00    Forever Young Software 1984-2016\n";

// Since CDROM's are "universal" to all platforms, if a value stored
//  in one of the following structures is more than a byte, the value
//  is stored twice.  The first being little_endian, the second, big_endian.
struct VOL_DATE {
  bit8u  since_1900;
  bit8u  month;
  bit8u  day;
  bit8u  hour;
  bit8u  min;
  bit8u  sec;
  bit8s  gmt_off;
};

struct DATE_TIME {
  char  year[4];
  char  month[2];
  char  day[2];
  char  hour[2];
  char  min[2];
  char  sec[2];
  char  jiffies[2];
  bit8s gmt_off;
};

struct ROOT {
  bit8u  len;
  bit8u  e_attrib;
  bit32u extent_loc;
  bit32u extent_loc_b;
  bit32u data_len;
  bit32u data_len_b;
  struct VOL_DATE date;
  bit8u  flags;
  bit8u  unit_size;
  bit8u  gap_size;
  bit16u sequ_num;
  bit16u sequ_num_b;
  bit8u  fi_len;
  bit8u  ident;
};

struct PVD {
  bit8u  type;
  char   ident[5];
  bit8u  ver;
  union {
    bit8u  resv0;   // pvd: type 1
    bit8u  vflags;  // pvd: type 2
  };
  char   sys_ident[32];
  char   vol_ident[32];
  bit8u  resv1[8];
  bit32u num_lbas;
  bit32u num_lbas_b;
  union {
    bit8u  resv2[32];
    bit8u  escape_sequ[32];
  };
  bit16u set_size;
  bit16u set_size_b;
  bit16u sequ_num;
  bit16u sequ_num_b;
  bit16u lba_size;
  bit16u lba_size_b;
  bit32u path_table_size;
  bit32u path_table_size_b;
  bit32u PathL_loc;
  bit32u PathLO_loc;
  bit32u PathM_loc;
  bit32u PathMO_loc;
  struct ROOT root; 
  char   set_ident[128];
  char   pub_ident[128];
  char   prep_ident[128];
  char   app_ident[128];
  char   copy_ident[37];
  char   abs_ident[37];
  char   bib_ident[37];
  struct DATE_TIME vol_date;
  struct DATE_TIME mod_date;
  struct DATE_TIME exp_date;
  struct DATE_TIME val_date;
  bit8u  struct_ver;
  bit8u  resv3;
  bit8u  app_use[512];
  bit8u  resv4[653];
};

struct PVD3 {
  bit8u  type;
  char   ident[5];
  bit8u  ver;
  bit8u  resv0;   // pvd: type 1
  char   sys_ident[32];
  char   part_ident[32];
  bit32u part_location;
  bit32u part_location_b;
  bit32u part_size;
  bit32u part_size_b;
  bit8u  app_use[1960];
};


// UDF stuff

enum {
  TAG_ID_PRIMARY = 1,
  TAG_ID_ANCHOR,
  TAG_ID_VOLUME,
  TAG_ID_IMPLEMENT,
  TAG_ID_PARTITION,
  TAG_ID_LOG_VOLUME,
  TAG_ID_UNALLOCATE,
  TAG_ID_TERM,
  TAG_ID_INTEGRITY
};

// ECMA-167: sect 7.2 page 42
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

struct TIMESTAMP {
  bit16u type_tzone;
  bit16u year;
  bit8u  month;
  bit8u  day;
  bit8u  hour;
  bit8u  min;
  bit8u  secs;
  bit8u  centi_secs;
  bit8u  hund_secs;
  bit8u  micro_secs;
};

struct DESC_PVD {
  struct DESC_TAG tag;
  bit32u vd_seq_num;
  bit32u number;
  bit8u  id[32];
  bit16u seq_num;
  bit16u max_seq_num;
  bit16u int_level;
  bit16u max_int_level;
  bit32u char_set_list;
  bit32u max_char_set_list;
  bit8u  set_id[128];
  bit8u  desc_char_set[64];
  bit8u  exp_char_set[64];
  bit8u  abstract[8];
  bit8u  copyright[8];
  bit8u  app_id[32];
  struct TIMESTAMP timestamp;
  bit8u  implementation_id[32];
  bit8u  implementation_use[64];
  bit32u pred_sequ_num;
  bit16u flags;
  bit8u  resv[22];  
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

struct DESC_VDP {
  struct DESC_TAG tag;
  bit32u sequ_num;
  struct EXTENT next;
  bit8u  resv[484];
};

struct DESC_IMPL_USE {
  struct DESC_TAG tag;
  bit32u sequ_num;
  bit8u  id[32];
  bit8u  use[460];
};

struct DESC_UNALLOC {
  struct DESC_TAG tag;
  bit32u sequ_num;
  bit32u cnt;
  struct EXTENT allocs[1];  // at least 1
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

char *sprinf_date_time(struct DATE_TIME *);
void debug(bit8u *, bit32u);

void print_udf_timestamp(struct TIMESTAMP *stamp);
bool check_tag(struct DESC_TAG *tag);
