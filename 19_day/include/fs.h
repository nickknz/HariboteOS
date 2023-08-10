#ifndef _FS_H_
#define _FS_H_

#define ADR_DISKIMG 0x00100000

struct FileInfo {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clsutno;
  unsigned int size;
};

#endif // _FS_H_