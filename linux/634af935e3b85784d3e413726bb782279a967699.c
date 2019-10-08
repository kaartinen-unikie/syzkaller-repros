// general protection fault in perf_iterate_sb
// https://syzkaller.appspot.com/bug?id=634af935e3b85784d3e413726bb782279a967699
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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

unsigned long long procid;

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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
      reset_test();
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
  long res = 0;
  *(uint32_t*)0x20000200 = 1;
  *(uint32_t*)0x20000204 = 0x70;
  *(uint8_t*)0x20000208 = 0;
  *(uint8_t*)0x20000209 = 0;
  *(uint8_t*)0x2000020a = 0;
  *(uint8_t*)0x2000020b = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 3, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 29, 35);
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint16_t*)0x2000026c = 0;
  *(uint16_t*)0x2000026e = 0;
  res = syscall(__NR_perf_event_open, 0x20000200, -1, 0, -1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 5;
  *(uint32_t*)0x20000004 = 0x70;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint64_t*)0x20000010 = 0;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000028, 0, 29, 35);
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 2;
  *(uint64_t*)0x20000038 = 0;
  *(uint64_t*)0x20000040 = 1;
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint32_t*)0x20000068 = 0;
  *(uint16_t*)0x2000006c = 0;
  *(uint16_t*)0x2000006e = 0;
  syscall(__NR_perf_event_open, 0x20000000, -1, 0, r[0], 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}