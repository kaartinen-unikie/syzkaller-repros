// panic: runtime error: invalid memory address or nil pointer dereference (2)
// https://syzkaller.appspot.com/bug?id=b40f04a96bb57b8fa5b183ff62a19ca1d34ac2df
// status:fixed
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x2000fffa, "./bus", 6);
  res = syscall(__NR_open, 0x2000fffa, 0x141042, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20001000, 0xa000, 0x800002, 0x11, r[0], 0);
  res = syscall(__NR_pipe, 0x20000400);
  if (res != -1) {
    r[1] = *(uint32_t*)0x20000400;
    r[2] = *(uint32_t*)0x20000404;
  }
  syscall(__NR_write, r[2], 0x200001c0, 0xfffffef3);
  syscall(__NR_read, r[1], 0x20000200, 0x50c7e3e3);
  return 0;
}
