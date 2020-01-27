// WARNING: ODEBUG bug in tcindex_destroy_work
// https://syzkaller.appspot.com/bug?id=7700d823db05d055fc4d9b35704358a077a0324c
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
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

#include <linux/capability.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

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

static struct {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
} nlmsg;

static void netlink_init(int typ, int flags, const void* data, int size)
{
  memset(&nlmsg, 0, sizeof(nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg.pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(int typ, const void* data, int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg.pos += NLMSG_ALIGN(attr->nla_len);
}

static void netlink_nest(int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_type = typ;
  nlmsg.pos += sizeof(*attr);
  nlmsg.nested[nlmsg.nesting++] = attr;
}

static void netlink_done(void)
{
  struct nlattr* attr = nlmsg.nested[--nlmsg.nesting];
  attr->nla_len = nlmsg.pos - (char*)attr;
}

static int netlink_send(int sock)
{
  if (nlmsg.pos > nlmsg.buf + sizeof(nlmsg.buf) || nlmsg.nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_len = nlmsg.pos - nlmsg.buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg.buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0);
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static void netlink_add_device_impl(const char* type, const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  if (name)
    netlink_attr(IFLA_IFNAME, name, strlen(name));
  netlink_nest(IFLA_LINKINFO);
  netlink_attr(IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(int sock, const char* type, const char* name)
{
  netlink_add_device_impl(type, name);
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_veth(int sock, const char* name, const char* peer)
{
  netlink_add_device_impl("veth", name);
  netlink_nest(IFLA_INFO_DATA);
  netlink_nest(VETH_INFO_PEER);
  nlmsg.pos += sizeof(struct ifinfomsg);
  netlink_attr(IFLA_IFNAME, peer, strlen(peer));
  netlink_done();
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_hsr(int sock, const char* name, const char* slave1,
                            const char* slave2)
{
  netlink_add_device_impl("hsr", name);
  netlink_nest(IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_device_change(int sock, const char* name, bool up,
                                  const char* master, const void* mac,
                                  int macsize)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  netlink_attr(IFLA_IFNAME, name, strlen(name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int netlink_add_addr(int sock, const char* dev, const void* addr,
                            int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr, sizeof(hdr));
  netlink_attr(IFA_LOCAL, addr, addrsize);
  netlink_attr(IFA_ADDRESS, addr, addrsize);
  return netlink_send(sock);
}

static void netlink_add_addr4(int sock, const char* dev, const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(int sock, const char* dev, const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa
static void initialize_netdevices(void)
{
  char netdevsim[16];
  sprintf(netdevsim, "netdevsim%d", (int)procid);
  struct {
    const char* type;
    const char* dev;
  } devtypes[] = {
      {"ip6gretap", "ip6gretap0"}, {"bridge", "bridge0"},
      {"vcan", "vcan0"},           {"bond", "bond0"},
      {"team", "team0"},           {"dummy", "dummy0"},
      {"nlmon", "nlmon0"},         {"caif", "caif0"},
      {"batadv", "batadv0"},       {"vxcan", "vxcan1"},
      {"netdevsim", netdevsim},    {"veth", 0},
  };
  const char* devmasters[] = {"bridge", "bond", "team"};
  struct {
    const char* name;
    int macsize;
    bool noipv6;
  } devices[] = {
      {"lo", ETH_ALEN},
      {"sit0", 0},
      {"bridge0", ETH_ALEN},
      {"vcan0", 0, true},
      {"tunl0", 0},
      {"gre0", 0},
      {"gretap0", ETH_ALEN},
      {"ip_vti0", 0},
      {"ip6_vti0", 0},
      {"ip6tnl0", 0},
      {"ip6gre0", 0},
      {"ip6gretap0", ETH_ALEN},
      {"erspan0", ETH_ALEN},
      {"bond0", ETH_ALEN},
      {"veth0", ETH_ALEN},
      {"veth1", ETH_ALEN},
      {"team0", ETH_ALEN},
      {"veth0_to_bridge", ETH_ALEN},
      {"veth1_to_bridge", ETH_ALEN},
      {"veth0_to_bond", ETH_ALEN},
      {"veth1_to_bond", ETH_ALEN},
      {"veth0_to_team", ETH_ALEN},
      {"veth1_to_team", ETH_ALEN},
      {"veth0_to_hsr", ETH_ALEN},
      {"veth1_to_hsr", ETH_ALEN},
      {"hsr0", 0},
      {"dummy0", ETH_ALEN},
      {"nlmon0", 0},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(sock, slave0, false, master, 0, 0);
    netlink_device_change(sock, slave1, false, master, 0, 0);
  }
  netlink_device_change(sock, "bridge_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "bridge_slave_1", true, 0, 0, 0);
  netlink_add_veth(sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(sock, "hsr_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "hsr_slave_1", true, 0, 0, 0);
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize);
  }
  close(sock);
}
static void initialize_netdevices_init(void)
{
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  struct {
    const char* type;
    int macsize;
    bool noipv6;
    bool noup;
  } devtypes[] = {
      {"nr", 7, true}, {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(sock, dev, !devtypes[i].noup, 0, &macaddr, macsize);
  }
  close(sock);
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
  write_file("/syzcgroup/unified/cgroup.subtree_control",
             "+cpu +memory +io +pids +rdma");
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  write_file("/syzcgroup/cpu/cgroup.clone_children", "1");
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
}

static void setup_cgroups_loop()
{
  int pid = getpid();
  char file[128];
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  write_file(file, "32");
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  write_file(file, "%d", 298 << 20);
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  write_file(file, "%d", 300 << 20);
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
}

static void setup_cgroups_test()
{
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
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
  setup_cgroups();
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
  initialize_netdevices_init();
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

static void setup_loop()
{
  setup_cgroups_loop();
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setup_cgroups_test();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:\x01::./file0:");
  write_file("/proc/sys/fs/binfmt_misc/register",
             ":syz1:M:1:\x02::./file0:POC");
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
      close_fds();
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

uint64_t r[7] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xa, 2, 0x73);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0);
  *(uint32_t*)0x20000004 = htobe32(0);
  *(uint64_t*)0x20000008 = htobe64(0);
  *(uint64_t*)0x20000010 = htobe64(1);
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_bind, r[0], 0x20000000, 0x69);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[3] = res;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[4] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[4], 0x200001c0, 0);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[4], 0x20000100, 0x20000200);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000080;
  memcpy((void*)0x20000080, "\x48\x00\x00\x00\x10\x00\x05\x07\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000094 = r[5];
  memcpy((void*)0x20000098, "\x00\x00\x00\x00\x00\x00\x00\x00\x28\x00\x12\x00"
                            "\x0c\x00\x01\x00\x76\x65\x74\x68\x00\x00\x00\x00"
                            "\x18\x00\x02\x00\x14\x00\x01\x00\x00\x00\x00\x00",
         36);
  *(uint32_t*)0x200000bc = 0;
  memcpy((void*)0x200000c0, "\x00\x00\xbd", 3);
  *(uint64_t*)0x20000008 = 0x48;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[3], 0x20000040, 0);
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x20000380;
  *(uint32_t*)0x20000380 = 0x74;
  *(uint16_t*)0x20000384 = 0x24;
  *(uint16_t*)0x20000386 = 0x507;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint8_t*)0x20000390 = 0;
  *(uint32_t*)0x20000394 = r[5];
  *(uint16_t*)0x20000398 = 0;
  *(uint16_t*)0x2000039a = 0;
  *(uint16_t*)0x2000039c = -1;
  *(uint16_t*)0x2000039e = -1;
  *(uint16_t*)0x200003a0 = 0;
  *(uint16_t*)0x200003a2 = 0;
  *(uint16_t*)0x200003a4 = 8;
  *(uint16_t*)0x200003a6 = 1;
  memcpy((void*)0x200003a8, "sfq\000", 4);
  *(uint16_t*)0x200003ac = 0x48;
  *(uint16_t*)0x200003ae = 2;
  *(uint32_t*)0x200003b0 = 0;
  *(uint32_t*)0x200003b4 = 0;
  *(uint32_t*)0x200003b8 = 0;
  *(uint32_t*)0x200003bc = 0;
  *(uint32_t*)0x200003c0 = 0;
  *(uint32_t*)0x200003c4 = 0;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint32_t*)0x200003d0 = 0;
  *(uint32_t*)0x200003d4 = 0;
  *(uint8_t*)0x200003d8 = 0;
  *(uint8_t*)0x200003d9 = 0;
  *(uint8_t*)0x200003da = 0;
  *(uint8_t*)0x200003db = 0;
  *(uint32_t*)0x200003dc = 0;
  *(uint32_t*)0x200003e0 = 0;
  *(uint32_t*)0x200003e4 = 0;
  *(uint32_t*)0x200003e8 = 0;
  *(uint32_t*)0x200003ec = 0;
  *(uint32_t*)0x200003f0 = 0;
  *(uint64_t*)0x20000288 = 0x74;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000240, 0);
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[6] = res;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000ac0;
  memcpy((void*)0x20000ac0, "\x3c\x00\x00\x00\x2c\x00\x01\x07\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000ad4 = r[5];
  memcpy((void*)0x20000ad8,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x0c\x00\x01\x00\x74"
         "\x63\x69\x6e\x64\x65\x78\x00\x0c\x00\x02\x00\x08\x00\x02\x00\x00\x00"
         "\x00\x00\xc4\x15\x80\x90\x9a\xf1\xc9\x9b\xc9\x50\x82\xff\x3a\x31\x60"
         "\x84\x92\x71\x4e\x11\xd7\xec\x69\x03\x72\x66\x1b\x8d\xd6\x53\xfb\x7e"
         "\x31\xbb\x6b\x22\x11\x72\x8d\x7a\x80\x97\xf1\x29\x06\xfb\x87\xa4\xbb"
         "\x88\xb3\x33\xd2\x96\x73\x1b\x45\x64\xc8\xb6\x18\xd6\x91\xc8\xdd\x7a"
         "\x34\x55\x65\xd7\x2c\xbc\x7d\x2a\xdc\x8e\xed\x1f\x3b\xac\xf5\xad\x19"
         "\x8e\x37\x54\x94\x2c\x72\x5b\x41\x6d\x93\x32\x32\xbe\x3e\x06\x27\x3b"
         "\xbe\x4e\xeb\x8a\x7f\x1a\x7c\x3b\x57\x4c\x62\xe1\x37\xfb\x6d\xd4\xd6"
         "\x67\x54\x09\x0c\xcf\x6d\xd4\x5c\x73\xf3\x0f\x0f\x1f\x94\x72\x4b\x83"
         "\x16\xf8\xeb\x41\x6c\x41\x24\x53\xd4\xcb\x09\x35\x79\x18\x78\xc7\x76"
         "\x6a\xfd\xd0\x2e\x42\xf4\x45\xf1\xc5\x81\xfd\x01\xa1\x0d\x59\x69\x95"
         "\x54\x32\x5b\x6a\x62\x77\xd1\xcd\xca\xb6\x04\xdf\x0d\x71\x7f\x00\xd2"
         "\x2b\xc5\xb9\x40\x7f\x20\xb9\x28\xcf\x58\xf3\x30\x2b\x42\x2c\xd6\x5a"
         "\xb2\x9c\x5b\x90\x5a\xf6\x63\x04\x8f\xcb\xc1\x00\xa6\xc0\xfb\xd6\x17"
         "\x6f\x5c",
         257);
  *(uint64_t*)0x200000c8 = 0x3c;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmsg, r[6], 0x20000140, 0);
  *(uint64_t*)0x20000340 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x20;
  *(uint16_t*)0x20000044 = 0x11;
  *(uint16_t*)0x20000046 = 1;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = r[5];
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint64_t*)0x200000c8 = 0x20;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000340, 0);
  *(uint64_t*)0x20000a00 = 0x20000040;
  *(uint16_t*)0x20000040 = 0xa;
  *(uint16_t*)0x20000042 = htobe16(0x4e20);
  *(uint32_t*)0x20000044 = htobe32(0x10001);
  *(uint8_t*)0x20000048 = 0xfe;
  *(uint8_t*)0x20000049 = 0x80;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint8_t*)0x20000052 = 0;
  *(uint8_t*)0x20000053 = 0;
  *(uint8_t*)0x20000054 = 0;
  *(uint8_t*)0x20000055 = 0;
  *(uint8_t*)0x20000056 = 0;
  *(uint8_t*)0x20000057 = 0xbb;
  *(uint32_t*)0x20000058 = 0x3f;
  *(uint32_t*)0x20000a08 = 0x1c;
  *(uint64_t*)0x20000a10 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000080;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000a18 = 1;
  *(uint64_t*)0x20000a20 = 0;
  *(uint64_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint64_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint64_t*)0x20000a50 = 0x20000580;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x20000280;
  memcpy((void*)0x20000280,
         "\x8b\xa0\x52\x20\xc6\x24\x95\x51\x3d\x5d\xfa\x48\x54\x52\xfe\xae\x13"
         "\x47\x4b\x81\xd1\x42\x04\x5d\x0a\x22\x86\xca\x34\x28\xcc\xb3\x59\x4d"
         "\x15\x72\x5b\x15\xee\x1b\xc7\x4e\xa8\xea\x65\x0c\x61\xb8\xfe\x50\xbc"
         "\x14\x7d\xd7\x36\xeb\xd2\x83\xb7\xc1\x3f\xd1\xee\xb2\xd7\xdc\xeb\xb6"
         "\x0e\x33\x56\x06\xab\xb9\x29\x6b\x6f\x07\x44\xbb\xbd\xc7\x7f\x85\x5c"
         "\x68\xcb\x8a\x0e\x67\x77\xfb\x0f\x12\x8e\x51\x55\x35\x68\x09\xe0\x21"
         "\x23\x09\x62\xa6\x60\xdb\x7d\xef\x42\xfa\xe2\x76\x06\xe5\x96\xc8\xa6"
         "\xb5\x23\xe6\x2d\x61\xe1\xe2\x4b\x50\x8c\x33\x83\xcf\x31\xfe\x55\x08"
         "\xe8\xb9\x3c\xa2\x3b\x88\x16\x52\x27\x83\xac\x31\x4c\x67\xa7\x8c\xc2"
         "\x53\xd2\xe4\x9f\x43\x1b\xdd\xf3\x13\x26\xc6\xcf\x8c\x26\x09\x2b",
         169);
  *(uint64_t*)0x20000598 = 0xa9;
  *(uint64_t*)0x200005a0 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x7e\xcf\x8b\x37\xdc\xc3\x5d\x21\xd7\x22\x5b\xeb\xf7\x8c\x38\xc8\x93"
         "\x10\x21\xa4\xaf\xfc\xd1\x54\x4c\x30\xfa\x17\x8d\x4b\x9e\x42\x65\x6d"
         "\x3c\x99\xfa\x4b\x4e\x61\xc5\x81\x61\xda\x1c\x8a\xc4\xbb\xeb\xee\xa9"
         "\x71\xad\xaf\xd0\xb5\x4e\x8c\xc7\x4f\xf6\xd7\x15\x6f\xeb\x31\x22\xee"
         "\x9b\x77\x0f\x4e\xd0\x89\xad\xfc\x38\x3d\x6c\xd2\x56\xde\xe6\xbb\x31"
         "\x91\x9c\x1b\x55\x92\xce\x78\xca\x3e\xb4\x3b\xaf\xc4\x98\xca\x1e\xb5"
         "\x1b\x85\x38\xd5\xa9\x04\x6b\x62\xe2\xb6\xef\x80\x53\x96\x65\x92\x56"
         "\xa3\x5f\x21\x0f\x1e\xae\xb5\xa2\xc1\x5f\x3e\x0e\x70\x37\x9b\x0b\x03"
         "\xbf\xe2\x72\x85\xc4\x78\x52\xa9\x07\x34\xe4\x05\x16\x89\x45\x75\x8f"
         "\xda\xb6\x0f\x00\x6f\x1e\x4b",
         160);
  *(uint64_t*)0x200005a8 = 0xa0;
  *(uint64_t*)0x200005b0 = 0x20000400;
  memcpy((void*)0x20000400, "\x71\x07\xca\xf0\x80\xc6\x33\x9b\x1a\x49", 10);
  *(uint64_t*)0x200005b8 = 0xa;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x20000a58 = 6;
  *(uint64_t*)0x20000a60 = 0x20000740;
  *(uint64_t*)0x20000740 = 0x24;
  *(uint32_t*)0x20000748 = 0x29;
  *(uint32_t*)0x2000074c = 0x32;
  *(uint8_t*)0x20000750 = 0;
  *(uint8_t*)0x20000751 = 0;
  *(uint8_t*)0x20000752 = 0;
  *(uint8_t*)0x20000753 = 0;
  *(uint8_t*)0x20000754 = 0;
  *(uint8_t*)0x20000755 = 0;
  *(uint8_t*)0x20000756 = 0;
  *(uint8_t*)0x20000757 = 0;
  *(uint8_t*)0x20000758 = 0;
  *(uint8_t*)0x20000759 = 0;
  *(uint8_t*)0x2000075a = 0;
  *(uint8_t*)0x2000075b = 0;
  *(uint8_t*)0x2000075c = 0;
  *(uint8_t*)0x2000075d = 0;
  *(uint8_t*)0x2000075e = 0;
  *(uint8_t*)0x2000075f = 0;
  *(uint32_t*)0x20000760 = r[5];
  *(uint64_t*)0x20000768 = 0xc0;
  *(uint32_t*)0x20000770 = 0x29;
  *(uint32_t*)0x20000774 = 0x37;
  *(uint8_t*)0x20000778 = 0;
  *(uint8_t*)0x20000779 = 0x14;
  *(uint8_t*)0x2000077a = 0;
  *(uint8_t*)0x2000077b = 0;
  *(uint8_t*)0x2000077c = 0;
  *(uint8_t*)0x2000077d = 0;
  *(uint8_t*)0x2000077e = 0;
  *(uint8_t*)0x2000077f = 0;
  *(uint8_t*)0x20000780 = 0x6d;
  *(uint8_t*)0x20000781 = 0x87;
  memcpy((void*)0x20000782,
         "\xe3\xba\xe7\xb5\x7b\x4f\x68\x82\xab\x5b\x43\x9f\x4e\x3f\x56\x02\x0c"
         "\x2b\xab\x25\x6d\x3a\x85\x44\x26\x0e\x91\xe3\x5d\x34\x6b\x36\x79\x78"
         "\xff\xe8\x05\x51\x6c\x3c\x35\xf1\xb0\x9e\xf1\x06\x31\x04\x31\x0b\x6e"
         "\x55\x1b\xa5\x4c\x84\xe4\x9b\x65\x13\x5e\xde\x93\x86\xc0\xfa\xf1\xf0"
         "\x7d\x14\x76\x2e\xd4\xb8\x17\x9f\x24\x50\xd0\x11\xba\xce\x92\xd1\x98"
         "\x6b\x2c\xa3\xd7\x87\xec\x59\xfc\x88\xa8\x14\xfe\xdb\x04\x9e\xff\x2d"
         "\x0b\x05\xe0\x7b\x75\xf4\xfd\x0f\xf9\x51\xd0\x47\x08\xc1\x62\xf7\x99"
         "\x5f\x94\x39\xde\xf1\x25\x1b\xa0\x58\x0e\x17\x1c\x8a\x06\x93\xf8",
         135);
  *(uint8_t*)0x20000809 = 0xc2;
  *(uint8_t*)0x2000080a = 4;
  *(uint32_t*)0x2000080b = htobe32(2);
  *(uint8_t*)0x2000080f = 0xc9;
  *(uint8_t*)0x20000810 = 0x10;
  *(uint8_t*)0x20000811 = -1;
  *(uint8_t*)0x20000812 = 1;
  *(uint8_t*)0x20000813 = 0;
  *(uint8_t*)0x20000814 = 0;
  *(uint8_t*)0x20000815 = 0;
  *(uint8_t*)0x20000816 = 0;
  *(uint8_t*)0x20000817 = 0;
  *(uint8_t*)0x20000818 = 0;
  *(uint8_t*)0x20000819 = 0;
  *(uint8_t*)0x2000081a = 0;
  *(uint8_t*)0x2000081b = 0;
  *(uint8_t*)0x2000081c = 0;
  *(uint8_t*)0x2000081d = 0;
  *(uint8_t*)0x2000081e = 0;
  *(uint8_t*)0x2000081f = 0;
  *(uint8_t*)0x20000820 = 1;
  *(uint64_t*)0x20000828 = 0x24;
  *(uint32_t*)0x20000830 = 0x29;
  *(uint32_t*)0x20000834 = 0x32;
  *(uint8_t*)0x20000838 = 0xfe;
  *(uint8_t*)0x20000839 = 0x80;
  *(uint8_t*)0x2000083a = 0;
  *(uint8_t*)0x2000083b = 0;
  *(uint8_t*)0x2000083c = 0;
  *(uint8_t*)0x2000083d = 0;
  *(uint8_t*)0x2000083e = 0;
  *(uint8_t*)0x2000083f = 0;
  *(uint8_t*)0x20000840 = 0;
  *(uint8_t*)0x20000841 = 0;
  *(uint8_t*)0x20000842 = 0;
  *(uint8_t*)0x20000843 = 0;
  *(uint8_t*)0x20000844 = 0;
  *(uint8_t*)0x20000845 = 0;
  *(uint8_t*)0x20000846 = 0;
  *(uint8_t*)0x20000847 = 0xaa;
  *(uint32_t*)0x20000848 = 0;
  *(uint64_t*)0x20000850 = 0x30;
  *(uint32_t*)0x20000858 = 0x29;
  *(uint32_t*)0x2000085c = 0x36;
  *(uint8_t*)0x20000860 = 0x2e;
  *(uint8_t*)0x20000861 = 2;
  *(uint8_t*)0x20000862 = 0;
  *(uint8_t*)0x20000863 = 0;
  *(uint8_t*)0x20000864 = 0;
  *(uint8_t*)0x20000865 = 0;
  *(uint8_t*)0x20000866 = 0;
  *(uint8_t*)0x20000867 = 0;
  *(uint8_t*)0x20000868 = 0xc9;
  *(uint8_t*)0x20000869 = 0x10;
  memcpy((void*)0x2000086a,
         "\x23\x22\x96\xf7\x39\xf9\x0a\x8d\x50\x76\x03\x9d\x3b\xe7\x87\x24",
         16);
  *(uint64_t*)0x20000a68 = 0x140;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint64_t*)0x20000a80 = 0x20000880;
  *(uint16_t*)0x20000880 = 0xa;
  *(uint16_t*)0x20000882 = htobe16(0x4e24);
  *(uint32_t*)0x20000884 = htobe32(0x400);
  *(uint8_t*)0x20000888 = 0xfe;
  *(uint8_t*)0x20000889 = 0x80;
  *(uint8_t*)0x2000088a = 0;
  *(uint8_t*)0x2000088b = 0;
  *(uint8_t*)0x2000088c = 0;
  *(uint8_t*)0x2000088d = 0;
  *(uint8_t*)0x2000088e = 0;
  *(uint8_t*)0x2000088f = 0;
  *(uint8_t*)0x20000890 = 0;
  *(uint8_t*)0x20000891 = 0;
  *(uint8_t*)0x20000892 = 0;
  *(uint8_t*)0x20000893 = 0;
  *(uint8_t*)0x20000894 = 0;
  *(uint8_t*)0x20000895 = 0;
  *(uint8_t*)0x20000896 = 0;
  *(uint8_t*)0x20000897 = 0xbb;
  *(uint32_t*)0x20000898 = 0x1000;
  *(uint32_t*)0x20000a88 = 0x1c;
  *(uint64_t*)0x20000a90 = 0;
  *(uint64_t*)0x20000a98 = 0;
  *(uint64_t*)0x20000aa0 = 0x200009c0;
  *(uint64_t*)0x200009c0 = 0x14;
  *(uint32_t*)0x200009c8 = 0x29;
  *(uint32_t*)0x200009cc = 0x3e;
  *(uint32_t*)0x200009d0 = 6;
  *(uint64_t*)0x20000aa8 = 0x18;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  syscall(__NR_sendmmsg, -1, 0x20000a00, 3, 0x40);
  *(uint16_t*)0x20000180 = 0xa;
  *(uint16_t*)0x20000182 = htobe16(0);
  *(uint32_t*)0x20000184 = htobe32(0);
  *(uint8_t*)0x20000188 = 0xfe;
  *(uint8_t*)0x20000189 = 0x80;
  *(uint8_t*)0x2000018a = 0;
  *(uint8_t*)0x2000018b = 0;
  *(uint8_t*)0x2000018c = 0;
  *(uint8_t*)0x2000018d = 0;
  *(uint8_t*)0x2000018e = 0;
  *(uint8_t*)0x2000018f = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint8_t*)0x20000192 = 0;
  *(uint8_t*)0x20000193 = 0;
  *(uint8_t*)0x20000194 = 0;
  *(uint8_t*)0x20000195 = 0;
  *(uint8_t*)0x20000196 = 0;
  *(uint8_t*)0x20000197 = 0x10;
  *(uint32_t*)0x20000198 = 0;
  syscall(__NR_connect, r[0], 0x20000180, 0x80);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_binfmt_misc();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}