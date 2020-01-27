// KASAN: slab-out-of-bounds Read in bpf_skb_change_head
// https://syzkaller.appspot.com/bug?id=fd6bf14a1604a952f31023f3903bd6a2d9a9482c
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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
#define DEVLINK_ATTR_NETNS_FD 137

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
  int id;
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
  netlink_send(&nlmsg, sock);
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  *(uint32_t*)0x20000200 = 0xc;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000e80;
  memcpy(
      (void*)0x20000e80,
      "\xb7\x02\x00\x00\x03\x07\x00\x08\xbf\xa3\x00\x00\x00\x00\x00\x00\x07\x01"
      "\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4\xf0\xff"
      "\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05\x00\x00\x00"
      "\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00\xf5\x06\x7d\x60"
      "\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x18\x00\x00\x00\x85\x00"
      "\x00\x00\x2b\x00\x00\x00\xb7\x00\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00"
      "\x00\x00\x00\x00\x0d\x7a\x28\x07\x3a\x41\x02\xe4\xae\xb5\x4f\x36\x63\x3e"
      "\x27\xc2\x79\x34\x1b\xf4\x89\x90\x3c\xfd\xb4\xc0\x5e\x96\xe3\x04\x6f\x04"
      "\xe7\x79\x69\xbe\x06\x34\x0e\xe6\xaf\x0d\x49\x91\x0d\x06\x35\x18\x59\x8e"
      "\x7e\x29\x0b\x39\xa6\xf2\xfc\x2a\x5e\x01\x9b\xc6\xb4\x56\x84\xf0\x02\xcf"
      "\x57\xbf\x88\x7e\x83\xfb\xb2\x21\x5b\x8a\x34\xe6\xbd\xc4\xdc\x1a\xf6\xd3"
      "\xc6\x95\x8d\xa4\xbd\xda\xc6\x02\xe0\x04\x8b\xec\x11\xe8\x74\x60\x2f\x06"
      "\x00\x00\x00\x2a\xf2\x1b\x75\x52\xf0\xa0\xcc\x85\xae\x28\x19\x93\xbf\xa2"
      "\x13\x9a\x3b\xb7\x55\xc1\xf1\xab\xd1\x96\x40\x07\x00\x00\x00\xe2\x7b\x21"
      "\x21\xa5\xf0\x3d\xff\x97\x87\xdf\xd6\xe7\x60\x8e\xb6\x38\xe6\xfc\x4e\x0b"
      "\xc6\xbd\x5f\xf3\x59\x28\xa5\xb5\xfa\x72\x30\x28\xeb\x6d\xdd\x35\xea\x79"
      "\xb9\x2b\xee\xb5\x22\x65\xa4\xb0\x4b\xa5\x0f\xd4\x6a\x50\x41\x16\xfb\x8e"
      "\xf8\x4c\x2b\xe0\x21\xa6\xc9\xda\xf3\xf2\x4a\xd7\xb9\x65\xaf\x8f\x52\x23"
      "\x71\xc9\xa4\x3c\x80\xee\x6b\x39\x7b\xea\x24\x7e\x2d\x71\x40\x90\xd4\x3f"
      "\xe6\x6b\xbf\x4f\x4f\xfe\x41\x0b\xd7\x01\xd0\x35\xf8\x67\xde\x2a\x20\x69"
      "\x50\x33\xd9\x1e\xe2\x38\xc8\x24\xe0\xb7\xa0\xae\xff\xb9\x84\x39\x47\xc3"
      "\xbc\x96\xe1\xf9\x5c\x24\x51\x68\xd2\xae\xed\x2e\x00\x46\x3d\x9c\x03\x9b"
      "\xfd\xd5\x87\x09\xe8\x98\xc7\xeb\xad\x73\xfc\x48\x67\x3c\x75\xc8\xb5\x0d"
      "\xb8\x52\x62\x1a\xd1\x9c\x85\x46\x22\xf7\xc7\xd7\x9e\xc3\xab\x44\x94\x35"
      "\x3b\x45\x8c\x71\x80\x21\xf7\x01\x44\x2b\xbc\x64\x56\xbf\x0c\x9f\x6b\x82"
      "\x22\x11\xeb\x1a\xa4\x88\xfc\xae\xbb\x10\x93\x82\xc7\xab\x6d\xb9\xb3\x6e"
      "\xf2\x4e\x46\x64\x94\xe7\xb8\x54\x9c\xc1\x39\xa7\x4b\x5a\xaa\xf2\x61\xf3"
      "\x5e\x83\x47\xea\xab\xe0\x1a\xfe\x21\xd7\xb7\xa9\x58\xe9\xcd\x04\xb5\xbf"
      "\xa5\xcf\x78\xdd\x2f\xa9\x58\xdb\xb6\x05\xcd\x1a\x99\x61\x33\x69\x18\x5b"
      "\x2b\xcf\xe0\xbf\x0e\x31\xc8\x3f\xdc\xb2\x54\xda\x7c\xd3\x95\x89\x83\xd0"
      "\x79\x8b\x45\x5c\x8c\x5a\x5a\x85\x65\xd8\x3d\x28\x43\x78\x95\x92\x9f\xa0"
      "\x89\x6c\xf4\x5e\xda\x77\xc4\xfe\xae\x29\xd9\x65\x68\xc4\x87\xa7\x4d\xd4"
      "\xf9\xe8\x46\x5f\x09\x81\x8c\x12\xdf\xc9\x3f\xdb\x09\xa1\xd0\x4a\x08\x7d"
      "\x3b\xf2\x19\xef\xab\x76\x4d\x3c\xd6\x76\xf1\x01\xd3\xb6\xc9\x17\x7c\x57"
      "\x34\x0b\x24\x5f\x15\xaf\x47\x2f\x1b\x83\x70\x81\x96\x9d\xb2\xd5\x8c\xea"
      "\xb0\xb4\x32\x00\x5a\x86\xe6\xc4\x50\xae\x42\x41\xc0\x8a\x64\x69\xac\x65"
      "\xaf\x64\x73\x7a\xf9\x61\xcc\x90\x13\x2c\xd2\xcc\xb6\xd7\xad\xf6\x38\x46"
      "\xaf\x15\x54\xcf\xca\xa4\xeb\x35\x71\x42\xa5\xb5\x25\xa1\x8d\x9d\x88\xd4"
      "\x25\x20\xc0\x90\x3b\xc4\x44\xdc\x0e\xec\x4d\x26\xe5\x86\xef\xf7\xc4\x32"
      "\xcd\x12\xbe\x4c\x79\x6f\x9e\xbe\x44\x81\xf9\x71\xc5\x2a\x8c\xc7\xb0\xed"
      "\xab\x7a\xe8\x72\xc4\x83\x2b\x61\xa5\x49\x63\x18\x5b\xb7\xcd\x1f\xb7\x8f"
      "\xf9\xe1\x94\xe1\x7c\x65\xac\xd8\xd0\x15\x44\x3a\x21\x81\x5b\xf5\x6f\x08"
      "\xf1\xd2\x29\x3c\xed\x27\x88\x99\xa9\x72\x05\x63\x80\xb4\xf3\x32\xd3\x97"
      "\x37\xb1\x49\xe1\x6b\xb0\x98\xc0\xef\x5d\x36\x2c\xde\x82\x50\x9e\xad\x89"
      "\x32\x86\x9d\x3d\x9b\x52\x7c\x9e\x53\xc5\xc4\x1f\x4d\x71\x1c\x8b\x68\xa0"
      "\xa1\x6c\x01\x8c\xe2\xa2\x48\x32\xcd\xdb\x18\xbe\xc4\x14\xd7\x8f\x88\x6d"
      "\x1e\xbf\x73\x00\x38\x4b\x13\xef\xef\xa0\x3a\xd5\x5f\xba\xef\xc0\x26\xea"
      "\xa5\x71\xd5\xde\xb4\x49\x5e\xbd\x6f\xc7\xd4\x57\xaf\x00\x8e\x74\x85\x55"
      "\x62\xbb\xb4\xf2\x55\xc5\xa6\x81\x9a\xd2\xb6\x07\xda\xd7\x32\x77\xce\x27"
      "\x56\xe0\xcc\xfd\xd7\x40\x3d\x13\x93\x8b\x4b\xae\x98\xb9\x8b\x62\x1c\x30"
      "\x9b\x18\xdf\x38\xa1\xcd\x77\x9b\xf4\x79\x09\x6c\x09\x85\x1c\x9a\xa5\x3f"
      "\x91\x04\x1f\x2e\x17\x65\x21\x20\x94\xff\xb9\x7e\xbf\x97\x2e\x48\x87\x1f"
      "\x76\xb5\xe0\xcd\xfd\xa8\x1d\xf2\x8e\x5f\x72\x1f\x8a\x63\xc0\xfb\xd8\xac"
      "\x99\x2d\x75\x35\x45\x2f\x7a\xf6\x8f\x5a\xe2\x20\x20\xaf\xd0\xa6\x03\x6d"
      "\x51\xcd\xa9\x8f\xef\x44\xb6\x91\x39\x31\x58\x32\xd4\x9f\xef\x81\x19\x8f"
      "\x15\xeb\xe3\xa7\x54\x8b\xa4\x64\x51\xd9\x3d\xa0\x3b\x99\xbe\xcd\x85\x85"
      "\x1e\x71\x57\xd7\xe2\x3b\x28\x06\x0f\xc4\xfa\x98\xd8\x7d\xbc\x5c\xec\x5a"
      "\x4e\x06\x53\x72\x75\xa8\xfb\x1d\x41\x94\x08\x24\x8d\xdc\x98\x11\x3e\x88"
      "\x49\x91\x72\x6b\xf0\x54\x01\x00\x00\x00\xa5\xe0\xf6\xbc\x37\x3e\x43\x8d"
      "\xf8\x50\x81\x98\xcb\x61\xa4\x64\x06\x84\xd0\x2f\x9f\x97\xce\xde\xe6\x6f"
      "\x20\x98\xea\xa9\xee\x8d\xed\xc0\x00\x37\x31\xc5\x11\xef\xcf\xc7\x8b\xd2"
      "\x56\x5a\xdf\xa0\x13\x0b\x48\x1b\x2c\xa8\xfe\x13\x22\x1b\x4e\x99\xd9\x4d"
      "\xed\x40\x78\x7e\x07\xe6\x26\x4b\xb2\x8e\xab\x38\x3d\x90\xd4\x19\xb1\x7d"
      "\x36\xe6\xf7\x4d\x73\xbd\xce\x1f\x7d\x81\x60\xd9\x06\x7f\x63\x5f\xd5\x12"
      "\x36\x52\xf6\xfa\xea\x7a\x3a\x6e\xfe\x11\xf1\xdb\x1c\x31\x74\x47\x6b\x47"
      "\x63\x07\x3a\xbb\x6b\x3a\x05\x29\x56\x6a\xf5\x1e\xb3\x1a\x1a\x82\xf2\x95"
      "\x59\xdb\x29\x38\xcf\xc5\x89\x5c\x61\x52\xb2\xf9\x34\x74\xfb\xfc\xb3\x39"
      "\xcb\xa7\xd0\x6e\xc6\x5a\xfb\x9f\xd5\xb4\xfc",
      1217);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller\000", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = -1;
  *(uint32_t*)0x2000024c = 8;
  *(uint64_t*)0x20000250 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0x10;
  *(uint64_t*)0x20000260 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000268 = 0;
  res = syscall(__NR_bpf, 5ul, 0x20000200ul, 0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200004c0 = r[0];
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 0xe;
  *(uint32_t*)0x200004cc = 0xfffffee4;
  *(uint64_t*)0x200004d0 = 0x20000040;
  memcpy((void*)0x20000040,
         "\x1c\x04\xff\x01\xda\x6d\xa9\x72\x5f\x90\xc5\xa0\x6d\x0f", 14);
  *(uint64_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004e0 = 0xf0;
  *(uint32_t*)0x200004e4 = 0;
  *(uint32_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004ec = 0;
  *(uint64_t*)0x200004f0 = 0x20000000;
  *(uint64_t*)0x200004f8 = 0x20000000;
  syscall(__NR_bpf, 0xaul, 0x200004c0ul, 0x28ul);
  return 0;
}
