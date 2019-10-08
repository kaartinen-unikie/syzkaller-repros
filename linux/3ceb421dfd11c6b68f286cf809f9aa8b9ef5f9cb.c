// possible deadlock in ftrace_profile_set_filter
// https://syzkaller.appspot.com/bug?id=3ceb421dfd11c6b68f286cf809f9aa8b9ef5f9cb
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  *(uint32_t*)0x20000180 = 2;
  *(uint32_t*)0x20000184 = 0x70;
  *(uint8_t*)0x20000188 = 0xcd;
  *(uint8_t*)0x20000189 = 0;
  *(uint8_t*)0x2000018a = 0;
  *(uint8_t*)0x2000018b = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint64_t*)0x20000190 = 0;
  *(uint64_t*)0x20000198 = 0;
  *(uint64_t*)0x200001a0 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0x10000, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0x800, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0x40, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0x400000000, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 1, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0x10, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200001a8, 0, 29, 35);
  *(uint32_t*)0x200001b0 = 1;
  *(uint32_t*)0x200001b4 = -1;
  *(uint64_t*)0x200001b8 = 0;
  *(uint64_t*)0x200001c0 = 2;
  *(uint64_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 2;
  *(uint64_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e8 = 0;
  *(uint16_t*)0x200001ec = 0;
  *(uint16_t*)0x200001ee = 0;
  res = syscall(__NR_perf_event_open, 0x20000180, -1, 0, -1, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000,
         "cpu&5\n\n\n\000\000\001\000\000\003\234\323\0165\314\240\035="
         "\v\n\034`fo\215\314m\\v\375.\236\tbk1\336\352\033\r;\201\204\207-"
         "X\266,\305\264\"7&\265yt\202\373\035\203\346\370.-m\n\363\261(\351`"
         "D\001i\\\215l\206lh\250\374\200\336,Kt\364\032\305]Y;\t\006\276*\252&"
         "\275\026xQ\206\363\326\032\375\320\004\'y\233|"
         "\344\267\bE\355\227\200s\031W\267[\360%>MM\365\230\276^=q!"
         "\246\017p\0012\000\273\276\235r5\257ep\020R\v&\257\250$"
         "\1777V\355LJ4\317\220@\325T\n\312\302\206_"
         "\301\316\215\355bS\215\351t\202\3641zwr\346o\210\345\343\347Gcx\300"
         "\221I\215\375\344\273s\277\n\000K\236\345[\240\n\017\004\246\260sE)"
         "\212\320R\303\301,b "
         "\034#IRz6\376J~\332\327_"
         "\276\037\345\206\261x\000\000\000\000\000\000\000\000\223\253\005",
         265);
  syscall(__NR_ioctl, r[0], 0x40082406, 0x20000000);
  return 0;
}