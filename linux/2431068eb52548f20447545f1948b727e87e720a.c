// lost connection to test machine (4)
// https://syzkaller.appspot.com/bug?id=2431068eb52548f20447545f1948b727e87e720a
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

uint64_t r[1] = {0x0};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  syscall(__NR_mmap, 0x20000000, 0xb36000, 0, 0x80000008031, -1, 0);
  syscall(__NR_clone, 0x2102001fff, 0, 0x9999999999999999, 0x20000000, -1);
  syscall(__NR_socket, 0, 0, 0);
  res = syscall(__NR_getpid);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mprotect, 0x20000000, 0x800000, 4);
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  syscall(__NR_rt_tgsigqueueinfo, r[0], r[0], 0x16, 0x20000180);
  syscall(__NR_ptrace, 0x10, r[0]);
  syscall(__NR_ptrace, 4, r[0], 0x200000c0, 0);
  return 0;
}
