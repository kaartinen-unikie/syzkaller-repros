// KASAN: stack-out-of-bounds Read in string
// https://syzkaller.appspot.com/bug?id=6625234933e74cf3bcf9bfdd8abfa0160d704cc5
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/capability.h>
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
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

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
}

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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  int pid;

  setup_tun(executor_pid, enable_tun);

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid = clone(namespace_sandbox_proc,
              &sandbox_stack[sizeof(sandbox_stack) - 64],
              CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

#ifndef __NR_getsockopt
#define __NR_getsockopt 365
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    syscall(__NR_mmap, 0x20000000ul, 0x3000ul, 0x3ul, 0x32ul,
            0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x2ul, 0x2ul, 0x0ul);
    break;
  case 2:
    NONFAILING(memcpy((void*)0x20000000,
                      "\x97\xb3\x7e\x40\xec\x2c\xc1\xed\xde\x9e\xe7\xb9"
                      "\x1f\x3a\xd4\xe6\x90\xb0\x1c\x00\x80\x00\x56\x9c"
                      "\xa3\x3a\x5c\x00\x00\x00\x00\x80\x00\x00\x01\x41"
                      "\x05\x1a\x91\xb5\x57\x69\xd7\x46\x7c\x3b\x19\x43"
                      "\x96\x5c\xb5\xf7\x75\x22\x8d\xa1\xd4\xfe\xb3\x85"
                      "\x00\x1e\x0b\x5d\x03\xc3\xa5\xb3",
                      68));
    syscall(__NR_setsockopt, r[0], 0x0ul, 0x489ul, 0x20000000ul,
            0x44ul);
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20000000 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20000004 = (uint32_t)0x72);
    NONFAILING(memcpy(
        (void*)0x20000008,
        "\x20\xeb\x74\x07\xcc\x00\x53\x2c\xc3\xd5\x6e\x7a\x27\x05\xfa"
        "\x0b\x87\x9b\x36\x77\xa8\x40\x5e\xf8\x4f\x71\x37\xe9\x20\x93"
        "\x18\xad\xe8\x81\xa1\x63\xb2\x9a\xba\x9b\xf8\xcc\x3c\x1e\xc6"
        "\x22\x60\xcb\xa3\x5b\x54\xc3\x55\x4c\x5c\x64\x8f\x2a\x57\xc9"
        "\x3a\x42\xa0\x47\x7c\x03\x9a\xd1\x04\x3b\xd5\xc3\x89\x79\x2b"
        "\x22\x5a\x24\xc3\x3e\xce\xd7\x25\x34\x7e\xae\x27\xa4\x46\x1d"
        "\x6d\x67\x60\xd7\x0c\xb8\x87\x18\xa2\xfb\x8a\x4a\x56\xea\x7f"
        "\xa9\x26\x14\xa6\xc0\x1f\xdd\xc6\x77",
        114));
    NONFAILING(*(uint32_t*)0x20000ffc = (uint32_t)0x7a);
    syscall(__NR_getsockopt, 0xffffffffffffff9cul, 0x84ul, 0x1bul,
            0x20000000ul, 0x20000ffcul);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[8];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 4; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 4; i++) {
    pthread_create(&th[4 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  install_segv_handler();
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}