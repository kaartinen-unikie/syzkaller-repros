// kernel panic: Attempted to kill init! (2)
// https://syzkaller.appspot.com/bug?id=2750ba2ca7720b03a59184ff76397e5198164848
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
  for (;;) {
  }
}

static void install_segv_handler()
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

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                    \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format,
                  args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf(
        "tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as "
           "intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf)));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  int pid = fork();
  if (pid)
    return pid;

  sandbox_common();
  setup_tun(executor_pid, enable_tun);

  loop();
  doexit(1);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_finit_module
#define __NR_finit_module 313
#endif

long r[126];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_inotify_init1, 0x0ul);
    break;
  case 2:
    r[2] = syscall(__NR_dup, r[1]);
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x208c8ff4 = (uint32_t)0x10000008);
    NONFAILING(*(uint64_t*)0x208c8ff8 = (uint64_t)0x0);
    r[5] = syscall(__NR_epoll_ctl, r[2], 0x1ul, r[2], 0x208c8ff4ul);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x202b6000 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x202b6004 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x202b6008 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x202b600c = (uint32_t)0x1);
    NONFAILING(*(uint32_t*)0x202b6010 = (uint32_t)0x5);
    NONFAILING(*(uint32_t*)0x202b6014 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x202b6018 = (uint32_t)0x8);
    NONFAILING(*(uint8_t*)0x202b601c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b601d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b601e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b601f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6020 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6021 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6022 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6023 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6024 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6025 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6026 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6027 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6028 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6029 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602b = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b602f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6030 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6031 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6032 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6033 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6034 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6035 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6036 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6037 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6038 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6039 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603b = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b603f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6040 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6041 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6042 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6043 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6044 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6045 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6046 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6047 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6048 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6049 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604b = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b604f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6050 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6051 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6052 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6053 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6054 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6055 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6056 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6057 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6058 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b6059 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b605a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x202b605b = (uint8_t)0x0);
    r[77] = syscall(__NR_ioctl, r[2], 0xc0605345ul, 0x202b6000ul);
    break;
  case 5:
    r[78] = syscall(__NR_ioctl, r[2], 0xc0045878ul);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x20048000 = (uint32_t)0xc);
    r[80] = syscall(__NR_getsockopt, 0xfffffffffffffffful, 0x1ul,
                    0x11ul, 0x2003cff4ul, 0x20048000ul);
    if (r[80] != -1)
      NONFAILING(r[81] = *(uint32_t*)0x2003cff4);
    break;
  case 7:
    NONFAILING(memcpy((void*)0x20dd9fef,
                      "\x2f\x64\x65\x76\x2f\x76\x67\x61\x5f\x61\x72\x62"
                      "\x69\x74\x65\x72\x00",
                      17));
    r[83] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20dd9feful,
                    0x40000ul, 0x0ul);
    break;
  case 8:
    r[84] = syscall(__NR_ioctl, r[83], 0x5425ul, 0xfaul);
    break;
  case 9:
    NONFAILING(*(uint32_t*)0x20eae000 = (uint32_t)0xc);
    r[86] = syscall(__NR_getsockopt, r[2], 0x1ul, 0x11ul, 0x20eed000ul,
                    0x20eae000ul);
    break;
  case 10:
    r[87] = syscall(__NR_fcntl, r[1], 0x8ul, r[81]);
    break;
  case 11:
    r[88] = syscall(__NR_fcntl, r[2], 0x10ul, 0x206aeff8ul);
    if (r[88] != -1)
      NONFAILING(r[89] = *(uint32_t*)0x206aeffc);
    break;
  case 12:
    NONFAILING(*(uint32_t*)0x20dd8fff = (uint32_t)0xc);
    r[91] = syscall(__NR_getsockopt, r[83], 0x1ul, 0x11ul, 0x2046b000ul,
                    0x20dd8ffful);
    break;
  case 13:
    r[92] = syscall(__NR_ptrace, 0x4206ul, r[89], 0x0ul, 0x0ul);
    break;
  case 14:
    r[93] = syscall(__NR_setpgid, r[89], r[89]);
    break;
  case 15:
    NONFAILING(*(uint32_t*)0x203d6000 = (uint32_t)0x6);
    NONFAILING(*(uint16_t*)0x203d6004 = (uint16_t)0x7f);
    NONFAILING(*(uint16_t*)0x203d6006 = (uint16_t)0x6);
    NONFAILING(*(uint16_t*)0x203d6008 = (uint16_t)0x4);
    NONFAILING(*(uint16_t*)0x203d600a = (uint16_t)0x5);
    NONFAILING(*(uint16_t*)0x203d600c = (uint16_t)0x0);
    NONFAILING(*(uint16_t*)0x203d600e = (uint16_t)0x6);
    NONFAILING(*(uint32_t*)0x20d2b000 = (uint32_t)0x10);
    r[102] = syscall(__NR_getsockopt, r[83], 0x84ul, 0x16ul,
                     0x203d6000ul, 0x20d2b000ul);
    break;
  case 16:
    NONFAILING(*(uint32_t*)0x20851000 = (uint32_t)0x10);
    r[104] = syscall(__NR_accept, r[2], 0x20f69ff0ul, 0x20851000ul);
    break;
  case 17:
    r[105] = syscall(__NR_fanotify_init, 0x3ul, 0x8800ul);
    break;
  case 18:
    NONFAILING(memcpy((void*)0x203c0feb,
                      "\x2f\x64\x65\x76\x2f\x70\x6b\x74\x63\x64\x76\x64"
                      "\x2f\x63\x6f\x6e\x74\x72\x6f\x6c\x00",
                      21));
    r[107] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x203c0febul,
                     0x14000ul, 0x0ul);
    break;
  case 19:
    NONFAILING(*(uint32_t*)0x2006cff4 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x2006cff8 = (uint32_t)0xffffffff);
    NONFAILING(*(uint32_t*)0x2006cffc = (uint32_t)0x100007f);
    r[111] = syscall(__NR_setsockopt, r[104], 0x0ul, 0x2ful,
                     0x2006cff4ul, 0xcul);
    break;
  case 20:
    NONFAILING(*(uint32_t*)0x20b4affc = (uint32_t)0xb);
    r[113] = syscall(__NR_getsockopt, r[107], 0x84ul, 0xbul,
                     0x20adeff5ul, 0x20b4affcul);
    break;
  case 21:
    NONFAILING(
        memcpy((void*)0x208ab000,
               "\x24\xb7\x7b\x5c\x76\x6d\x6e\x65\x74\x30\x70\x6f\x73"
               "\x69\x78\x5f\x61\x63\x6c\x5f\x61\x63\x63\x65\x73\x73"
               "\x6b\x65\x79\x72\x69\x6e\x67\xcb\x2e\x00",
               36));
    r[115] = syscall(__NR_finit_module, r[1], 0x208ab000ul, 0x2ul);
    break;
  case 22:
    NONFAILING(*(uint32_t*)0x2083b000 = (uint32_t)0xc);
    r[117] = syscall(__NR_getsockopt, r[83], 0x84ul, 0x1dul,
                     0x204a9ff4ul, 0x2083b000ul);
    break;
  case 23:
    r[118] = syscall(__NR_ptrace, 0x4207ul, r[89]);
    break;
  case 24:
    r[119] = syscall(__NR_ioctl, r[107], 0x5412ul, 0x6ul);
    break;
  case 25:
    r[120] = syscall(__NR_ptrace, 0xdul, r[89], 0xfffffffffffffffcul,
                     0x20a66000ul);
    break;
  case 26:
    NONFAILING(memcpy((void*)0x208c1000,
                      "\x2f\x64\x65\x76\x2f\x69\x6e\x70\x75\x74\x2f\x6d"
                      "\x69\x63\x65\x00",
                      16));
    r[122] = syz_open_dev(0x208c1000ul, 0x0ul, 0x40ul);
    break;
  case 27:
    NONFAILING(
        memcpy((void*)0x20680ff4,
               "\x2f\x64\x65\x76\x2f\x72\x66\x6b\x69\x6c\x6c\x00", 12));
    r[124] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20680ff4ul,
                     0x22000ul, 0x0ul);
    break;
  case 28:
    r[125] = syscall(__NR_ioctl, r[122], 0x541bul, 0x20212000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[58];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 29; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 29; i++) {
    pthread_create(&th[29 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      install_segv_handler();
      use_temporary_dir();
      int pid = do_sandbox_none(i, true);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
