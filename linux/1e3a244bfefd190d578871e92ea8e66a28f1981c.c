// general protection fault in hash_netport6_uadt
// https://syzkaller.appspot.com/bug?id=1e3a244bfefd190d578871e92ea8e66a28f1981c
// status:dup
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
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
#include <linux/futex.h>
#include <linux/genetlink.h>
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
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
};

static struct nlmsg nlmsg;

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static void netlink_nest(struct nlmsg* nlmsg, int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_type = typ;
  nlmsg->pos += sizeof(*attr);
  nlmsg->nested[nlmsg->nesting++] = attr;
}

static void netlink_done(struct nlmsg* nlmsg)
{
  struct nlattr* attr = nlmsg->nested[--nlmsg->nesting];
  attr->nla_len = nlmsg->pos - (char*)attr;
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (hdr->nlmsg_type == NLMSG_DONE) {
    *reply_len = 0;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
}

static void netlink_add_device_impl(struct nlmsg* nlmsg, const char* type,
                                    const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(nlmsg, RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  if (name)
    netlink_attr(nlmsg, IFLA_IFNAME, name, strlen(name));
  netlink_nest(nlmsg, IFLA_LINKINFO);
  netlink_attr(nlmsg, IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(struct nlmsg* nlmsg, int sock, const char* type,
                               const char* name)
{
  netlink_add_device_impl(nlmsg, type, name);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_veth(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* peer)
{
  netlink_add_device_impl(nlmsg, "veth", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_nest(nlmsg, VETH_INFO_PEER);
  nlmsg->pos += sizeof(struct ifinfomsg);
  netlink_attr(nlmsg, IFLA_IFNAME, peer, strlen(peer));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_hsr(struct nlmsg* nlmsg, int sock, const char* name,
                            const char* slave1, const char* slave2)
{
  netlink_add_device_impl(nlmsg, "hsr", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_virt_wifi(struct nlmsg* nlmsg, int sock,
                                  const char* name, const char* link)
{
  netlink_add_device_impl(nlmsg, "virt_wifi", name);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_vlan(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* link, uint16_t id, uint16_t proto)
{
  netlink_add_device_impl(nlmsg, "vlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_VLAN_ID, &id, sizeof(id));
  netlink_attr(nlmsg, IFLA_VLAN_PROTOCOL, &proto, sizeof(proto));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_macvlan(struct nlmsg* nlmsg, int sock, const char* name,
                                const char* link)
{
  netlink_add_device_impl(nlmsg, "macvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  uint32_t mode = MACVLAN_MODE_BRIDGE;
  netlink_attr(nlmsg, IFLA_MACVLAN_MODE, &mode, sizeof(mode));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

#define IFLA_IPVLAN_FLAGS 2
#define IPVLAN_MODE_L3S 2
#undef IPVLAN_F_VEPA
#define IPVLAN_F_VEPA 2

static void netlink_add_ipvlan(struct nlmsg* nlmsg, int sock, const char* name,
                               const char* link, uint16_t mode, uint16_t flags)
{
  netlink_add_device_impl(nlmsg, "ipvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_IPVLAN_MODE, &mode, sizeof(mode));
  netlink_attr(nlmsg, IFLA_IPVLAN_FLAGS, &flags, sizeof(flags));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int netlink_add_addr(struct nlmsg* nlmsg, int sock, const char* dev,
                            const void* addr, int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(nlmsg, RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, IFA_LOCAL, addr, addrsize);
  netlink_attr(nlmsg, IFA_ADDRESS, addr, addrsize);
  return netlink_send(nlmsg, sock);
}

static void netlink_add_addr4(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(struct nlmsg* nlmsg, int sock, const char* name,
                              const void* addr, int addrsize, const void* mac,
                              int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(nlmsg, RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, NDA_DST, addr, addrsize);
  netlink_attr(nlmsg, NDA_LLADDR, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(&nlmsg, sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(&nlmsg, sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in_addr, sizeof(in_addr),
                    &macaddr, ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in6_addr, sizeof(in6_addr),
                    &macaddr, ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(&nlmsg, sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN,
                        NULL);
  close(sock);
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7
#define DEVLINK_ATTR_NETNS_FD 138

static int netlink_devlink_id_get(struct nlmsg* nlmsg, int sock)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int err, n;
  uint16_t id = 0;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err) {
    return -1;
  }
  attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                          NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0); /* recv ack */
  return id;
}

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  int sock;
  int id, err;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  err = netlink_send(&nlmsg, sock);
  if (err) {
  }
error:
  close(sock);
}

static struct nlmsg nlmsg2;

static void initialize_devlink_ports(const char* bus_name, const char* dev_name,
                                     const char* netdev_prefix)
{
  struct genlmsghdr genlhdr;
  int len, total_len, id, err, offset;
  uint16_t netdev_index;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  int rtsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (rtsock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len);
  if (err) {
    goto error;
  }
  offset = 0;
  netdev_index = 0;
  while ((len = netlink_next_msg(&nlmsg, offset, total_len)) != -1) {
    struct nlattr* attr = (struct nlattr*)(nlmsg.buf + offset + NLMSG_HDRLEN +
                                           NLMSG_ALIGN(sizeof(genlhdr)));
    for (; (char*)attr < nlmsg.buf + offset + len;
         attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
      if (attr->nla_type == DEVLINK_ATTR_NETDEV_NAME) {
        char* port_name;
        char netdev_name[IFNAMSIZ];
        port_name = (char*)(attr + 1);
        snprintf(netdev_name, sizeof(netdev_name), "%s%d", netdev_prefix,
                 netdev_index);
        netlink_device_change(&nlmsg2, rtsock, port_name, true, 0, 0, 0,
                              netdev_name);
        break;
      }
    }
    offset += len;
    netdev_index++;
  }
error:
  close(rtsock);
  close(sock);
}

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
  initialize_devlink_ports("pci", "0000:00:10.0", "netpci");
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa

static void netdevsim_add(unsigned int addr, unsigned int port_count)
{
  char buf[16];
  sprintf(buf, "%u %u", addr, port_count);
  if (write_file("/sys/bus/netdevsim/new_device", buf)) {
    snprintf(buf, sizeof(buf), "netdevsim%d", addr);
    initialize_devlink_ports("netdevsim", buf, "netdevsim");
  }
}
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
      {"xfrm", "xfrm0"},
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
      {"vxcan0", 0, true},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
      {"xfrm0", ETH_ALEN},
      {"veth0_virt_wifi", ETH_ALEN},
      {"veth1_virt_wifi", ETH_ALEN},
      {"virt_wifi0", ETH_ALEN},
      {"veth0_vlan", ETH_ALEN},
      {"veth1_vlan", ETH_ALEN},
      {"vlan0", ETH_ALEN},
      {"vlan1", ETH_ALEN},
      {"macvlan0", ETH_ALEN},
      {"macvlan1", ETH_ALEN},
      {"ipvlan0", ETH_ALEN},
      {"ipvlan1", ETH_ALEN},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(&nlmsg, sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(&nlmsg, sock, slave0, false, master, 0, 0, NULL);
    netlink_device_change(&nlmsg, sock, slave1, false, master, 0, 0, NULL);
  }
  netlink_device_change(&nlmsg, sock, "bridge_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "bridge_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(&nlmsg, sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(&nlmsg, sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(&nlmsg, sock, "hsr_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "hsr_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "veth0_virt_wifi", "veth1_virt_wifi");
  netlink_add_virt_wifi(&nlmsg, sock, "virt_wifi0", "veth1_virt_wifi");
  netlink_add_veth(&nlmsg, sock, "veth0_vlan", "veth1_vlan");
  netlink_add_vlan(&nlmsg, sock, "vlan0", "veth0_vlan", 0, htons(ETH_P_8021Q));
  netlink_add_vlan(&nlmsg, sock, "vlan1", "veth0_vlan", 1, htons(ETH_P_8021AD));
  netlink_add_macvlan(&nlmsg, sock, "macvlan0", "veth1_vlan");
  netlink_add_macvlan(&nlmsg, sock, "macvlan1", "veth1_vlan");
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan0", "veth0_vlan", IPVLAN_MODE_L2, 0);
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan1", "veth0_vlan", IPVLAN_MODE_L3S,
                     IPVLAN_F_VEPA);
  netdevsim_add((int)procid, 4);
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(&nlmsg, sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(&nlmsg, sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(&nlmsg, sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize, NULL);
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
      {"nr", 7, true},
      {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(&nlmsg, sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(&nlmsg, sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(&nlmsg, sock, dev, !devtypes[i].noup, 0, &macaddr,
                          macsize, NULL);
  }
  close(sock);
}

#define MAX_FDS 30

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
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  if (dup2(netns, kInitNetNsFd) < 0)
    exit(1);
  close(netns);
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
  initialize_devlink_pci();
  initialize_tun();
  initialize_netdevices();
  loop();
  exit(1);
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
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

static void loop(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 11; call++) {
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
  close_fds();
  if (!collide) {
    collide = 1;
    goto again;
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    syscall(__NR_sendmsg, -1, 0ul, 0x4000000ul);
    break;
  case 3:
    NONFAILING(*(uint64_t*)0x20001080 = 0);
    NONFAILING(*(uint32_t*)0x20001088 = 0);
    NONFAILING(*(uint64_t*)0x20001090 = 0x20000080);
    NONFAILING(*(uint64_t*)0x20000080 = 0x20000000);
    NONFAILING(*(uint32_t*)0x20000000 = 0x58);
    NONFAILING(*(uint8_t*)0x20000004 = 2);
    NONFAILING(*(uint8_t*)0x20000005 = 6);
    NONFAILING(*(uint16_t*)0x20000006 = 1);
    NONFAILING(*(uint32_t*)0x20000008 = 0);
    NONFAILING(*(uint32_t*)0x2000000c = 0);
    NONFAILING(*(uint8_t*)0x20000010 = 0);
    NONFAILING(*(uint8_t*)0x20000011 = 0);
    NONFAILING(*(uint16_t*)0x20000012 = htobe16(0));
    NONFAILING(*(uint16_t*)0x20000014 = 0x12);
    NONFAILING(*(uint16_t*)0x20000016 = 3);
    NONFAILING(memcpy((void*)0x20000018, "hash:net,port\000", 14));
    NONFAILING(*(uint16_t*)0x20000028 = 5);
    NONFAILING(*(uint16_t*)0x2000002a = 4);
    NONFAILING(*(uint8_t*)0x2000002c = 0);
    NONFAILING(*(uint16_t*)0x20000030 = 9);
    NONFAILING(*(uint16_t*)0x20000032 = 2);
    NONFAILING(memcpy((void*)0x20000034, "syz0\000", 5));
    NONFAILING(*(uint16_t*)0x2000003c = 0xc);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000003e, 7, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000003f, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000003f, 1, 7, 1));
    NONFAILING(*(uint16_t*)0x20000040 = 8);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000042, 6, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000043, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000043, 0, 7, 1));
    NONFAILING(*(uint32_t*)0x20000044 = htobe32(0x7a));
    NONFAILING(*(uint16_t*)0x20000048 = 5);
    NONFAILING(*(uint16_t*)0x2000004a = 5);
    NONFAILING(*(uint8_t*)0x2000004c = 0xa);
    NONFAILING(*(uint16_t*)0x20000050 = 5);
    NONFAILING(*(uint16_t*)0x20000052 = 1);
    NONFAILING(*(uint8_t*)0x20000054 = 7);
    NONFAILING(*(uint64_t*)0x20000088 = 0x58);
    NONFAILING(*(uint64_t*)0x20001098 = 1);
    NONFAILING(*(uint64_t*)0x200010a0 = 0);
    NONFAILING(*(uint64_t*)0x200010a8 = 0);
    NONFAILING(*(uint32_t*)0x200010b0 = 0);
    syscall(__NR_sendmsg, r[0], 0x20001080ul, 0ul);
    break;
  case 4:
    syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    break;
  case 5:
    syscall(__NR_sendmsg, r[1], 0ul, 0x40ul);
    break;
  case 6:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[2] = res;
    break;
  case 7:
    syscall(__NR_sendmsg, r[2], 0ul, 0x850ul);
    break;
  case 8:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[3] = res;
    break;
  case 9:
    NONFAILING(*(uint64_t*)0x20000340 = 0);
    NONFAILING(*(uint32_t*)0x20000348 = 0);
    NONFAILING(*(uint64_t*)0x20000350 = 0);
    NONFAILING(*(uint64_t*)0x20000358 = 1);
    NONFAILING(*(uint64_t*)0x20000360 = 0);
    NONFAILING(*(uint64_t*)0x20000368 = 0);
    NONFAILING(*(uint32_t*)0x20000370 = 0);
    syscall(__NR_sendmsg, r[3], 0x20000340ul, 0ul);
    break;
  case 10:
    NONFAILING(*(uint64_t*)0x20000340 = 0);
    NONFAILING(*(uint32_t*)0x20000348 = 0);
    NONFAILING(*(uint64_t*)0x20000350 = 0x20000280);
    NONFAILING(*(uint64_t*)0x20000280 = 0x200001c0);
    NONFAILING(*(uint32_t*)0x200001c0 = 0x64);
    NONFAILING(*(uint8_t*)0x200001c4 = 0xb);
    NONFAILING(*(uint8_t*)0x200001c5 = 6);
    NONFAILING(*(uint16_t*)0x200001c6 = 0x201);
    NONFAILING(*(uint32_t*)0x200001c8 = 0);
    NONFAILING(*(uint32_t*)0x200001cc = 0);
    NONFAILING(*(uint8_t*)0x200001d0 = 0);
    NONFAILING(*(uint8_t*)0x200001d1 = 0);
    NONFAILING(*(uint16_t*)0x200001d2 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200001d4 = 9);
    NONFAILING(*(uint16_t*)0x200001d6 = 2);
    NONFAILING(memcpy((void*)0x200001d8, "syz0\000", 5));
    NONFAILING(*(uint16_t*)0x200001e0 = 5);
    NONFAILING(*(uint16_t*)0x200001e2 = 1);
    NONFAILING(*(uint8_t*)0x200001e4 = 7);
    NONFAILING(*(uint16_t*)0x200001e8 = 0x3c);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ea, 7, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001eb, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001eb, 1, 7, 1));
    NONFAILING(*(uint16_t*)0x200001ec = 6);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ee, 0x1d, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ef, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ef, 0, 7, 1));
    NONFAILING(*(uint16_t*)0x200001f0 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200001f4 = 6);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001f6, 4, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001f7, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001f7, 0, 7, 1));
    NONFAILING(*(uint16_t*)0x200001f8 = htobe16(0));
    NONFAILING(*(uint16_t*)0x200001fc = 6);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001fe, 5, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ff, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x200001ff, 0, 7, 1));
    NONFAILING(*(uint16_t*)0x20000200 = htobe16(0));
    NONFAILING(*(uint16_t*)0x20000204 = 0x18);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000206, 1, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000207, 0, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x20000207, 1, 7, 1));
    NONFAILING(*(uint16_t*)0x20000208 = 0x14);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000020a, 2, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000020b, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000020b, 0, 7, 1));
    NONFAILING(memcpy(
        (void*)0x2000020c,
        "\x16\x15\xed\x1c\x3f\x30\xcb\x1f\x51\xf9\xc4\x67\x63\xf6\x4e\x9f",
        16));
    NONFAILING(*(uint16_t*)0x2000021c = 8);
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000021e, 9, 0, 14));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000021f, 1, 6, 1));
    NONFAILING(STORE_BY_BITMASK(uint16_t, , 0x2000021f, 0, 7, 1));
    NONFAILING(*(uint32_t*)0x20000220 = htobe32(0));
    NONFAILING(*(uint64_t*)0x20000288 = 0x64);
    NONFAILING(*(uint64_t*)0x20000358 = 1);
    NONFAILING(*(uint64_t*)0x20000360 = 0);
    NONFAILING(*(uint64_t*)0x20000368 = 0);
    NONFAILING(*(uint32_t*)0x20000370 = 0);
    syscall(__NR_sendmsg, -1, 0x20000340ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  setup_binfmt_misc();
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
