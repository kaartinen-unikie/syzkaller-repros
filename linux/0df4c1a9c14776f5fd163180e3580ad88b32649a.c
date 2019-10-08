// WARNING in vhost_dev_cleanup
// https://syzkaller.appspot.com/bug?id=0df4c1a9c14776f5fd163180e3580ad88b32649a
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[4];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0x27, 3, 0);
    break;
  case 2:
    if (syscall(__NR_ioctl, r[0], 0x8904, 0x20878ffc) != -1)
      r[1] = *(uint32_t*)0x20878ffc;
    break;
  case 3:
    *(uint32_t*)0x20063000 = 0x20071026;
    *(uint32_t*)0x20063004 = r[1];
    *(uint32_t*)0x2035bfe8 = 0x3ff;
    *(uint32_t*)0x2035bfec = 8;
    *(uint32_t*)0x2035bff0 = 0xc61;
    *(uint32_t*)0x2035bff4 = 1;
    *(uint32_t*)0x2035bff8 = 9;
    *(uint32_t*)0x2035bffc = 4;
    syscall(__NR_capset, 0x20063000, 0x2035bfe8);
    break;
  case 4:
    memcpy((void*)0x20a2c000, "/dev/vhost-net", 15);
    r[2] = syscall(__NR_openat, 0xffffffffffffff9c, 0x20a2c000, 2, 0);
    break;
  case 5:
    *(uint64_t*)0x201e3000 = 0;
    syscall(__NR_ioctl, r[2], 0xaf01, 0x201e3000);
    break;
  case 6:
    r[3] = syscall(__NR_eventfd, 0);
    break;
  case 7:
    *(uint32_t*)0x20857ff8 = 0;
    *(uint32_t*)0x20857ffc = r[3];
    syscall(__NR_ioctl, r[2], 0x4008af20, 0x20857ff8);
    break;
  case 8:
    memcpy((void*)0x20bfdff5, "/dev/midi#", 11);
    syz_open_dev(0x20bfdff5, 1, 0x80);
    break;
  case 9:
    syscall(__NR_ioctl, r[2], 0xaf02, 0);
    break;
  case 10:
    *(uint64_t*)0x201d3ff8 = 0;
    syscall(__NR_write, r[3], 0x201d3ff8, 8);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(11);
  collide = 1;
  execute(11);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}