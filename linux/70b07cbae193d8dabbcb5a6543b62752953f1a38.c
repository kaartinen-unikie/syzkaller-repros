// INFO: rcu detected stall in ip_set_udel
// https://syzkaller.appspot.com/bug?id=70b07cbae193d8dabbcb5a6543b62752953f1a38
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000440;
  *(uint32_t*)0x20000440 = 0x44;
  *(uint8_t*)0x20000444 = 2;
  *(uint8_t*)0x20000445 = 6;
  *(uint16_t*)0x20000446 = 1;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint8_t*)0x20000450 = 0;
  *(uint8_t*)0x20000451 = 3;
  *(uint16_t*)0x20000452 = htobe16(0);
  *(uint16_t*)0x20000454 = 9;
  *(uint16_t*)0x20000456 = 2;
  memcpy((void*)0x20000458, "syz2\000", 5);
  *(uint16_t*)0x20000460 = 0xc;
  *(uint16_t*)0x20000462 = 3;
  memcpy((void*)0x20000464, "hash:ip\000", 8);
  *(uint16_t*)0x2000046c = 5;
  *(uint16_t*)0x2000046e = 1;
  *(uint8_t*)0x20000470 = 7;
  *(uint16_t*)0x20000474 = 5;
  *(uint16_t*)0x20000476 = 4;
  *(uint8_t*)0x20000478 = 0;
  *(uint16_t*)0x2000047c = 5;
  *(uint16_t*)0x2000047e = 5;
  *(uint8_t*)0x20000480 = 2;
  *(uint64_t*)0x200002c8 = 0x44;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0x200000000000000;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x40;
  *(uint8_t*)0x20000144 = 0xa;
  *(uint8_t*)0x20000145 = 6;
  *(uint16_t*)0x20000146 = 0x401;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint16_t*)0x20000154 = 9;
  *(uint16_t*)0x20000156 = 2;
  memcpy((void*)0x20000158, "syz2\000", 5);
  *(uint16_t*)0x20000160 = 5;
  *(uint16_t*)0x20000162 = 1;
  *(uint8_t*)0x20000164 = 7;
  *(uint16_t*)0x20000168 = 0x18;
  STORE_BY_BITMASK(uint16_t, , 0x2000016a, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000016b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000016b, 1, 7, 1);
  *(uint16_t*)0x2000016c = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000016e, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000016f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000016f, 1, 7, 1);
  *(uint16_t*)0x20000170 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000172, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000173, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000173, 0, 7, 1);
  *(uint8_t*)0x20000174 = 0xac;
  *(uint8_t*)0x20000175 = 0x14;
  *(uint8_t*)0x20000176 = 0x14;
  *(uint8_t*)0x20000177 = 0xbb;
  *(uint16_t*)0x20000178 = 5;
  *(uint16_t*)0x2000017a = 3;
  *(uint8_t*)0x2000017c = 2;
  *(uint64_t*)0x20000008 = 0x40;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000080ul, 0ul);
  return 0;
}
