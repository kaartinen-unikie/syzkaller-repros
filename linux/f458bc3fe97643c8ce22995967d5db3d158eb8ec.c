// Invalid request partialResult in sendfile
// https://syzkaller.appspot.com/bug?id=f458bc3fe97643c8ce22995967d5db3d158eb8ec
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    fail("sandbox fork failed");
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  doexit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000700, "./bus", 6);
  res = syscall(__NR_creat, 0x20000700, 0);
  if (res != -1)
    r[0] = res;
  memcpy(
      (void*)0x20002100,
      "\xc5\x14\x3f\x9e\x4a\x93\xac\x45\xcd\xa3\xe5\x5d\xe8\x28\x1b\xb6\x20\x30"
      "\x38\x5d\x5b\xb9\xde\xd6\xa3\x7b\xe3\xe4\x59\xed\x85\x09\xb4\x80\x05\x89"
      "\xc7\x98\xc1\x59\x08\xdd\x3d\x2f\x4a\xf1\x84\xe9\x63\xe9\x19\x96\x14\x71"
      "\x8c\xac\xad\x8e\x98\xa7\x0c\x9a\xe7\x69\x03\x45\xd7\xb7\x1a\x36\x33\x1e"
      "\x6e\x49\x30\x42\x94\x9b\x24\x0a\xa9\x8b\x67\x03\x9e\xe8\x23\x0d\x29\x98"
      "\xa1\xb6\xa0\x49\x9b\x92\x9f\xa0\x57\x72\xab\x52\xdb\x8b\xb8\xc4\x39\xcc"
      "\x56\x85\x6e\x85\x08\x53\x21\xb5\x08\xd5\xe0\x1c\x7b\x3a\xc7\x6b\x3f\xb5"
      "\x95\x21\xe9\x60\xe0\x43\x31\xee\xc5\xfe\x37\xd3\x5c\x55\xb5\x93\x57\xae"
      "\x9a\x51\x64\x01\x76\x92\xc9\xe7\x1f\x56\xd0\x34\x3b\xc4\x13\xaa\x82\xb9"
      "\xb6\xbf\x37\x09\xb8\x29\x50\x28\x0c\x21\xb3\xf8\x6e\x67\xe7\x5c\x7b\xad"
      "\xe9\x77\x2e\xc6\xb2\x3e\xca\x78\xc3\xa1\x8f\x13\x0f\x47\xf2\x0c\x39\xfd"
      "\x3e\x36\xf6\x39\xcc\x4b\x76\x0b\x0c\x65\x45\x33\x6f\x34\x8b\x79\x55\x7e"
      "\xba\xba\x3c\x6e\x2d\xa7\x94\xd3\xb6\x8f\x17\xeb\xb1\x78\x72\x38\x32\x79"
      "\x59\xc8\xa2\x12\x0d\x4c\xe1\x07\x8e\x71\x47\x4e\x93\xc6\x9c\xca\xde\xf1"
      "\x02\x9f\x62\xac\xe6\xe2\x8b\x07\x36\x70\x5b\xd8\xa2\xa6\x14\xbb\x26\x4e"
      "\x1b\xc5\x15\x67\x17\x8c\xb0\x1d\xb2\x02\x3d\x76\xdd\xbe\x5b\xc5\x58\xc8"
      "\x03\xf4\x9c\x4f\x4e\xc2\x32\x2b\xe2\x06\x65\x6e\x3c\x59\xf9\xe0\xe4\xad"
      "\x9b\xf1\xe7\x3d\x73\xb9\x76\xe9\x20\x5e\x86\xb2\xa4\x75\x78\x78\x4c\x2e"
      "\x79\x7a\x8a\x54\xe5\xad\xcb\x64\xa0\x6a\x55\x24\x7e\x91\x7a\x26\xd1\x0e"
      "\xb3\x44\xb4\xde\xde\x29\x4c\xbc\xee\x36\x74\xa4\xda\xfe\xed\x6d\x48\x09"
      "\xef\xd4\xf9\x11\xbd\xc3\xb9\x14\x14\x78\x28\x93\xd6\x46\x91\xd8\x7d\x29"
      "\xf5\x9b\x3b\x53\xe2\xc7\x39\x10\xa1\xb1\xbc\x8a\x7a\x7a\xad\x7c\x5c\xda"
      "\x52\x47\xa4\x2c\x57\x48\x88\x08\xc8\x8b\x2e\x92\xe2\xd5\xe9\x09\xaa\x1f"
      "\x09\x06\xeb\xe9\x6c\x80\xc0\xa0\x59\x40\x14\xf0\x40\xa0\x59\xab\x4f\xb2"
      "\x5c\x21\x99\xe9\x9e\xad\xe5\xf1\xda\xe1\x97\x4b\x98\x7a\xdd\x16\x5c\xe2"
      "\x0f\xff\x5b\x9e\xaa\x47\xd4\x90\x90\x60\x8f\xcd\x28\xe1\x7e\x6b\x27\xe2"
      "\x67\xf8\x0a\x97\xa7\xf3\x76\xe5\x25\xc1\x4b\x6f\x34\x5f\x4e\xb8\x35\x8e"
      "\xca\x17\xcd\x06\x59\x10\xe6\x9d\x5e\x04\x63\x4f\x3e\xe7\x7a\x93\xc1\xdb"
      "\x41\x46\x41\x2d\x9e\x07\xa5\xc8\xcd\x8f\xe3\xc9\xac\x68\xe6\x83\x63\x00"
      "\x8d\xf0\xb9\x69\x17\x90\xe6\x1f\x3a\x70\xc4\x5b\x7d\x6a\x45\x29\xd9\x8b"
      "\xa1\xc4\xcf\x60\x57\x64\xf9\x5c\xa5\xa0\xdf\xbc\xc3\x6f\xb5\xa0\x5c\xf2"
      "\xe6\xbe\xc7\xd3\x8a\xa9\x5b\xce\x47\x52\x60\xc9\x1d\xe3\x41\x59\x99\xec"
      "\x17\x44\x00\xfb\x45\x43\xc2\xca\x63\xdf\x5e\x90\x40\x39\x08\xf3\xca\xe0"
      "\xe4\xb5\x97\x5f\x20\x2d\xc5\x24\x2b\x92\x47\xcd\x4c\x1b\x3f\x29\x86\xb0"
      "\xe7\xc3\x4c\x4b\x47\xe2\x85\xa4\x3e\x8b\x5b\x39\xca\x20\xe9\xa8\xa2\xd3"
      "\xca\x31\x32\x0a\x8b\x1a\x42\xf5\x21\x14\xce\xf3\x4f\xcc\x0f\xb9\x2b\xea"
      "\xc6\x02\x33\xa9\x1b\x40\xdc\xa9\x60\xfb\x6a\xaa\x8f\xfa\xba\xc4\x6c\x31"
      "\x66\xc0\xc3\x6c\xdd\xb8\x9d\xe3\xf5\x35\x96\x60\x32\x3b\x2d\xca\x3f\xec"
      "\x4e\x82\xd6\x23\x3d\x1e\xec\x0b\x69\xe3\xad\x83\xd9\xb6\xa7\xe0\xea\xf4"
      "\x3e\xee\x53\xb3\x93\x44\x3b\x7a\x38\x9d\xb6\x13\xad\xed\xd1\x07\x00\xe6"
      "\x6a\x7d\x97\x17\xd5\x3b\xba\x26\x82\x9c\xfb\xcf\x16\x24\xef\x11\x42\x5f"
      "\xae\x58\xd1\x2d\xf6\x41\x6a\x03\x49\x4b\x79\x7c\x70\xe6\xc1\x99\x61\xc4"
      "\x4a\xff\x76\xea\x6d\x9d\x3a\x95\xd4\x79\x48\xe0\xbd\x1a\x0f\x76\xf1\x32"
      "\xbd\x11\x04\x4d\x67\xf4\x25\x0d\x6f\x8c\x24\x48\x3d\xcc\x41\xda\x2e\x99"
      "\xc2\x2d\x78\x50\x92\x49\x7c\x27\xd7\xd8\x2b\x73\x6d\x03\x91\x74\xa3\x11"
      "\x74\x50\x97\xae\x29\x14\x94\xc7\x98\x84\xd6\xfa\x1e\x9a\x19\x2c\x16\x01"
      "\x48\xf1\xc5\x9c\x4e\xdf\xbe\x3b\x21\xce\x4e\xa6\x7b\xf3\xa5\xb5\x48\x49"
      "\xd8\x0a\x5f\x95\x42\xd0\x60\x30\xa2\xe6\xee\xd5\x56\xe8\x1b\x26\xf2\x38"
      "\xc2\xd3\xda\xe0\x51\xe0\x63\x93\x62\x83\x57\xb6\x36\xcd\x9a\xc6\x89\xc2"
      "\x25\xa0\xe0\x6a\x97\x54\x5f\x2d\x66\x70\x37\x6a\xd5\xff\xad\x38\x97\x79"
      "\xc7\x73\x3c\xd6\x05\xc1\x69\xe4\x36\x09\x82\x72\xa1\xd4\xe7\x61\xb9\x5c"
      "\xc0\xb6\x55\x21\xad\x2a\xe3\x24\x1a\x4e\x9f\x72\x37\x96\x06\x2b\x19\x91"
      "\x5c\x49\x18\x0a\xc6\x43\x4a\xa5\xe7\x19\x0c\xf4\x8c\xa9\x3d\xb8\x4a\xc7"
      "\x18\x1d\xda\x78\xb2\xed\x9c\x58\xd3\xc7\x64\xa4\xa3\x09\x52\x67\x7c\x47"
      "\x69\x8a\xcd\x53\xb6\xc7\xab\xaf\xe9\x40\xb3\xcf\x3a\xf6\x0b\x2f\x54\xa2"
      "\x4f\xb5\x68\xa2\xdf\x76\x4e\x17\xba\x51\x46\x5a\xca\x67\x76\xe3\xff\xc0"
      "\xd9\x17\x54\x0c\xf3\x6a\x82\x82\x5c\xcf\x2d\x1a\x21\x64\xa3\x40\x8a\xdb"
      "\xae\x24\x75\x36\xa9\x3e\x94\x41\x23\xac\xa4\x02\x57\x03\xae\x23\xd6\x24"
      "\x97\xf3\x9b\x99\x00\x6f\x03\x24\xcb\x6f\x34\x48\x98\x6a\xeb\x22\x03\x29"
      "\x53\x8d\x08\x1d\xba\x2a\xfb\x92\xfe\xbb\x93\x6b\x77\x9a\x38\x67\x39\xf7"
      "\x99\x12\xe2\x1d\x5d\x74\xe2\x7f\xce\x12\xa1\x12\xeb\xa0\x73\x3c\xfc\x9f"
      "\xa7\xf5\x29\xec\x8c\xc6\x39\xcf\xe3\xaa\x47\xbf\x0d\xe7\x31\xc7\x9a\x16"
      "\x73\x0a\xd6\x6f\x61\xb0\x2f\x5c\x6d\x5f\x22\x32\x8a\xc9\x47\x32\xcc\x0e"
      "\x0f\xc6\xac\x76\xa3\x5c\x7d\xeb\xad\x65\x82\x15\xaf\xc4\xea\xc6\x7c\xb4"
      "\x5c\xd7\xad\xf9\xb6\x32\x95\xc6\xfc\xd2\x50\x55\x1c\x32\xb7\x10\xcc\x06"
      "\xb1\x28\x5f\x77\x56\x7b\xba\x21\x0d\x28\xee\x49\xc1\x65\x29\x7a\xcb\x4d"
      "\xd5\x81\x32\x77\xa1\xf3\x64\xb3\x80\x35\x06\x63\xc1\x42\x44\x47\x14\xc4"
      "\x9d\xca\xdd\xd4\x77\x34\xd1\x86\xcf\xc8\xf1\x8a\x8f\x95\xec\xc9\x9f\xb0"
      "\x32\x18\x04\x76\x82\x6b\xb1\xc0\xc0\xad\x43\xe3\x20\xa7\xe3\xed\x0a\x5a"
      "\xc5\xb7\xd7\x45\xb6\x48\x56\x23\xb6\xb8\x1a\xb9\xfa\x55\xcb\x29\x1d\x5d"
      "\x2f\x36\x61\xb5\xbf\x4f\x52\x7e\x22\x52\x9b\xd7\x73\x3e\xf3\x15\xf4\xe7"
      "\xde\x3b\x85\x1b\xac\xe7\x22\xd1",
      1286);
  syscall(__NR_write, r[0], 0x20002100, 0x506);
  memcpy((void*)0x20000180, "./bus", 6);
  res = syscall(__NR_open, 0x20000180, 0, 0x104);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000040 = 0;
  syscall(__NR_sendfile, r[0], r[1], 0x20000040, 0x8000fffffffe);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
