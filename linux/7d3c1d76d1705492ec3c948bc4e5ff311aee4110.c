// kernel BUG at mm/slab.c:LINE! (2)
// https://syzkaller.appspot.com/bug?id=7d3c1d76d1705492ec3c948bc4e5ff311aee4110
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x20000200 = 0xc;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000000;
  memcpy((void*)0x20000000,
         "\xb7\x02\x00\x00\x02\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07"
         "\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x00\x00\x00\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x00"
         "\x00\x00\x00\x85\x00\x00\x00\x2b\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         112);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = r[0];
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0x44;
  *(uint32_t*)0x2000008c = 0x55;
  *(uint64_t*)0x20000090 = 0x20000100;
  memcpy((void*)0x20000100,
         "\xe7\xc6\x76\xd0\xc8\x3f\xa7\x35\xba\x46\xff\x92\x7e\xe4\xa9\x6c\x6e"
         "\xa6\x34\x21\x23\xbd\xa4\xd6\x8a\xd1\xb8\x4d\xb5\xcf\xb9\x69\xe7\x04"
         "\x5c\x7f\xfd\xab\x19\x1e\x55\xe9\xf7\x92\xd4\x8f\xeb\x9c\x55\x1a\x83"
         "\xc6\x04\x85\x65\x47\x5a\xc9\x2f\xb6\xf0\xb6\x97\xb7\x09\x4c\x3c\x5b",
         68);
  *(uint64_t*)0x20000098 = 0x20000280;
  *(uint32_t*)0x200000a0 = 0x401;
  *(uint32_t*)0x200000a4 = 0;
  syscall(__NR_bpf, 0xa, 0x20000080, 0x28);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}