// general protection fault in get_unique_tuple
// https://syzkaller.appspot.com/bug?id=18ea4982789de0c2d4e215eaabaf89d727449a34
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000240;
  *(uint64_t*)0x20000240 = 0x20000000;
  *(uint32_t*)0x20000000 = 0x78;
  *(uint8_t*)0x20000004 = 0;
  *(uint8_t*)0x20000005 = 1;
  *(uint16_t*)0x20000006 = 0x519;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x53;
  *(uint8_t*)0x20000011 = 0;
  *(uint16_t*)0x20000012 = htobe16(0);
  *(uint16_t*)0x20000014 = 0x24;
  *(uint16_t*)0x20000016 = 1;
  *(uint16_t*)0x20000018 = 0x14;
  *(uint16_t*)0x2000001a = 1;
  *(uint16_t*)0x2000001c = 8;
  *(uint16_t*)0x2000001e = 1;
  *(uint32_t*)0x20000020 = htobe32(-1);
  *(uint16_t*)0x20000024 = 8;
  *(uint16_t*)0x20000026 = 2;
  *(uint8_t*)0x20000028 = 0xac;
  *(uint8_t*)0x20000029 = 0x1e;
  *(uint8_t*)0x2000002a = 0;
  *(uint8_t*)0x2000002b = 1;
  *(uint16_t*)0x2000002c = 0xc;
  *(uint16_t*)0x2000002e = 2;
  *(uint16_t*)0x20000030 = 5;
  *(uint16_t*)0x20000032 = 1;
  *(uint8_t*)0x20000034 = 0;
  *(uint16_t*)0x20000038 = 0x24;
  *(uint16_t*)0x2000003a = 2;
  *(uint16_t*)0x2000003c = 0xc;
  *(uint16_t*)0x2000003e = 2;
  *(uint16_t*)0x20000040 = 5;
  *(uint16_t*)0x20000042 = 1;
  *(uint8_t*)0x20000044 = 0;
  *(uint16_t*)0x20000048 = 0x14;
  *(uint16_t*)0x2000004a = 1;
  *(uint16_t*)0x2000004c = 8;
  *(uint16_t*)0x2000004e = 1;
  *(uint32_t*)0x20000050 = htobe32(0);
  *(uint16_t*)0x20000054 = 8;
  *(uint16_t*)0x20000056 = 2;
  *(uint32_t*)0x20000058 = htobe32(0xe0000002);
  *(uint16_t*)0x2000005c = 8;
  *(uint16_t*)0x2000005e = 7;
  *(uint32_t*)0x20000060 = htobe32(0);
  *(uint16_t*)0x20000064 = 0x14;
  *(uint16_t*)0x20000066 = 6;
  *(uint16_t*)0x20000068 = 8;
  *(uint16_t*)0x2000006a = 2;
  *(uint32_t*)0x2000006c = htobe32(0x7f000001);
  *(uint16_t*)0x20000070 = 8;
  *(uint16_t*)0x20000072 = 1;
  *(uint32_t*)0x20000074 = htobe32(0x7f000001);
  *(uint64_t*)0x20000248 = 0x78;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000280ul, 0ul);
  return 0;
}
