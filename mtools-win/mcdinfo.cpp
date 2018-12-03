/************************************************************************ 
  MCDINFO  Get CD Information              v00.12.00
  Forever Young Software      Benjamin David Lunt

  This utility was desinged for use with Bochs to get the info from a CDROM.

  Bochs is located at:
    http://bochs.sourceforge.net

  I designed this program to be used for testing my own OS, though you are 
   welcome to use it any way you wish.

  Please note that I release it and it's code for others to use and do with 
   as they want.  You may copy it, modify it, do what ever you want with it
   as long as you release the source code and display this entire comment 
   block in your source or documentation file.
   (you may add to this comment block if you so desire)

  Please use at your own risk.  I do not specify that this code is correct 
   and unharmful.  No warranty of any kind is given for its release.

  I take no blame for what may or may not happen by using this code with 
   your purposes.

  'nuff of that!  You may modify this to your liking and if you see that it 
   will help others with their use of Bochs, please send the revised code to
   fys@fysnet.net.  I will then release it as I have this one.

  You may get the latest and greatest at:
    http://www.fysnet.net/mtools.htm

  Thanks, and thanks to those who contributed to Bochs....

  ********************************************************

  Things to know:
  - This only works with WinXP
    (As far as I know.  If it works for you on something else, let me know)
  - This code is quite messy.  I only write it to test a few things and
    get information from the disc.  It is not intended to be a release
    ready product.  Use caution when modifying.
  
  ********************************************************

  Compiles as is with MS VC++ 6.x         (Win32 .EXE file)

  ********************************************************

  Usage:
    When reading a physical CD, just run the file
    When reading an ISO image, give image name on command line

************************************************************************/

// don't know which ones are needed or not needed.  I just copied them
//  across from another project. :)
#include <ctype.h>
#include <conio.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include <math.h>
#include <time.h>

#include <windows.h>

#include "ctype.h" // our types include
#include "mcdinfo.h"   // our include

#define SECT_SIZE 2048
#define MIN_SECTORS    32   // needs to be atleast 16

char access_type_str[8][16] = {
  "Not specified",  // 0
  "Read Only",      // 1
  "Write Once",     // 2
  "Re-writable",    // 3
  "Over-writable",  // 4
  "undefined",      // 5
  "undefined",      // 6
  "undefined"       // 7
};

char yes_no_str[2][4] = { "no", "yes" };  // 0 = no, 1 = yes

char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

//// ***** WARNING.  At current, this only works on NT machines **********
//                    When reading actual CD's

