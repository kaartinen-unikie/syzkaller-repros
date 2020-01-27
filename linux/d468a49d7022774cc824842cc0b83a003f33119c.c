// kernel BUG at kernel/time/timer.c:LINE!
// https://syzkaller.appspot.com/bug?id=d468a49d7022774cc824842cc0b83a003f33119c
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
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

static long syz_open_pts(volatile long a0, volatile long a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
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
  for (call = 0; call < 7; call++) {
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
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
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
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    NONFAILING(memcpy(
        (void*)0x20000380,
        "\x37\xbc\x32\x6f\x97\x6f\x16\x41\x5d\x97\xbc\xa9\x00\xb5\x12\x6b\xf0"
        "\x66\x69\x33\x2a\xb4\x47\x1c\x45\xff\xa8\x87\x19\x41\x0a\xc8\x95\xac"
        "\x3f\x2e\x32\xd0\x64\xf1\x10\xc6\x4b\x15\x13\xf7\x98\x61\x85\x6b\x72"
        "\x96\xdf\x63\xf5\x4f\xd1\x9c\xa2\xbd\xb1\xf5\x70\xbe\x70\xb1\x5f\x7e"
        "\xed\x29\x92\x2e\xd4\x92\x08\xe7\xd3\xf8\x1f\xf8\x7a\xa6\x45\xe2\x92"
        "\xdd\x4c\x4f\xc5\x6c\xda\x60\x89\xaa\x98\x13\x90\x7d\xfa\x99\x27\x67"
        "\x1a\xa5\xa7\x5c\x86\x07\xf6\x41\x95\x52\x58\x4b\x37\x81\x6a\xaa\x89"
        "\xdb\xdb\x21\xf8\x0d\xd9\x0c\xdb\x3d\x4f\x5e\xec\x70\xab\x48\xbd\x61"
        "\x19\xa0\xda\x55\x36\x47\x43\x92\x7f\xbf\x3e\x8f\x08\x6a\xa4\x9f\xb1"
        "\xa6\xe0\xb9\x44\x44\x83\x72\x0e\x46\xe8\x9e\xa4\x90\x4e\xd4\xf8\xa3"
        "\x06\xbd\x6d\x17\xb8\xd3\x76\x91\x52\xe7\x91\x94\x58\xf2\x48\xfc\x4b"
        "\x72\xda\x73\xf0\x09\xdd\xf9\x9d\x39\x26\x29\xdd\x25\xde\xc8\xf2\x6c"
        "\x97\xed\x12\x17\x45\x4b\xc0\xa7\x5c\x35\x70\xa9\x62\x89\xeb\x1d\x8b"
        "\x9a\x22\x24\xff\xc4\x9c\x15\xa1\xca\xc0\xb5\xee\xb5\x78\x22\xc5\x4f"
        "\x2c\x54\x59\x3e\x3c\x4c\x8e\xf3\xaf\xda\x98\x19\x0b\x1e\x5c\xdf\xd1"
        "\x4e\xee\xed\xa0\x1d\x2e\x91\x56\xa2\x48\xe0\xa5\xe9\x0c\x73\x1a\x8a"
        "\x34\x29\x09\x31\xf8\xed\x0e\xfd\x34\xb6\xa2\x48\xb6\xfa\x99\xaf\x74"
        "\xf9\xe7\x8d\xe7\x46\xa2\xb5\xe3\x46\xfc\xa6\xf5\xdf\xdb\x54\xcc\xaa"
        "\x6f\x2e\xcd\x41\x0d\x0f\x47\x02\x59\xc4\xd7\x12\xf1\xf1\xe1\x1c\x92"
        "\x96\x7a\xb6\xf9\x55\x66\xa2\x24\x1e\x5d\x3f\x31\x7a\x36\x59\x9a\x36"
        "\xeb\xeb\x26\x24\x75\x80\x60\x7f\x3a\xcc\x07\xe9\x00\xaf\xf7\xc4\xee"
        "\x43\x75\x66\xe2\x86\xf4\xed\x09\x3e\xa1\xd6\xec\xd7\x3e\x8b\x8a\x65"
        "\x0e\x98\x13\xa9\x97\xc2\xad\xfb\x9e\x04\xd0\x76\xb8\x4a\xa7\x26\x11"
        "\xcb\xfe\xf2\x79\x67\x99\xf5\x3c\xfc\x34\x3e\xad\xaf\x42\xae\x00\x1d"
        "\xe3\x6f\xb5\x89\xd3\x2c\x9d\x65\x9d\x32\xb1\xdb\xe3\xc4\x85\x35\xad"
        "\x28\xf6\xd9\x3a\x85\x62\x75\xbf\xcf\x80\x88\x9e\xb4\x40\x24\xcc\x9e"
        "\x86\x56\xbf\x32\xb7\x51\xa0\x9f\xd0\x2f\x36\x4e\x90\x35\x25\x15\xc6"
        "\x23\xfd\x71\x34\xba\xf0\x0c\x7f\x6b\x78\x6d\xf3\xc2\x81\xfe\xfe\x13"
        "\xf7\xa2\xbc\xae\x83\xf2\x39\xe3\xe2\x53\x4c\xb2\x65\x22\xaf\x9e\xed"
        "\xaf\x74\xea\xa9\x11\x24\x0c\xcb\x0c\xad\x8a\x59\xe5\x6e\x9d\x6b\xee"
        "\xbc\x1f\x06\xb8\xe5\xda\xfd\x43\x79\x5d\xfc\xef\x38\xef\xea\xd3\xe5"
        "\x72\x29\x31\x0f\xaa\x19\x1e\x80\x21\x7b\x12\x29\x08\x6e\x46\x5b\xeb"
        "\x97\x4d\x1b\xfc\xcc\x18\xd0\xdc\xdf\xae\x8f\x49\x76\x32\x14\x1a\x20"
        "\xa8\x0b\x5a\xa3\x68\xa8\xdf\xfd\x71\xae\x73\x96\xcf\xd2\xcb\xc6\x26"
        "\x6b\xc6\xee\xf7\x3e\x83\xd4\x4c\x3e\xbf\x72\xf8\x24\x63\xc6\x1f\x6b"
        "\x2b\x1e\x5e\xd8\xf1\xb0\x6d\x02\x32\x7a\x33\x61\x69\x36\x69\xc8\x31"
        "\xa5\x01\x2d\x83\xbb\x31\x29\x61\x34\x4d\x37\xf7\x50\x69\xe3\x21\x78"
        "\x35\x60\xba\x65\x8b\x0e\x0e\x2f\x14\x06\x36\x83\x2b\x06\x70\x5b\xee"
        "\x56\x1e\x5f\x5f\xbe\xaa\xba\x45\x5c\x23\x6f\x43\x0d\x98\xae\x9d\x76"
        "\xe2\x68\xa0\xaf\x2b\x5d\x09\xb3\xad\xae\x94\xbc\x9b\xd8\xd5\xdd\xd4"
        "\xb0\xeb\x56\x88\x82\xf2\x24\x17\x0b\xe2\x14\x44\xb9\x2f\x14\x4b\xa1"
        "\x88\x89\x80\x84\xd2\x99\xf3\xf3\xb9\x69\xa8\xf0\xb8\x5a\xe8\x81\x52"
        "\xa5\x7c\x97\x54\x85\x10\x9c\xd8\x21\x3a\xd9\xbf\x2a\x6c\x56\x1d\x22"
        "\x9f\x2f\xd8\xae\x83\x18\xdc\x7d\x2a\xb4\x35\xcf\x0c\x63\x5c\x53\x34"
        "\x5a\x3a\x35\x3b\x6e\xc2\x82\x42\x4e\x86\xf2\xfe\x59\x45\xdc\xf3\xe3"
        "\x71\x21\x83\x31\x91\xaf\xc9\x9d\x1a\xe1\xa5\x2e\x60\xa3\xdb\xb5\x23"
        "\xc1\x33\xb1\x58\x9f\x76\x6e\x55\x35\xc2\x5a\x47\x61\x13\x63\xdb\xdc"
        "\xc2\x6c\xc7\x6b\x35\xd3\xf6\x43\x5e\xe9\x6d\x7e\xc0\xf5\x81\x5d\xa1"
        "\x36\x57\xf4\x77\x32\x3d\xc2\x32\x65\xca\xcb\x1b\xe4\x98\x56\x13\xdf"
        "\xbc\xbe\xac\x24\x63\x1e\x96\x94\x76\x94\x73\xc3\xc2\x23\x2c\x0a\x03"
        "\x72\xf9\xb9\x9d\xd2\xd6\x9c\xe0\x6b\x9a\x9f\x43\x46\x13\x50\x66\x3d"
        "\x9a\xbd\x74\x01\x21\xd2\x55\x56\x0f\xa5\x94\xbd\x95\x93\x3f\x9a\xc0"
        "\x1f\xb4\xff\x61\x39\x5e\x79\xe0\x5c\xce\xfd\xac\xf1\x37\xf7\xcb\x68"
        "\x1a\xeb\x43\x01\x62\x32\x03\xee\x0a\x69\xd3\xe1\xf2\x5a\xd4\xb1\x69"
        "\xfe\x9e\x21\xb9\x23\x96\xbd\xab\x37\x3e\x3f\x02\xd9\x87\x4f\xa7\xf2"
        "\xdc\x84\x8d\xbe\x0d\x6a\x8f\x84\xe9\x06\x48\xca\xa8\x87\xfe\xf9\x52"
        "\x54\x10\x6b\x91\x55\x89\xdc\x25\xc7\x02\xa6\x02\x87\x2e\x95\xf7\xd1"
        "\x96\x75\x93\xbe\x25\xa9\x85\xcb\x31\x45\xf3\x0e\xfe\x51\x6d\x91\x19"
        "\x15\x52\x77\x35\x59\x55\x86\x55\x22\xc8\x60\xeb\x8b\x29\x5f\xd0\x85"
        "\xbb\x95\x6c\xd5\xe3\xc2\xc6\x8b\xbf\xd9\xb4\x53\xb2\x7b\x18\xe5\x26"
        "\x61\xf7\xa0\xd2",
        1024));
    syscall(__NR_ioctl, -1, 0x5000aea5ul, 0x20000380ul);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000080, "/dev/ptmx\000", 10));
    res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 6ul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000040 = 0);
    NONFAILING(*(uint32_t*)0x20000044 = 0);
    NONFAILING(*(uint32_t*)0x20000048 = 0);
    NONFAILING(*(uint32_t*)0x2000004c = 0);
    NONFAILING(*(uint8_t*)0x20000050 = 0);
    NONFAILING(*(uint8_t*)0x20000051 = 0);
    NONFAILING(*(uint8_t*)0x20000052 = 0);
    NONFAILING(*(uint8_t*)0x20000053 = 0);
    NONFAILING(*(uint32_t*)0x20000054 = 0);
    NONFAILING(*(uint32_t*)0x20000058 = 0);
    NONFAILING(*(uint32_t*)0x2000005c = 0);
    NONFAILING(*(uint32_t*)0x20000060 = 0);
    syscall(__NR_ioctl, r[0], 0x40045431ul, 0x20000040ul);
    break;
  case 3:
    res = syz_open_pts(r[0], 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x200000c0 = 7);
    syscall(__NR_ioctl, r[1], 0x5423ul, 0x200000c0ul);
    break;
  case 5:
    syscall(__NR_write, r[0], 0x200000c0ul, 0xffa8ul);
    break;
  case 6:
    NONFAILING(*(uint32_t*)0x20000000 = 0xec);
    NONFAILING(*(uint32_t*)0x20000004 = 0);
    NONFAILING(*(uint32_t*)0x20000008 = 0);
    NONFAILING(*(uint32_t*)0x2000000c = 0);
    NONFAILING(*(uint8_t*)0x20000010 = 0);
    NONFAILING(*(uint8_t*)0x20000011 = 0);
    NONFAILING(*(uint8_t*)0x20000012 = 0);
    NONFAILING(*(uint8_t*)0x20000013 = 0);
    NONFAILING(*(uint32_t*)0x20000014 = 0);
    NONFAILING(*(uint32_t*)0x20000018 = 0);
    NONFAILING(*(uint32_t*)0x2000001c = 0);
    NONFAILING(*(uint32_t*)0x20000020 = 0);
    syscall(__NR_ioctl, r[1], 0x5412ul, 0x20000000ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
