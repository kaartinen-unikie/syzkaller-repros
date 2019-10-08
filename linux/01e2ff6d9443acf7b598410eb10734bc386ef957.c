// KASAN: use-after-free Read in __lock_acquire
// https://syzkaller.appspot.com/bug?id=01e2ff6d9443acf7b598410eb10734bc386ef957
// status:fixed
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
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

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

long r[48];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfd4000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x2000cffc = (uint32_t)0x4);
    NONFAILING(*(uint64_t*)0x20edfff0 = (uint64_t)0x77359400);
    NONFAILING(*(uint64_t*)0x20edfff8 = (uint64_t)0x4);
    NONFAILING(*(uint32_t*)0x20048000 = (uint32_t)0x80000000);
    r[5] = syscall(__NR_futex, 0x2000cffcul, 0x80000000000bul, 0x4ul,
                   0x20edfff0ul, 0x20048000ul, 0x0ul);
    break;
  case 2:
    NONFAILING(memcpy((void*)0x20fd4000,
                      "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      16));
    NONFAILING(*(uint32_t*)0x20fd4010 = (uint32_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4014 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4015 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4016 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4017 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4018 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4019 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401b = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd401f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4020 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4021 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4022 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4023 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4024 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4025 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4026 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20fd4027 = (uint8_t)0x0);
    r[28] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x8933ul,
                    0x20fd4000ul);
    break;
  case 3:
    r[29] = syscall(__NR_mmap, 0x20fd7000ul, 0x1000ul, 0x3ul, 0x32ul,
                    0xfffffffffffffffful, 0x0ul);
    break;
  case 4:
    r[30] = syscall(__NR_mmap, 0x20fd4000ul, 0x1000ul, 0x3ul, 0x32ul,
                    0xfffffffffffffffful, 0x0ul);
    break;
  case 5:
    r[31] = syscall(__NR_fstat, 0xfffffffffffffffful, 0x20215fbcul);
    break;
  case 6:
    r[32] = syscall(__NR_mmap, 0x20fd5000ul, 0x1000ul, 0x3ul, 0x32ul,
                    0xfffffffffffffffful, 0x0ul);
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x2000cffc = (uint32_t)0x4);
    NONFAILING(*(uint64_t*)0x2000b000 = (uint64_t)0x77359400);
    NONFAILING(*(uint64_t*)0x2000b008 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20048000 = (uint32_t)0x0);
    r[37] = syscall(__NR_futex, 0x2000cffcul, 0x80000000000bul, 0x4ul,
                    0x2000b000ul, 0x20048000ul, 0x0ul);
    break;
  case 8:
    NONFAILING(*(uint32_t*)0x2000cffc = (uint32_t)0x4);
    NONFAILING(*(uint64_t*)0x20edfff0 = (uint64_t)0x77359400);
    NONFAILING(*(uint64_t*)0x20edfff8 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20048000 = (uint32_t)0x0);
    r[42] = syscall(__NR_futex, 0x2000cffcul, 0x80000000000bul, 0x4ul,
                    0x20edfff0ul, 0x20048000ul, 0x0ul);
    break;
  case 9:
    NONFAILING(*(uint32_t*)0x2000cffc = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20fd7ff0 = (uint64_t)0x77359400);
    NONFAILING(*(uint64_t*)0x20fd7ff8 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20048000 = (uint32_t)0x0);
    r[47] = syscall(__NR_futex, 0x2000cffcul, 0xcul, 0x1ul,
                    0x20fd7ff0ul, 0x20048000ul, 0x0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[20];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 10; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
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