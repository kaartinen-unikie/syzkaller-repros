// KASAN: global-out-of-bounds Read in do_blockdev_direct_IO
// https://syzkaller.appspot.com/bug?id=83c1e71f3d034bd70e25d22f6fa3238f370750e9
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x20000040, "./bus\000", 6);
  syscall(__NR_creat, 0x20000040, 0);
  res = syscall(__NR_pipe, 0x20000380);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000380;
    r[1] = *(uint32_t*)0x20000384;
  }
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = 1;
  syscall(__NR_setsockopt, r[2], 6, 0x10000000013, 0x20000080, 4);
  *(uint32_t*)0x20788ffc = 1;
  syscall(__NR_setsockopt, r[2], 6, 0x14, 0x20788ffc, 0xfdf6);
  *(uint16_t*)0x20000000 = 2;
  *(uint16_t*)0x20000002 = htobe16(0);
  *(uint8_t*)0x20000004 = 0xac;
  *(uint8_t*)0x20000005 = 0x14;
  *(uint8_t*)0x20000006 = 0x14;
  *(uint8_t*)0x20000007 = 0xaa;
  syscall(__NR_connect, r[2], 0x20000000, 0x10);
  memcpy((void*)0x200000c0, "\000", 1);
  syscall(__NR_sendto, r[2], 0x200000c0, 0xffffffffffffff23, 0, 0, 0);
  syscall(__NR_splice, r[2], 0, r[1], 0, 0x10000011001, 0);
  memcpy((void*)0x20000600, "./bus\000", 6);
  res = syscall(__NR_open, 0x20000600, 0x4001, 0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_splice, r[0], 0, r[3], 0, 0xffffffff, 0);
  return 0;
}
