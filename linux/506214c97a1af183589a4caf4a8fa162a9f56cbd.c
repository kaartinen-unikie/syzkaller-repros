// general protection fault in tcp_splice_read
// https://syzkaller.appspot.com/bug?id=506214c97a1af183589a4caf4a8fa162a9f56cbd
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
  res = syscall(__NR_pipe, 0x20000140);
  if (res != -1)
    r[0] = *(uint32_t*)0x20000144;
  res = syscall(__NR_socket, 2, 2, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close, r[1]);
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[2] = res;
  *(uint16_t*)0x20000600 = 2;
  *(uint16_t*)0x20000602 = htobe16(0x4e23);
  *(uint32_t*)0x20000604 = htobe32(0xe0000002);
  syscall(__NR_bind, r[2], 0x20000600, 0x10);
  *(uint16_t*)0x200001c0 = 2;
  *(uint16_t*)0x200001c2 = htobe16(0x4e23);
  *(uint32_t*)0x200001c4 = htobe32(0);
  syscall(__NR_connect, r[2], 0x200001c0, 0x10);
  syscall(__NR_splice, r[1], 0, r[0], 0, 0x10009, 2);
  return 0;
}
