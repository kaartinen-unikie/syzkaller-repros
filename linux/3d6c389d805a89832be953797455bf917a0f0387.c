// KASAN: slab-out-of-bounds Read in soft_cursor
// https://syzkaller.appspot.com/bug?id=3d6c389d805a89832be953797455bf917a0f0387
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x200;
  *(uint16_t*)0x20000082 = 2;
  *(uint64_t*)0x20000088 = 0x20000400;
  memcpy(
      (void*)0x20000400,
      "\xaa\xb2\x1a\x2f\x3c\xc8\xe1\x79\xe7\x8c\xe3\x8e\x0e\x08\x3b\x82\xbb\x46"
      "\x21\xab\xd9\x86\xa7\x2c\xf9\x64\x87\x19\x96\xe0\x93\x43\x72\xce\xc9\xcf"
      "\x97\xbe\xda\x53\x3a\x80\x40\xaf\x41\x8a\x84\x06\x1d\xe1\x3b\x9f\x1d\x79"
      "\xd0\xd0\xb4\xa0\x44\x94\x03\x0e\x7d\xa5\xa9\x49\x8c\x4c\x86\xe9\xcb\xf8"
      "\xb5\xfb\x37\x71\xfa\xd6\x78\xe2\x12\x0a\xe1\x2a\x66\xa9\x7f\xe8\xca\x2b"
      "\x25\x40\xc2\x6f\xd1\xb1\x9b\x35\xaf\x34\x8b\x25\x72\x7c\x22\xd6\xfe\x94"
      "\xdc\xed\x8e\x88\xd5\xf5\xbb\xeb\x18\x3c\xeb\xbf\x84\x33\xf2\xa0\xcf\x2e"
      "\xb5\x21\xc5\x56\x43\x33\xa4\xd0\x0d\x5d\x4c\x5a\x1e\xd3\x8d\x20\xc5\x7d"
      "\x95\xde\x12\x13\xba\xd4\x85\x63\x5b\x84\x9d\xc1\xba\x2e\xe5\x9b\x11\x18"
      "\x6a\xe6\x98\x9e\xd6\x5d\xed\x54\x23\x7d\xe4\x89\xa7\x7c\x3c\xb3\x14\xd3"
      "\x7d\x79\x6b\x5e\xd8\x7c\x72\x67\x9b\x2f\x16\x2c\xfd\x7d\x78\xa2\xb5\x00"
      "\x20\x83\x58\xcf\x2c\xb8\x72\x2b\x75\xf3\x96\x9f\xe0\xc4\x7a\x9f\xf2\x07"
      "\xc9\x73\x6b\x1b\x61\xc1\x95\xad\xab\xd4\x4b\x9d\xf6\x98\x28\xcf\x69\x51"
      "\x3c\xbe\x1c\xf6\x08\xa0\xb0\x58\x23\x6d\x62\x41\x00\x33\xeb\xd5\xa4\x39"
      "\xf1\x47\xe2\xcb\xc1\x9b\x8b\x9e\xbd\x7c\x40\x1a\xc8\x26\xcd\x24\xb3\x27"
      "\xd9\xd7\x0b\xe2\xc0\xaa\x45\xef\x48\x35\x8c\x93\x25\xa3\x58\xcb\x5d\xf0"
      "\xaf\xc4\x49\x59\x35\xfb\xad\x66\x06\x47\x69\x7f\x22\xdc\x19\xf8\xcc\x40"
      "\x3e\x9b\xa0\xc4\x33\x74\xbc\xb1\xb0\xde\x3e\x26\x55\xb7\x08\xf3\x90\x0a"
      "\x0c\x7c\x45\x6c\x16\xa2\xff\xc9\x86\xb3\xc2\xf5\xf6\x73\x13\xb9\xa5\x2e"
      "\x84\x0e\x1c\x43\x74\x98\xe3\xbc\x10\x9b\x3a\xd5\xdc\x65\x15\xae\xc4\x27"
      "\x8a\xce\x18\x22\x1f\x6b\x73\xc7\x7a\xa9\x52\xe8\x89\x43\xd7\x49\x12\xcb"
      "\x53\x7d\x30\x2c\x30\xa0\xc3\xc4\x13\x98\x3f\x4a\x8b\x45\xd5\x55\x6c\x75"
      "\xec\x77\xfa\xb6\xe6\x60\x6f\x35\xcd\x0a\x77\x8a\x4d\xf5\x6c\x5c\x29\xc8"
      "\xe9\xb0\x77\x23\x6b\xb5\x78\x0d\xe7\x25\xa5\x06\xc0\x40\x29\x2f\xcb\xe1"
      "\xe4\xca\x7e\x81\x9f\x30\xed\x73\x0f\x8c\x7c\x97\xcd\x70\xaf\x4f\x74\x31"
      "\x3e\xfd\x94\x7c\xa7\x1a\x2a\x6f\xd3\xf6\x72\xed\x72\xc5\x2f\x39\x87\xa6"
      "\x43\x25\x03\xd0\xa5\x41\xe2\xf4\x7c\x43\x10\xe6\xd2\xb7\x47\xf8\x4c\xfa"
      "\x54\x75\x55\xb1\xee\x82\xb3\xd7\xaf\xa7\x42\xcb\x64\xad\x95\x5a\xc9\x7b"
      "\x62\xe6\x10\xda\xc7\x38\x58\xbb\x1f\x3b\xe7\x69\x7f\xae\x2a\x63\xde\x11"
      "\x1e\x9b\xd3\x2a\x10\x44\x64\x9a\x9a\x83\x30\x46\xe5\xca\xb5\xc6\x11\x36"
      "\xb9\x13\x66\xec\x8b\xeb\xba\xe4\x53\xcd\xd4\xcf\x63\x36\x68\xbc\xaa\x38"
      "\x55\x76\x99\x75\xc0\xbf\xd8\xf6\x1b\x01\x6a\x33\x96\xde\xbb\xe2\x92\x2e"
      "\xd4\xdd\xb5\xc1\x2d\x50\xb6\xe4\x65\xef\xa3\x3f\x08\xcb\x8d\xcd\xf8\x0c"
      "\xb3\x87\x00\x90\xc8\x63\xec\xb8\x4a\x7c\xc4\x67\x5d\x6c\xa7\x11\xa3\x50"
      "\x81\x86\xa8\x6c\x9b\xff\xd1\x82\xa1\x1f\x3f\xcb\xe2\x5c\x81\xb6\xeb\xbc"
      "\xdb\xbd\x8c\x12\xe8\xaf\x02\xb2\xae\xe6\x96\x44\x79\xe9\xa7\x64\x96\x12"
      "\xc9\x52\x7a\x95\x07\xb5\x32\x4e\x82\xfe\x64\x30\x12\x10\x1b\x3f\xe5\x1a"
      "\xe9\x6f\x83\x69\x1d\xa3\xff\x72\xad\xf5\x1f\x98\x02\x2d\x85\x84\x66\x67"
      "\x33\xa2\xab\xfb\xbb\x59\x4d\xd1\x5f\xf8\xf6\xb2\x50\xca\x46\x82\xaf\xfb"
      "\xe1\x66\x17\x93\xff\x0a\xa6\x11\xac\x10\x4a\x0d\x21\x59\x85\x5b\xf9\x85"
      "\x44\x30\x1d\x57\x3a\x7f\x3b\x2c\x4e\x1d\x10\xa3\x24\x27\x0f\x38\x1c\x72"
      "\x6c\x2b\x76\x50\x82\x50\x2a\xc3\x15\x6f\x14\x41\xc1\xa6\xc4\xe5\xca\x01"
      "\xcc\xd1\x5d\xaf\xf9\xf2\x73\xa8\xb0\x99\x72\xc1\xc0\xac\xb1\xfc\x4b\x6c"
      "\xb8\xb7\xcf\x86\x63\xf5\x63\x6e\xb4\x94\x6f\x34\x0e\x65\xe2\xea\x0d\x03"
      "\xdf\x51\x79\xb9\xca\xe9\x56\x0e\xc2\xdd\x3d\x50\x84\x51\xca\x3e\x99\xe2"
      "\x7e\x83\x61\x2f\x06\xfd\x8c\x6b\xaf\x34\x83\x46\xd4\x9e\x43\xcf\x51\xca"
      "\x7e\x40\xe5\x00\x4a\xe4\x12\xb7\x57\x00\xc5\x84\x52\xa7\xf2\xb2\xd5\xc3"
      "\x83\x42\x87\xf7\x4b\x18\x03\x2d\x83\xa1\xb9\x0a\x29\x95\x86\xeb\x96\x4f"
      "\x68\x84\xab\x71\x03\x40\xab\x90\x89\xba\xdd\xfe\xa3\x9e\x3e\x69\x4e\x68"
      "\xbb\xe8\x3f\xca\xbc\x34\x00\xc0\xa3\x29\x4e\xff\x10\xee\x90\x2b\xb5\xb6"
      "\xb2\x98\x23\xe1\x05\xf5\x91\x0a\x8b\x1c\xab\x7b\x26\xd5\xe0\x19\xae\xda"
      "\xd8\x53\xc3\x7a\x31\xb8\xbc\x5d\x6d\x3b\x41\x12\x08\x71\xac\x58\x7c\x31"
      "\xe2\xe4\xe3\x70\x22\x45\x5f\x13\x17\x13\x2b\x52\x24\xa4\xa7\x1e\x70\xf9"
      "\x1d\x4a\x24\x43\x3d\xe6\xd1\x98\xbe\x63\x7f\x23\x1f\x80\x80\x69\xed\x66"
      "\xee\x23\xc3\x5c\x10\x69\x7b\x89\x4d\x7e\x5a\xbe\xdc\x9d\x8b\x52\x85\xb6"
      "\xfc\xbc\x4a\x57\xc8\x5c\xa2\xc6\xc5\x11\x81\xba\x92\xfc\x1a\x1e\xde\xf5"
      "\x98\x8b\xa8\x3a\x35\xed\xda\xd0\x45\x97\x20\x3f\x38\xfd\xf1\x38",
      1024);
  syscall(__NR_ioctl, r[0], 0x4b6cul, 0x20000080ul);
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000000 = 0;
  *(uint16_t*)0x20000002 = 0;
  *(uint16_t*)0x20000004 = 0;
  *(uint16_t*)0x20000006 = 0x10;
  *(uint8_t*)0x20000008 = 0;
  memcpy((void*)0x20000009, "\xbb\xc1\xac\x0f\x87\x59\x92\x13", 8);
  syscall(__NR_ioctl, r[1], 0x560aul, 0x20000000ul);
  return 0;
}
