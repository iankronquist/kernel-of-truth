#pragma once

#include <truth/file.h>

enum drive_status {
    Drive_Buffer_Error = 0x1,
    Drive_Buffer_Read  = 0x2,
    Drive_Buffer_Dirty = 0x4,
};

#define Drive_Sector_Size 512
#define Drive_Buffer_Size 0x1000

struct drive_buffer {
  enum drive_status flags;
  unsigned int dev;
  unsigned int block_number;
  struct lock lock;
  struct drive_buffer *next;
  uint8_t *data;
};
