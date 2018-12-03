/************************************************************************ 
  MCDINFO  Get CD Image                 v00.12.02
  Forever Young Software      Benjamin David Lunt

  This utility was desinged for use with Bochs to get an image from a CDROM.

  Bochs is located at:
    http://bochs.sourceforge.net

  I designed this program to be used for testing my own OS, though you are 
   welcome to use it any way you wish.
  
  Please note that I release it and it's code for others to use and do with as 
   they want.  You may copy it, modify it, do whatever you want with it as long
   as you release the source code and display this entire comment block in your
   source or documentation file. (you may add to this comment block if you so desire)
  
  Please use at your own risk.  I do not specify that this code is correct 
   and unharmful.  No warranty of any kind is given for its release.

  I take no blame for what may or may not happen by using this code with your
   purposes.

  'nuff of that!  You may modify this to your liking and if you see that it will
   help others with their use of Bochs, please send the revised code to 
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
    Nothing.  Just run it...

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
#include <winbase.h>

#include "ctype.h"   // our types include

#include "ntddcdrm.h"
#include "mgetcd.h"   // our include

#define SECT_SIZE    2048
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

int main(int argc, char *argv[]) {
  HANDLE cdFile, imgFile;
  char drive[16], drvletter;
  char filename[128], temp[128];
  bit32u ntemp, ul;
  LARGE_INTEGER pos, last_session_start;
  char ret_val;
  bit32u num_lbas = 0;
  
  // print start string
  printf(strtstr);
  
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
  
  printf("\n  Filename of image to create [cdrom.img]: ");
  gets(filename);
  if (!strlen(filename)) strcpy(filename, "cdrom.img");
  
  do {
    printf("                         Drive letter [d]: ");
    gets(temp);
    if (!strlen(temp))
      drvletter = 'd';
    else
      drvletter = tolower(temp[0]);
  } while ((drvletter < 'd') || (drvletter > 'z'));
  sprintf(drive, "\\\\.\\%c:", drvletter);
  
  imgFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
  if (imgFile == INVALID_HANDLE_VALUE) {} // TODO: error
  
  cdFile = CreateFile(drive, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (cdFile == INVALID_HANDLE_VALUE) {
    printf("\n ERROR opening drive: %i", GetLastError());
    return 0;
  }
  
  // Lock the compact disc in the CD-ROM drive to prevent accidental removal while reading from it.
  PREVENT_MEDIA_REMOVAL pmrLockCDROM;
  pmrLockCDROM.PreventMediaRemoval = TRUE;
  DeviceIoControl(cdFile, IOCTL_CDROM_MEDIA_REMOVAL, &pmrLockCDROM, sizeof(pmrLockCDROM), NULL, 0, &ntemp, NULL);
  
  // Get sector size of compact disc
  DISK_GEOMETRY dgCDROM;
  DeviceIoControl(cdFile, IOCTL_CDROM_GET_DRIVE_GEOMETRY, NULL, 0, &dgCDROM, sizeof(dgCDROM), &ntemp, NULL);
  if (dgCDROM.BytesPerSector != SECT_SIZE) {
    printf("\n Error: Disc Sector size isn't 2048 (%i)", dgCDROM.BytesPerSector);
    return 0xFF;
  }
  
  // allocate our buffer
  bit8u *buffer = (bit8u *) VirtualAlloc(NULL, SECT_SIZE * MIN_SECTORS, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  
  CDROM_READ_TOC_EX toc;
  memset(&toc, 0, sizeof(toc));
  toc.Format = CDROM_READ_TOC_EX_FORMAT_SESSION;
  toc.Msf = 0;
  toc.SessionTrack = 0;
  
  _CDROM_TOC_SESSION_DATA *data = (_CDROM_TOC_SESSION_DATA *) buffer;
  DeviceIoControl(cdFile, IOCTL_CDROM_READ_TOC_EX, &toc, sizeof(toc), data, 804, &ntemp, NULL);
  
  int tot_sessions = data->LastCompleteSession - data->FirstCompleteSession + 1;
  printf("\n Found %i session(s).", tot_sessions);
  
  last_session_start.QuadPart = (data->TrackData[0].Address[0] << 24) | (data->TrackData[0].Address[1] << 16) | 
    (data->TrackData[0].Address[2] << 8) | (data->TrackData[0].Address[3] << 0);
  
  printf("\n Last starts at %lli", last_session_start.QuadPart);
  
  pos.QuadPart = ((last_session_start.QuadPart + 16) * SECT_SIZE);
  pos.LowPart = SetFilePointer(cdFile, pos.LowPart, &pos.HighPart, FILE_BEGIN);
  
  if (pos.QuadPart != ((last_session_start.QuadPart + 16) * SECT_SIZE)) {
    printf("\n Could not seek to %li.  Seeked to %li", ((last_session_start.QuadPart + 16)*SECT_SIZE), pos.QuadPart);
    return -1;
  }
  
  
  // now read in the descriptors
  ret_val = ReadFile(cdFile, buffer, SECT_SIZE, &ntemp, NULL);
  //printf("\n **** read %i sectors, ntemp = %i  error = %i", ret_val, ntemp, GetLastError());
  
  if (!ret_val || (ret_val && (ntemp < SECT_SIZE))) {
    printf("\n Error reading the first two sectors.  Ret_val = %i, size read = %i", ret_val, ntemp);
    return -1;
  }
  
  if (memcmp(&buffer[1], "CD001", 5) == 0) {
    struct PVD *pvd = (struct PVD *) buffer;
    printf("\n Found ISO 9660 disc: lbas = %i  size = %i", pvd->num_lbas, pvd->set_size);
    num_lbas = pvd->num_lbas;
  } else if (memcmp(&buffer[1], "BEA01", 5) == 0) {
    // read in the next sector
    ret_val = ReadFile(cdFile, buffer, SECT_SIZE, &ntemp, NULL);

    // should be NSR02 or NSR03 for UDF discs
    if ((memcmp(&buffer[1], "NSR02", 5) == 0) ||
        (memcmp(&buffer[1], "NSR03", 5) == 0)) {
      
      // Anchor Volume Descriptor Pointer (always at sector 256)
      printf("\n Found Anchor Volume Descriptor Pointer at LBA 256");
      pos.QuadPart = (256 * SECT_SIZE);
      SetFilePointer(cdFile, pos.LowPart, &pos.HighPart, FILE_BEGIN);
      ret_val = ReadFile(cdFile, buffer, SECT_SIZE, &ntemp, NULL);
      //WriteFile(imgFile, buffer, SECT_SIZE, &ntemp, NULL);
      
      // read Main Volume Descriptor Sequence
      struct AVDP *avdp = (struct AVDP *) buffer;
      int len = avdp->main_vds.length;
      printf("\n Main Volume Descriptor Sequence at LBA %i (len = %i)", avdp->main_vds.location, avdp->main_vds.length);
      pos.QuadPart = (avdp->main_vds.location * SECT_SIZE);
      SetFilePointer(cdFile, pos.LowPart, &pos.HighPart, FILE_BEGIN);
      ret_val = ReadFile(cdFile, buffer, avdp->main_vds.length, &ntemp, NULL);
      
      // buffer now contains the Main Volume Descriptor Sequence
      struct DESC_TAG *tag = (struct DESC_TAG *) buffer;
      struct DESC_LVD *lvd;
      struct DESC_PART *part;
      struct LVD_MAP_1 *map;
      struct LVD_MAP_2 *map2;
      while (len > 0) {
        switch (tag->id) {
          case 0x0001:  // Primary Volume Descriptor
            printf("\n Found Primary Volume Descriptor");
            break;
          case 0x0002:  // AVDP (shouldn't find this one)
            printf("\n Found AVDP");
            break;
          case 0x0003:  // Volume Descriptor Pointer
            printf("\n Found VDP");
            break;
          case 0x0004:  // Implement Use Volume Descriptor
            printf("\n Found Implement Use Volume Descriptor");
            break;
          case 0x0005:  // Partition Descriptor
            printf("\n Found Partition Descriptor");
            part = (struct DESC_PART *) tag;
            printf("\n  num: %i  sequ %i", part->number, part->sequ_num);
            printf("\n  Starting LBA: %i, sectors: %i, allocated: %s", part->start_lba, part->sectors, yes_no_str[part->flags & 1]);
            printf("\n  [%c%c%c%c%c]", part->contents[1], part->contents[2], part->contents[3], part->contents[4], part->contents[4]);
            printf("\n  access type: %s", access_type_str[part->access_type & 0x07]);  // 0x07 to make sure not above 7
            printf("\n  Implementation: [%s] [%s]", part->implement, part->implement_use);
            // if is allocated and last sector is above our count, then make our count above last sector
            if ((part->flags & 1) &&
               ((part->start_lba + part->sectors) > num_lbas))
              num_lbas = (part->start_lba + part->sectors);
            break;
          case 0x0006:  // Logical Volume Descriptor
            printf("\n Found Logical Volume Descriptor");
            lvd = (struct DESC_LVD *) tag;
            printf("\n   block size = %i", lvd->block_size);
            printf("\n   number of partition map entries = %i", lvd->partition_maps);
            map = (struct LVD_MAP_1 *) lvd->maps;
            for (ul=0; ul<lvd->partition_maps; ul++) {
              if (map->type == 1)
                printf("\n    Map %i: type 1:  sequ = %i, part = %i", ul, map->sequ_num, map->part_num);
              else if (map->type == 2) {
                map2 = (struct LVD_MAP_2 *) map;
                printf("\n    Map %i: type 2:  [%02X %02X %02X %02X]", ul, map2->id[0], map2->id[1], map2->id[2], map2->id[3]);
              } else 
                break;
              map = (struct LVD_MAP_1 *) ((bit32u) map + map->len);
            }
            if (lvd->block_size != SECT_SIZE)
              printf("\n ***** Block size doesn't equal %i *****", SECT_SIZE);
            break;
          case 0x0007:  // Unallocated Space Descriptor
            printf("\n Found Unallocated Space Descriptor");
            break;
          case 0x0008:  // Terminating Descriptor
            printf("\n Found Terminating Descriptor");
            len = 2048;
            break;
          case 0x0009:  // Logical Volume Integrity Descriptor
            printf("\n Found Logical Volume Integrity Descriptor");
            break;
          default:
            printf("\n Found unknown Volume Descriptor (0x%04X)", tag->id);
            len = 2048;
        }
        len -= 2048;
        tag = (struct DESC_TAG *) ((bit32u) tag + 2048);
      }
    } else {
      printf("\n Did not find NSR0x descriptor...");
      return -1;
    }
    printf("\n Found a size of %i sectors (%i meg)...Continue (Y|N): [N]", num_lbas, (num_lbas * SECT_SIZE) / 1000000);
    gets(temp);
    if (strlen(temp) && (strcmp(temp, "Y") != 0))
      return 0;
  } else {
    printf("\n Did not find ""CD001"" or ""BEA01"" signature at found session start");
    printf("\n Found [%c%c%c%c%c] instead.", buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
    return -1;
  }
  
  // rewind(cdFile)
  printf("\n\n");
  SetFilePointer(cdFile, last_session_start.LowPart, &last_session_start.HighPart, FILE_BEGIN);
  
  // read in the blocks
  for (ul=0; ul<num_lbas; ul++) {
    printf("\r Sector %i of %i", ul + 1, num_lbas);
    ret_val = ReadFile(cdFile, buffer, SECT_SIZE, &ntemp, NULL);
    if (ret_val && (ntemp == SECT_SIZE))
      WriteFile(imgFile, buffer, SECT_SIZE, &ntemp, NULL);
    else if (!ret_val && (ntemp == 0)) {
      memset(buffer, 0, SECT_SIZE);
      WriteFile(imgFile, buffer, SECT_SIZE, &ntemp, NULL);
    } else {
      printf("\n There was an unknown type of error");
      break;
    }
  }
  
  // Unlock the disc in the CD-ROM drive.
  pmrLockCDROM.PreventMediaRemoval = FALSE;
  DeviceIoControl(cdFile, IOCTL_CDROM_MEDIA_REMOVAL, &pmrLockCDROM, sizeof(pmrLockCDROM), NULL, 0, &ntemp, NULL);
  
  VirtualFree(buffer, 0, MEM_RELEASE);
  
  CloseHandle(cdFile);
  CloseHandle(imgFile);
  
  return 0;
}
