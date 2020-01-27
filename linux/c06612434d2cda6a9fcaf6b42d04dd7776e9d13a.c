// general protection fault in ip6_dst_destroy
// https://syzkaller.appspot.com/bug?id=c06612434d2cda6a9fcaf6b42d04dd7776e9d13a
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>

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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[1], 0x200001c0, 0);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000100, 0x20000200);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000340;
  *(uint32_t*)0x20000340 = 0x4c;
  *(uint16_t*)0x20000344 = 0x18;
  *(uint16_t*)0x20000346 = 0x105;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint8_t*)0x20000350 = 0xa;
  *(uint8_t*)0x20000351 = 0;
  *(uint8_t*)0x20000352 = 0;
  *(uint8_t*)0x20000353 = 0;
  *(uint8_t*)0x20000354 = 0;
  *(uint8_t*)0x20000355 = 0x4e;
  *(uint8_t*)0x20000356 = 0;
  *(uint8_t*)0x20000357 = 0;
  *(uint32_t*)0x20000358 = 0;
  *(uint16_t*)0x2000035c = 8;
  *(uint16_t*)0x2000035e = 0x10;
  *(uint32_t*)0x20000360 = 0xfffffc00;
  *(uint16_t*)0x20000364 = 8;
  *(uint16_t*)0x20000366 = 0x15;
  *(uint16_t*)0x20000368 = 4;
  *(uint16_t*)0x2000036c = 8;
  *(uint16_t*)0x2000036e = 0x10;
  *(uint32_t*)0x20000370 = 0x5fe859d3;
  *(uint16_t*)0x20000374 = 8;
  *(uint16_t*)0x20000376 = 4;
  *(uint32_t*)0x20000378 = 0;
  *(uint16_t*)0x2000037c = 8;
  *(uint16_t*)0x2000037e = 6;
  *(uint32_t*)0x20000380 = -1;
  *(uint16_t*)0x20000384 = 8;
  *(uint16_t*)0x20000386 = 4;
  *(uint32_t*)0x20000388 = r[2];
  *(uint64_t*)0x20000088 = 0x4c;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
