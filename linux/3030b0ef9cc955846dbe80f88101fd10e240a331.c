// general protection fault in mqueue_get_tree
// https://syzkaller.appspot.com/bug?id=3030b0ef9cc955846dbe80f88101fd10e240a331
// status:invalid
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/tcp.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;
  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    exit(1);
  if ((size_t)rv >= size)
    exit(1);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;
  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      exit(1);
  }
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}
static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};
  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static long syz_genetlink_get_family_id(long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
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
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static void setup_cgroups()
{
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  if (!write_file("/syzcgroup/unified/cgroup.subtree_control",
                  "+cpu +memory +io +pids +rdma")) {
  }
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  if (!write_file("/syzcgroup/cpu/cgroup.clone_children", "1")) {
  }
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
  if (!write_file("/proc/self/oom_score_adj", "-1000")) {
  }
}
static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz0:M:0:\x01::./file0:")) {
  }
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz1:M:1:\x02::./file0:POC")) {
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
  setup_cgroups();
  setup_binfmt_misc();
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

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  exit(1);
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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

#define SYZ_HAVE_SETUP_LOOP 1
static void setup_loop()
{
  int pid = getpid();
  char cgroupdir[64];
  char file[128];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  if (!write_file(file, "32")) {
  }
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  if (!write_file(file, "%d", 198 << 20)) {
  }
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  if (!write_file(file, "%d", 199 << 20)) {
  }
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  if (!write_file(file, "%d", 200 << 20)) {
  }
  if (!write_file("/proc/self/oom_score_adj", "-1000")) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  if (!write_file(file, "%d", pid)) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  if (!write_file(file, "%d", pid)) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  if (!write_file(file, "%d", pid)) {
  }
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.cpu")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.net")) {
  }
  if (!write_file("/proc/self/oom_score_adj", "0")) {
  }
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 14; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  setup_loop();
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
  }
}
#ifndef __NR_seccomp
#define __NR_seccomp 317
#endif

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20003000, "/dev/null", 10);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20003000, 0, 0);
    break;
  case 1:
    *(uint32_t*)0x200031c0 = 0x6e;
    syscall(__NR_accept4, 0xffffff9c, 0x20003140, 0x200031c0, 0);
    break;
  case 2:
    syscall(__NR_socket, 0xa, 0, 0x84);
    break;
  case 3:
    *(uint32_t*)0x20003100 = -1;
    syscall(__NR_ioctl, -1, 0x89e2, 0x20003100);
    break;
  case 4:
    *(uint32_t*)0x20000640 = 0;
    syscall(__NR_setsockopt, -1, 1, 1, 0x20000640, 4);
    break;
  case 5:
    *(uint32_t*)0x20000500 = 0;
    *(uint32_t*)0x20000504 = 0;
    *(uint32_t*)0x20000540 = 8;
    syscall(__NR_getsockopt, 0xffffff9c, 0x84, 0xd, 0x20000500, 0x20000540);
    break;
  case 6:
    *(uint32_t*)0x20000580 = 0;
    *(uint32_t*)0x20000584 = 0;
    *(uint32_t*)0x200005c0 = 8;
    syscall(__NR_getsockopt, -1, 0x84, 0x7b, 0x20000580, 0x200005c0);
    break;
  case 7:
    *(uint16_t*)0x20000300 = 0;
    *(uint64_t*)0x20000308 = 0x20000280;
    syscall(__NR_seccomp, 0, 0, 0x20000300);
    break;
  case 8:
    memcpy((void*)0x20000040, "./file0", 8);
    syscall(__NR_open, 0x20000040, 0, 0);
    break;
  case 9:
    memcpy((void*)0x200000c0, "team", 5);
    syz_genetlink_get_family_id(0x200000c0);
    break;
  case 10:
    memcpy((void*)0x20000800,
           "\x65\x72\x73\x70\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000810 = 0;
    syscall(__NR_ioctl, -1, 0x8933, 0x20000800);
    break;
  case 11:
    memcpy((void*)0x20000b00,
           "\x74\x65\x61\x6d\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000b10 = 0;
    syscall(__NR_ioctl, -1, 0x8933, 0x20000b00);
    break;
  case 12:
    *(uint32_t*)0x200010c0 = 0xe8;
    syscall(__NR_getsockopt, -1, 0, 0x11, 0x20000fc0, 0x200010c0);
    break;
  case 13:
    memcpy((void*)0x20001180,
           "\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20001190 = 0;
    syscall(__NR_ioctl, -1, 0x8933, 0x20001180);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
