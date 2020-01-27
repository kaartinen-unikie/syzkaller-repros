// WARNING: ODEBUG bug in io_sqe_files_unregister
// https://syzkaller.appspot.com/bug?id=460cc948740aa1e715156c0edf5d5d397401d557
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

#ifndef __NR_io_uring_register
#define __NR_io_uring_register 427
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  install_segv_handler();
  use_temporary_dir();
  intptr_t res = 0;
  NONFAILING(*(uint32_t*)0x20000080 = 0);
  NONFAILING(*(uint32_t*)0x20000084 = 0);
  NONFAILING(*(uint32_t*)0x20000088 = 0);
  NONFAILING(*(uint32_t*)0x2000008c = 0);
  NONFAILING(*(uint32_t*)0x20000090 = 0);
  NONFAILING(*(uint32_t*)0x20000094 = 0);
  NONFAILING(*(uint32_t*)0x20000098 = 0);
  NONFAILING(*(uint32_t*)0x2000009c = 0);
  NONFAILING(*(uint32_t*)0x200000a0 = 0);
  NONFAILING(*(uint32_t*)0x200000a4 = 0);
  NONFAILING(*(uint32_t*)0x200000a8 = 0);
  NONFAILING(*(uint32_t*)0x200000ac = 0);
  NONFAILING(*(uint32_t*)0x200000b0 = 0);
  NONFAILING(*(uint32_t*)0x200000b4 = 0);
  NONFAILING(*(uint32_t*)0x200000b8 = 0);
  NONFAILING(*(uint32_t*)0x200000bc = 0);
  NONFAILING(*(uint32_t*)0x200000c0 = 0);
  NONFAILING(*(uint32_t*)0x200000c4 = 0);
  NONFAILING(*(uint64_t*)0x200000c8 = 0);
  NONFAILING(*(uint32_t*)0x200000d0 = 0);
  NONFAILING(*(uint32_t*)0x200000d4 = 0);
  NONFAILING(*(uint32_t*)0x200000d8 = 0);
  NONFAILING(*(uint32_t*)0x200000dc = 0);
  NONFAILING(*(uint32_t*)0x200000e0 = 0);
  NONFAILING(*(uint32_t*)0x200000e4 = 0);
  NONFAILING(*(uint32_t*)0x200000e8 = 0);
  NONFAILING(*(uint32_t*)0x200000ec = 0);
  NONFAILING(*(uint64_t*)0x200000f0 = 0);
  res = syscall(__NR_io_uring_setup, 0xa4, 0x20000080ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20000280 = -1);
  NONFAILING(*(uint32_t*)0x20000284 = -1);
  NONFAILING(*(uint32_t*)0x20000288 = -1);
  syscall(__NR_io_uring_register, r[0], 2ul, 0x20000280ul,
          0x40000000000000a1ul);
  return 0;
}