int main(int argc, char *argv[]) {
  HANDLE hFile;
  char drive[128], temp[128], drvletter;
  bit32u ntemp, ul;
  LARGE_INTEGER pos;
  bit8u *pathtable;
  int i, j, cur_vol_sect;
  char temp_str[256];
  struct PVD *pvd;
  bit8u *buffer;
  struct DESC_TAG *tag;
  
  // print start string
  fprintf(stderr, strtstr);

  if (argc == 2) {
    strcpy(drive, argv[1]);
  } else {
    // Make sure we are a version of Windows that allows direct disk access.
    OSVERSIONINFO os_ver_info;
    os_ver_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&os_ver_info);
    if (os_ver_info.dwMajorVersion < 5) {
      printf("\nThis utility only works with WinXP and possibly earlier vesrions of WinNT");
      if (os_ver_info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        printf("\nDid not find WinXP, but did find an NT version of Windows.  Continue? (Y|N)");
        gets(temp);
        if (strcmp(temp, "Y") && strcmp(temp, "Yes") && strcmp(temp, "YES"))
          return -1;
      } else
        return -1;
    }
    
    do {
      fprintf(stderr, "\n  Drive letter [d]: ");
      gets(temp);
      if (!strlen(temp))
        drvletter = 'd';
      else
        drvletter = tolower(temp[0]);
    } while ((drvletter < 'd') || (drvletter > 'z'));
    sprintf(drive, "\\\\.\\%c:", drvletter);
  }
  
  hFile = CreateFile((char *) &drive, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL); 
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("\n Error opening file/drive.");
    return -2;
  }
  
  // allocate our buffer
  buffer = (bit8u *) VirtualAlloc(NULL, SECT_SIZE * MIN_SECTORS, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  
  puts("");
  
  pos.HighPart = 0;
  pos.LowPart = SetFilePointer(hFile, (16 * SECT_SIZE), &pos.HighPart, SEEK_SET);
  ReadFile(hFile, buffer, SECT_SIZE, &ntemp, NULL);
  if (ntemp != SECT_SIZE) {
    printf(" Did not read all of the first sector.\n");
    return -3;
  }
  
  // see what kind of filesystem is on it
  //
  // ISO9660 ?
  if (memcmp(&buffer[1], "CD001", 5) == 0) {
    pvd = (struct PVD *) buffer;
    printf("Found ISO 9660 disc: lbas = %i  size = %i\n", pvd->num_lbas, pvd->set_size);
    
    memset(temp, 0, sizeof(temp));
    printf("                Label:  value  <normal>\n"
           " -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("                 Type:  %i      <1-3>\n", pvd->type);
    if ((pvd->type < 1) || (pvd->type > 3)) {
      printf(" **** We only know about type 1, 2, or type 3.\n");
      exit(1);
    }
  
    if ((pvd->type >= 1) && (pvd->type <= 2)) {
      memcpy(temp, pvd->ident, 5);
      printf("  Standard Identifier:  %5s  <CD001>\n"
             "   Descriptor Version:  %i      <1>\n", temp, pvd->ver);
      if (pvd->type == 1)
        printf("         Unused Field:  %02X     <00>\n", pvd->resv0);
      else
        printf("         Volume Flags:  %02X     <00>\n", pvd->vflags);
      memcpy(temp_str, pvd->sys_ident, 32);
      temp_str[32] = 0;
      printf("    System Identifier:  [%s]\n", temp_str);
      memcpy(temp_str, pvd->vol_ident, 32);
      temp_str[32] = 0;
      printf("    Volume Identifier:  [%s]\n", temp_str);
      printf("         Unused Field:  %02X %02X %02X %02X %02X %02X %02X %02X  <zeros>\n",
        pvd->resv1[0], pvd->resv1[1], pvd->resv1[2], pvd->resv1[3], pvd->resv1[4], pvd->resv1[5], pvd->resv1[6], pvd->resv1[7]);
      printf("    Sectors in Volume:  %i {%i} (%3.2f meg)\n", pvd->num_lbas, ENDIAN_32(pvd->num_lbas_b), (double) ((double) pvd->num_lbas / 512));
  
      if (pvd->type == 1) {
        printf("         Unused Field:  %02X %02X %02X %02X %02X %02X %02X %02X  <zeros>\n",
          pvd->resv2[0], pvd->resv2[1], pvd->resv2[2], pvd->resv2[3], pvd->resv2[4], pvd->resv2[5], pvd->resv2[6], pvd->resv2[7]);
        printf("                        %02X %02X %02X %02X %02X %02X %02X %02X  <zeros>\n",
          pvd->resv2[8], pvd->resv2[9], pvd->resv2[10], pvd->resv2[11], pvd->resv2[12], pvd->resv2[13], pvd->resv2[14], pvd->resv2[15]);
        printf("                        %02X %02X %02X %02X %02X %02X %02X %02X  <zeros>\n",
          pvd->resv2[16], pvd->resv2[17], pvd->resv2[18], pvd->resv2[19], pvd->resv2[20], pvd->resv2[21], pvd->resv2[22], pvd->resv2[23]);
        printf("                        %02X %02X %02X %02X %02X %02X %02X %02X  <zeros>\n",
          pvd->resv2[24], pvd->resv2[25], pvd->resv2[26], pvd->resv2[27], pvd->resv2[28], pvd->resv2[29], pvd->resv2[30], pvd->resv2[31]);
      } else if (pvd->type == 2) {
        printf("     Escape Sequences:\n");
        debug(pvd->escape_sequ, 32);
      } 
  
      printf("      Volume Set Size:  %i {%i}\n", pvd->set_size, ENDIAN_16(pvd->set_size_b));
      printf("  Volume Sequence num:  %i {%i}\n", pvd->sequ_num, ENDIAN_16(pvd->sequ_num_b));
      printf("   Logical Block Size:  %i {%i} <2048>\n", pvd->lba_size, ENDIAN_16(pvd->lba_size_b));
         
      ntemp = (pvd->path_table_size / SECT_SIZE) + (pvd->path_table_size % SECT_SIZE ? 1 : 0); 
      printf("      Path Table Size:  %i {%i} (%i sectors)\n", pvd->path_table_size, ENDIAN_32(pvd->path_table_size_b), ntemp);
      
      printf("  Path Table Location:  %i\n", pvd->PathL_loc);
      printf(" OPath Table Location:  %i\n", pvd->PathLO_loc);
      printf(" MPath Table Location:  %i (big-endian)\n", ENDIAN_32(pvd->PathM_loc));
      printf("MOPath Table Location:  %i (big-endian)\n", ENDIAN_32(pvd->PathMO_loc));
      
      printf("  *** Root  ***"
             "         Entry Length:  %i <34>\n", pvd->root.len);
      printf("      Extended Attrib:  %i <2>\n", pvd->root.e_attrib);
      printf("      Extent Location:  %i {%i}\n", pvd->root.extent_loc, ENDIAN_32(pvd->root.extent_loc_b));
      printf("          Data Length:  %i {%i}  (%i sectors)\n", pvd->root.data_len, ENDIAN_32(pvd->root.data_len_b),
        ((pvd->root.data_len / SECT_SIZE) + (pvd->root.data_len % SECT_SIZE ? 1 : 0)));
      printf("        Date and Time:  %02i%s%4i %02i:%02i:%02i%c (GMT %i)\n", 
        pvd->root.date.day, months[pvd->root.date.month-1], pvd->root.date.since_1900 + 1900,
        (pvd->root.date.hour <= 12) ? pvd->root.date.hour : pvd->root.date.hour - 12, pvd->root.date.min, pvd->root.date.sec, 
        (pvd->root.date.hour <= 12) ? 'a' : 'p', pvd->root.date.gmt_off);
      
      printf("                Flags:  %i\n", pvd->root.flags);
      printf("            Unit Size:  %i\n", pvd->root.unit_size);
      printf("  Interleave Gap Size:  %i\n", pvd->root.gap_size);
      printf("  Volume Sequence Num:  %i {%i}\n", pvd->root.sequ_num, ENDIAN_16(pvd->root.sequ_num_b));
      printf("    File Ident Length:  %i <1>\n", pvd->root.fi_len);
      printf("           Identifier:  %i <0>\n", pvd->root.ident);
      
      memcpy(temp_str, pvd->set_ident, 127);
      temp_str[127] = 0;
      printf("Volume Set Identifier: [%s]\n", temp_str);
      
      memcpy(temp_str, pvd->pub_ident, 127);
      printf(" Publisher Identifier: [%s]\n", temp_str);
      
      memcpy(temp_str, pvd->prep_ident, 127);
      printf("  Preparer Identifier: [%s]\n", temp_str);
    
      memcpy(temp_str, pvd->app_ident, 127);
      printf("       App Identifier: [%s]\n", temp_str);
      
      memcpy(temp_str, pvd->copy_ident, 37);
      temp_str[37] = 0;
      printf(" Copyright Identifier: [%s]\n", temp_str);
      
      memcpy(temp_str, pvd->abs_ident, 37);
      printf("  Abstract Identifier: [%s]\n", temp_str);
      
      memcpy(temp_str, pvd->bib_ident, 37);
      printf("    Biblio Identifier: [%s]\n", temp_str);
      
      printf(" Volume Date and Time:  %s\n", sprinf_date_time(&pvd->vol_date));
      printf("        Last Modified:  %s\n", sprinf_date_time(&pvd->mod_date));
      printf("    Volume Expiration:  %s\n", sprinf_date_time(&pvd->exp_date));
      printf("     Volume Effective:  %s\n", sprinf_date_time(&pvd->val_date));
      
      printf("   File Structure Ver:  %i\n", pvd->struct_ver);

      printf("         Unused Field:  %02X\n", pvd->resv3);
  
      printf("Application Use:\n");
      debug(pvd->app_use, 512);
      
      printf("Unused Field:\n");
      debug(pvd->resv4, 653);
      
      printf("\n PATH TABLE ENTRIES:\n");
      
      pathtable = (bit8u *) calloc(SECT_SIZE * ntemp, sizeof(bit8u));
      if (pathtable == NULL) {} // TODO: error
      pos.HighPart = 0;
      pos.LowPart = SetFilePointer(hFile, (pvd->PathL_loc * SECT_SIZE), &pos.HighPart, SEEK_SET);
      ReadFile(hFile, (void *) pathtable, SECT_SIZE * ntemp, (bit32u *) &ntemp, NULL);
      if (ntemp == 0) {} // TODO: error
      j = 0;
      ntemp = 0;
      do {
        memset(temp, 0, 128);
        for (i=0; i<(bit8u) *pathtable; i++)
          temp[i] = (char) *(pathtable+8+i);
        printf(" Path Table Entry %i\n"
             "   Logical Block: %i\n"
             "          Parent: %i\n"
             "      Identifier: %s\n",
             j++, (unsigned short) *(pathtable+2),
             (unsigned short) *(pathtable+6), temp
             );
        i = 8;
        i += (bit8u) *pathtable;
        i += (bit8u) *pathtable & 1;
        pathtable += i;
        ntemp += i;
      } while (ntemp < pvd->path_table_size);
    } else if (pvd->type == 3) {
      struct PVD3 *pvd3 = (struct PVD3 *) &pvd;
      memcpy(temp, pvd3->ident, 5);
      printf("  Standard Identifier:  %5s  <CD001>\n", temp);
      printf("         Unused Field:  %02X     <00>\n", pvd3->resv0);
      memcpy(temp_str, pvd3->sys_ident, 32);
      temp_str[32] = 0;
      printf("    System Identifier:  [%s]\n", temp_str);
      memcpy(temp_str, pvd3->part_ident, 32);
      temp_str[32] = 0;
      printf(" Partition Identifier:  [%s]\n", temp_str);
      printf("   Partition Location:  %i {%i}\n", pvd3->part_location, ENDIAN_32(pvd3->part_location_b));
      printf("       Partition Size:  %i {%i}\n", pvd3->part_size, ENDIAN_32(pvd3->part_size_b));
      printf("\nApplication Use:");
      debug(pvd3->app_use, 1960);
    }
  } else

  // UDF ?
  if (memcmp(&buffer[1], "BEA01", 5) == 0) {
    // on sector boundaries, starting with BEA01 (sector 16), until TEA01.
    cur_vol_sect = 17;
    while (1) {
      pos.HighPart = 0;
      pos.LowPart = SetFilePointer(hFile, (cur_vol_sect * SECT_SIZE), &pos.HighPart, SEEK_SET);
      ReadFile(hFile, buffer, SECT_SIZE, &ntemp, NULL);
      // should be NSR02 or NSR03 for UDF discs
      if ((memcmp(&buffer[1], "NSR02", 5) == 0) ||
          (memcmp(&buffer[1], "NSR03", 5) == 0)) {
        // we are a UDF type format
        // Anchor Volume Descriptor Pointer (always at sector 256)
        printf(" Looking for Anchor Volume Descriptor Pointer at LBA 256\n");
        pos.QuadPart = (256 * SECT_SIZE);
        SetFilePointer(hFile, pos.LowPart, &pos.HighPart, FILE_BEGIN);
        ReadFile(hFile, buffer, SECT_SIZE, &ntemp, NULL);
        tag = (struct DESC_TAG *) buffer;
        if (tag->id != TAG_ID_ANCHOR) {
          printf(" Tag.ID != 2 at sector 256...\n");
          break;
        }
        if (!check_tag(tag))
          break;
        
        // read Main Volume Descriptor Sequence
        struct AVDP *avdp = (struct AVDP *) buffer;
        int len = avdp->main_vds.length;
        printf(" Main Volume Descriptor Sequence at LBA %i (len = %i)\n", avdp->main_vds.location, avdp->main_vds.length);
        pos.QuadPart = (avdp->main_vds.location * SECT_SIZE);
        SetFilePointer(hFile, pos.LowPart, &pos.HighPart, FILE_BEGIN);
        ReadFile(hFile, buffer, avdp->main_vds.length, &ntemp, NULL);
        
        // buffer now contains the Main Volume Descriptor Sequence
        struct DESC_TAG *tag = (struct DESC_TAG *) buffer;
        struct DESC_LVD *lvd;
        struct DESC_PVD *pvd;
        struct DESC_VDP *vdp;
        struct DESC_IMPL_USE *use;
        struct DESC_PART *part;
        struct DESC_UNALLOC *unalloc;
        struct LVD_MAP_1 *map;
        struct LVD_MAP_2 *map2;
        while (len > 0) {
          switch (tag->id) {
            case TAG_ID_PRIMARY:  // Primary Volume Descriptor
              printf(" Found Primary Volume Descriptor\n");
              if (!check_tag(tag))
                break;
              pvd = (struct DESC_PVD *) tag;
              printf("   Primary Sequence Number: %i\n", pvd->vd_seq_num);
              printf("   Primary Volume Number: %i\n", pvd->number);
              printf("   Min and Max: %i  %i\n", pvd->seq_num, pvd->max_seq_num);
              printf("   Volume Set Id:\n");
              debug(pvd->set_id, 128);
              printf("   Copyright:\n");
              debug(pvd->copyright, 8);
              printf("   Timestamp:\n");
              print_udf_timestamp(&pvd->timestamp);
              break;
            case TAG_ID_ANCHOR:  // AVDP (shouldn't find this one)
              printf(" Found AVDP\n");
              if (!check_tag(tag))
                break;
              break;
            case TAG_ID_VOLUME:  // Volume Descriptor Pointer
              printf(" Found Volume Descriptor Pointer.\n");
              if (!check_tag(tag))
                break;
              vdp = (struct DESC_VDP *) tag;
              printf("   Volume Sequence Number: %i\n", vdp->sequ_num);
              printf("   Next at %i, length %i\n", vdp->next.location, vdp->next.length);
              break;
            case TAG_ID_IMPLEMENT:  // Implement Use Volume Descriptor
              printf(" Found Implement Use Volume Descriptor\n");
              if (!check_tag(tag))
                break;
              use = (struct DESC_IMPL_USE *) tag;
              printf("   Sequence Number: %i\n", use->sequ_num);
              printf(" ID:\n");
              debug(part->implement, 32);
              printf(" Use:\n");
              debug(part->implement_use, 460);
              break;
            case TAG_ID_PARTITION:  // Partition Descriptor
              printf(" Found Partition Descriptor\n");
              if (!check_tag(tag))
                break;
              part = (struct DESC_PART *) tag;
              printf("  num: %i  sequ %i\n", part->number, part->sequ_num);
              printf("  Starting LBA: %i, sectors: %i, allocated: %s\n", part->start_lba, part->sectors, yes_no_str[part->flags & 1]);
              printf("  Contents:\n");
              debug(part->contents, 32);
              printf("  access type: %s\n", access_type_str[part->access_type & 0x07]);  // 0x07 to make sure not above 7
              printf("  Implementation and Use:\n");
              debug(part->implement, 32);
              debug(part->implement_use, 128);
              break;
            case TAG_ID_LOG_VOLUME:  // Logical Volume Descriptor
              printf(" Found Logical Volume Descriptor\n");
              if (!check_tag(tag))
                break;
              lvd = (struct DESC_LVD *) tag;
              printf("   block size = %i\n", lvd->block_size);
              printf("   number of partition map entries = %i\n", lvd->partition_maps);
              map = (struct LVD_MAP_1 *) lvd->maps;
              for (ul=0; ul<lvd->partition_maps; ul++) {
                if (map->type == 1)
                  printf("    Map %i: type 1:  sequ = %i, part = %i\n", ul, map->sequ_num, map->part_num);
                else if (map->type == 2) {
                  map2 = (struct LVD_MAP_2 *) map;
                  printf("    Map %i: type 2:  [%02X %02X %02X %02X]\n", ul, map2->id[0], map2->id[1], map2->id[2], map2->id[3]);
                } else 
                  break;
                map = (struct LVD_MAP_1 *) ((bit32u) map + map->len);
              }
              break;
            case TAG_ID_UNALLOCATE:  // Unallocated Space Descriptor
              printf(" Found Unallocated Space Descriptor\n");
              if (!check_tag(tag))
                break;
              unalloc = (struct DESC_UNALLOC *) tag;
              printf("   Sequence Number: %i\n", unalloc->sequ_num);
              printf("   Count: %i\n", unalloc->cnt);
              for (ul=0; ul<unalloc->cnt; ul++)
                printf("     start: %10i, length %i (%i)\n", unalloc->allocs[ul].location, unalloc->allocs[ul].length, unalloc->allocs[ul].length / SECT_SIZE);
              break;
            case TAG_ID_TERM:  // Terminating Descriptor
              printf(" Found Terminating Descriptor\n");
              if (!check_tag(tag))
                break;
              len = SECT_SIZE; // so that we exit after this
              break;
            case TAG_ID_INTEGRITY:  // Logical Volume Integrity Descriptor
              printf(" Found Logical Volume Integrity Descriptor\n");
              if (!check_tag(tag))
                break;
              break;
            default:
              printf(" Found unknown Volume Descriptor (0x%04X)\n", tag->id);
              if (!check_tag(tag))
                break;
              len = SECT_SIZE; // so that we exit after this
          }
          len -= SECT_SIZE;
          tag = (struct DESC_TAG *) ((bit32u) tag + SECT_SIZE);
        }
      } else
      if (memcmp(&buffer[1], "TEA01", 5) == 0)
        break;
      else
        printf(" Unknown Identifier: [%c%c%c%c%c]\n", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
      // move to next identifier
      // (check incase we don't miss TEA01)
      if (++cur_vol_sect > 32)
        break;
    }
  } else {
    // must be an unknown type of format
    printf(" Did not find ""CD001"" or ""BEA01"" signature at found session start\n");
    printf(" Found [%c%c%c%c%c] instead.\n", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
  }
  
  printf("\n\n Legend:"
         "  []:   Strings are enclosed in [] so to see all characters.\n"
         "  {}:   Big Endian values are converted to Little Endian and shown in {}.\n"
         "  <>:   These are values that this item should be.  The normal value.\n"
         "  ():   More information for the item shown.\n\n");
  
  VirtualFree(buffer, 0, MEM_RELEASE);
  CloseHandle(hFile);
  
  return 0;
}

//                                             0         1         2         3
//                                             012345678901234567890123456789012
// return a string with the following format:  DD/MM/YYYY  HH:MM:SS.jj (xxx GMT)
char date_string[128];

char *sprinf_date_time(struct DATE_TIME *date) {
  
  strcpy(date_string, "DD/MM/YYYY  HH:MM:SS.jj           ");
  
  memcpy(&date_string[0], date->day, 2);
  memcpy(&date_string[3], date->month, 2);
  memcpy(&date_string[6], date->year, 4);
  
  memcpy(&date_string[12], date->hour, 2);
  memcpy(&date_string[15], date->min, 2);
  memcpy(&date_string[18], date->sec, 2);
  memcpy(&date_string[21], date->jiffies, 2);
  
  sprintf(&date_string[24], "(%i GMT)", date->gmt_off);
  
  date_string[33] = 0;
  
  return date_string;
}

void debug(bit8u *data, bit32u size) {

  bit32u offset = 0;
  bit8u *temp_buf;
  unsigned i;
  
  while (size) {
    printf("   ");
    offset += 16;
    temp_buf = data;
    for (i=0; (i<16) && (i<size); i++)
      printf("%02X%c", *temp_buf++, (i==7) ? ((size>8) ? '-' : ' ') : ' ');
    for (; i<16; i++)
      printf("   ");
    printf("   ");
    for (i=0; (i<16) && (i<size); i++) {
      putchar(isprint(*data) ? *data : '.');
      data++;
    }
    puts("");
    size -= i;
  }
}

void print_udf_timestamp(struct TIMESTAMP *stamp) {
  int offset = (stamp->type_tzone & 0x0FFF);
  
  printf("    %04i/%02i/%02i  %02i:%02i:%02i (%i)(%i)(%i)", 
    stamp->year, stamp->month, stamp->day, 
    stamp->hour, stamp->min, stamp->secs,
    stamp->centi_secs, stamp->hund_secs, stamp->micro_secs);
  switch ((stamp->type_tzone & 0xF000) >> 12) {
    case 0:
      if ((offset & 0x3FF) < 1440)
        printf(" UTC%+i\n", -((offset & 0x3FF) / 60)); // this isn't really correct...But for now...
      else
        printf(" UTC\n");
      break;
    case 1:
      printf(" local time\n");
      break;
    default:
      printf("\n");
  }
}

// checks the CRC of the passed tag struct
//  returns TRUE if passed
bool check_tag(struct DESC_TAG *tag) {
  bit8u crc = 0, *t = (bit8u *) tag;
  int i;
  
  for (i=0;i<sizeof(struct DESC_TAG); i++) {
    if (i == 4)
      continue;
    crc += t[i];
  }
  
  if (crc != t[4])
    printf("  Tag for this item did not pass CRC check... <%02X>\n", t[4]);
  
  return (crc == t[4]);
}
