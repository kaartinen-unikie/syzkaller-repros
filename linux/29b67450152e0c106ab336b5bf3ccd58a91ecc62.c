// KASAN: use-after-free Write in preempt_notifier_register (2)
// https://syzkaller.appspot.com/bug?id=29b67450152e0c106ab336b5bf3ccd58a91ecc62
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000040, "/dev/kvm", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0x20000000000004);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, r[1], 0xae41, 0);
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x20000000);
  return 0;
}