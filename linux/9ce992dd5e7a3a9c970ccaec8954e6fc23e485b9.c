// KASAN: slab-out-of-bounds Read in bitmap_ipmac_gc
// https://syzkaller.appspot.com/bug?id=9ce992dd5e7a3a9c970ccaec8954e6fc23e485b9
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000380;
  *(uint32_t*)0x20000380 = 0x70;
  *(uint8_t*)0x20000384 = 2;
  *(uint8_t*)0x20000385 = 6;
  *(uint16_t*)0x20000386 = 1;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint8_t*)0x20000390 = 0;
  *(uint8_t*)0x20000391 = 0;
  *(uint16_t*)0x20000392 = htobe16(0);
  *(uint16_t*)0x20000394 = 0x12;
  *(uint16_t*)0x20000396 = 3;
  memcpy((void*)0x20000398, "bitmap:ip,mac\000", 14);
  *(uint16_t*)0x200003a8 = 9;
  *(uint16_t*)0x200003aa = 2;
  memcpy((void*)0x200003ac, "syz0\000", 5);
  *(uint16_t*)0x200003b4 = 0x24;
  STORE_BY_BITMASK(uint16_t, , 0x200003b6, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003b7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003b7, 1, 7, 1);
  *(uint16_t*)0x200003b8 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200003ba, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003bb, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003bb, 1, 7, 1);
  *(uint16_t*)0x200003bc = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200003be, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003bf, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003bf, 0, 7, 1);
  *(uint32_t*)0x200003c0 = htobe32(0);
  *(uint16_t*)0x200003c4 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200003c6, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003c7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003c7, 1, 7, 1);
  *(uint16_t*)0x200003c8 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200003ca, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003cb, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003cb, 0, 7, 1);
  *(uint32_t*)0x200003cc = htobe32(0);
  *(uint16_t*)0x200003d0 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200003d2, 6, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200003d3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200003d3, 0, 7, 1);
  *(uint32_t*)0x200003d4 = htobe32(8);
  *(uint16_t*)0x200003d8 = 5;
  *(uint16_t*)0x200003da = 1;
  *(uint8_t*)0x200003dc = 7;
  *(uint16_t*)0x200003e0 = 5;
  *(uint16_t*)0x200003e2 = 4;
  *(uint8_t*)0x200003e4 = 0;
  *(uint16_t*)0x200003e8 = 5;
  *(uint16_t*)0x200003ea = 5;
  *(uint8_t*)0x200003ec = 2;
  *(uint64_t*)0x200002c8 = 0x70;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  loop();
  return 0;
}
