

char strtstr[] = "\nMTOOLS   Make DOS Image  v00.17.12    Forever Young Software 1984-2015\n\n";

// set it to 1 (align on byte)
#ifdef _MSC_VER
  #if (_MSC_VER > 1000)
    #pragma pack(push, 1)
  #else
    #pragma pack(1)
  #endif
#elif defined(DJGPP)
  #pragma pack(push, 1)
#endif

// http://www.geocities.com/thestarman3/asm/mbr/PartTables.htm#Decoding
struct PART_TBLE {
  bit8u  bi;
  bit8u  s_head;    // 8 bit head count
  bit8u  s_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  bit8u  s_cyl;     // bottom 8 bits
  bit8u  si;
  bit8u  e_head;    // 8 bit head count
  bit8u  e_sector;  // hi 2 bits is hi 2 bits of cyl, bottom 6 bits is sector
  bit8u  e_cyl;     // bottom 8 bits
  bit32u startlba;
  bit32u size;
};

struct S_FAT1216_BPB {
  bit8u  jmps[2];       // The jump short instruction
  bit8u  nop;           // nop instruction;
  char   oemname[8];    // OEM name
  bit16u nBytesPerSec;  // Bytes per sector
  bit8u  nSecPerClust;  // Sectors per cluster
  bit16u nSecRes;       // Sectors reserved for Boot Record
  bit8u  nFATs;         // Number of FATs
  bit16u nRootEnts;     // Max Root Directory Entries allowed
  bit16u nSecs;         // Number of Logical Sectors (0B40h)
  bit8u  mDesc;         // Medium Descriptor Byte
  bit16u nSecPerFat;    // Sectors per FAT
  bit16u nSecPerTrack;  // Sectors per Track
  bit16u nHeads;        // Number of Heads
  bit32u nSecHidden;    // Number of Hidden Sectors
  bit32u nSecsExt;      // This value used when there are more
  bit8u  DriveNum;      // Physical drive number
  bit8u  nResByte;      // Reserved (we use for FAT type (12- 16-bit)
  bit8u  sig;           // Signature for Extended Boot Record
  bit32u SerNum;        // Volume Serial Number
  char   VolName[11];   // Volume Label
  char   FSType[8];     // File system type
  bit8u  filler[384];
  struct PART_TBLE part_tble[4]; // partition table
  bit16u boot_sig;
};

#define  SECT_RES32  32   // sectors reserved

struct S_FAT32_BPB {
  bit8u  jmps[2];
  bit8u  nop;           // nop instruction;
  char   oemname[8];
  bit16u nBytesPerSec;
  bit8u  nSecPerClust;
  bit16u nSecRes;
  bit8u  nFATs;
  bit16u nRootEnts;
  bit16u nSecs;
  bit8u  mDesc;
  bit16u nSecPerFat;
  bit16u nSecPerTrack;
  bit16u nHeads;
  bit32u nSecHidden;
  bit32u nSecsExt;
  bit32u sect_per_fat32; // offset 36 (24h)
  bit16u ext_flags;      // bit 8 = write to all copies of FAT(s).  bit0:3 = which fat is active
  bit16u fs_version;
  bit32u root_base_cluster; //
  bit16u fs_info_sec;
  bit16u backup_boot_sec;
  bit8u  reserved[12];
  bit8u  DriveNum;  // not FAT specific
  bit8u  nResByte;
  bit8u  sig;
  bit32u SerNum;
  char   VolName[11];
  char   FSType[8];
  bit8u  filler[356];
  struct PART_TBLE part_tble[4]; // partition table
  bit16u boot_sig;
};

struct S_FAT32_FSINFO {
	bit32u sig0;              // 0x41615252 ("RRaA")
  bit8u  resv[480];
	bit32u sig1;              // 0x61417272 ("rrAa")
	bit32u free_clust_fnt;    // 0xFFFFFFFF when the count is unknown
	bit32u next_free_clust;   // most recent allocated cluster  + 1
	bit8u  resv1[12];
	bit32u trail_sig;         // 0xAA550000
};


unsigned char boot_code[] = {
  0xFA,           //CLI
  0xB8,0xC0,0x07, //MOV AX,07C0
  0x8E,0xD8,      //MOV DS,AX
  0x8E,0xD0,      //MOV SS,AX
  0xBC,0x00,0x40, //MOV SP,4000
  0xFB,           //STI
  0xBE,0x6B,0x00, //MOV SI,006B
  0xE8,0x06,0x00, //CALL  0156
  0x30,0xE4,      //XOR AH,AH
  0xCD,0x16,      //INT 16
  0xCD,0x18,      //INT 18
  0x50,           //PUSH  AX
  0x53,           //PUSH  BX
  0x56,           //PUSH  SI
  0xB4,0x0E,      //MOV AH,0E
  0x31,0xDB,      //XOR BX,BX
  0xFC,           //CLD
  0xAC,           //LODSB
  0x08,0xC0,      //OR  AL,AL
  0x74,0x04,      //JZ  0167
  0xCD,0x10,      //INT 10
  0xEB,0xF6,      //JMP 015D
  0x5E,           //POP SI
  0x5B,           //POP BX
  0x58,           //POP AX
  0xC3,           //RET
  13,10
};
unsigned char boot_data[] = "Error reading disk or Non-System Disk"
                            "\x0D\x0A"
                            "Press a key to reboot\x00";

unsigned char empty_mbr[] = {
  0xFA, 0xB8, 0xC0, 0x07, 0x8E, 0xD8, 0x8E, 0xD0, 0xBC, 0x00, 0x40, 0xFB, 0xBE, 0x24, 0x00, 0xE8, 
  0x03, 0x00, 0xF4, 0xEB, 0xFD, 0xB4, 0x0E, 0x31, 0xDB, 0xFC, 0xAC, 0x08, 0xC0, 0x74, 0x04, 0xCD, 
  0x10, 0xEB, 0xF6, 0xC3, 0x03, 0x0A, 0x07, 0x49, 0x20, 0x61, 0x6D, 0x20, 0x61, 0x6E, 0x20, 0x65, 
  0x6D, 0x70, 0x74, 0x79, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x20, 0x73, 0x65, 0x63, 0x74, 0x6F, 0x72, 
  0x2E, 0x20, 0x20, 0x49, 0x20, 0x77, 0x69, 0x6C, 0x6C, 0x20, 0x6A, 0x75, 0x73, 0x74, 0x20, 0x68, 
  0x61, 0x6C, 0x74, 0x20, 0x68, 0x65, 0x72, 0x65, 0x2E, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x55, 0xAA
};

#ifdef _MSC_VER
  #if (_MSC_VER > 1000)
    #pragma pack(pop)
  #endif
#elif defined(DJGPP)
  #pragma pack(pop)
#endif
