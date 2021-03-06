// WARNING in task_participate_group_stop
// https://syzkaller.appspot.com/bug?id=e2fe06b813f900f9c187da84bfd42eab23d0b84c
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <stdint.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_inotify_init1
#define __NR_inotify_init1 332
#endif
#ifndef __NR_getsockopt
#define __NR_getsockopt 365
#endif
#ifndef __NR_fcntl
#define __NR_fcntl 55
#endif
#ifndef __NR_ptrace
#define __NR_ptrace 26
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[3];
uint64_t procid;
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_inotify_init1, 0x80800);
  *(uint32_t*)0x20000000 = 0xc;
  if (syscall(__NR_getsockopt, -1, 1, 0x11, 0x20000ff4, 0x20000000) != -1)
    r[1] = *(uint32_t*)0x20000ff4;
  syscall(__NR_fcntl, r[0], 8, r[1]);
  if (syscall(__NR_fcntl, r[0], 0x10, 0x206dbff8) != -1)
    r[2] = *(uint32_t*)0x206dbffc;
  syscall(__NR_ptrace, 0x10, r[2]);
  syscall(__NR_ptrace, 0x4200, r[2], 0x440207, 0x100030);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
