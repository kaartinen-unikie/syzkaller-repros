// WARNING in untrack_pfn (2)
// https://syzkaller.appspot.com/bug?id=ef98d8c91e682e920ceb7b5d47412f3847e33a1f
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/selinux/status\000", 16);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20ffc000, 0x1000, 0, 0x80012, r[0], 0);
  syscall(__NR_mremap, 0x20ffc000, 0x2000, 0x1000, 3, 0x20ff2000);
  return 0;
}