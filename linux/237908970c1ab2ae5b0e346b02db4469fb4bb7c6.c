// KASAN: use-after-free Read in bitmap_ipmac_ext_cleanup
// https://syzkaller.appspot.com/bug?id=237908970c1ab2ae5b0e346b02db4469fb4bb7c6
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0x200002c0;
  *(uint32_t*)0x200002c0 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x70;
  *(uint8_t*)0x20000244 = 2;
  *(uint8_t*)0x20000245 = 6;
  *(uint16_t*)0x20000246 = 1;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = htobe16(0);
  *(uint16_t*)0x20000254 = 0x12;
  *(uint16_t*)0x20000256 = 3;
  memcpy((void*)0x20000258, "bitmap:ip,mac\000", 14);
  *(uint16_t*)0x20000268 = 9;
  *(uint16_t*)0x2000026a = 2;
  memcpy((void*)0x2000026c, "syz0\000", 5);
  *(uint16_t*)0x20000274 = 0x24;
  STORE_BY_BITMASK(uint16_t, , 0x20000276, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000277, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000277, 1, 7, 1);
  *(uint16_t*)0x20000278 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000027a, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000027b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000027b, 1, 7, 1);
  *(uint16_t*)0x2000027c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000027e, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000027f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000027f, 0, 7, 1);
  *(uint32_t*)0x20000280 = htobe32(0);
  *(uint16_t*)0x20000284 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x20000286, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000287, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000287, 1, 7, 1);
  *(uint16_t*)0x20000288 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000028a, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000028b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000028b, 0, 7, 1);
  *(uint32_t*)0x2000028c = htobe32(0);
  *(uint16_t*)0x20000290 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000292, 8, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000293, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000293, 0, 7, 1);
  *(uint32_t*)0x20000294 = htobe32(0x10);
  *(uint16_t*)0x20000298 = 5;
  *(uint16_t*)0x2000029a = 1;
  *(uint8_t*)0x2000029c = 7;
  *(uint16_t*)0x200002a0 = 5;
  *(uint16_t*)0x200002a2 = 4;
  *(uint8_t*)0x200002a4 = 0;
  *(uint16_t*)0x200002a8 = 5;
  *(uint16_t*)0x200002aa = 5;
  *(uint8_t*)0x200002ac = 2;
  *(uint32_t*)0x200002c4 = 0x70;
  *(uint32_t*)0x2000030c = 1;
  *(uint32_t*)0x20000310 = 0;
  *(uint32_t*)0x20000314 = 0;
  *(uint32_t*)0x20000318 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000300, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
