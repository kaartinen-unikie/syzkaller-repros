// KMSAN: uninit-value in __se_sys_waitid (2)
// https://syzkaller.appspot.com/bug?id=de779e4996d219b981c2afa66cd6e9c68e6d4654
// status:invalid
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

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  syscall(__NR_waitid, 1, 0, 0x20000580, 0x40000000, 0);
  return 0;
}
