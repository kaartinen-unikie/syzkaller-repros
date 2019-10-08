// general protection fault in ucma_accept
// https://syzkaller.appspot.com/bug?id=0036bceb487e54824818b8d6e9e0b7284af77e97
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffff};
uint64_t procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x200001c0, "/dev/infiniband/rdma_cm", 24);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200001c0, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20000500 = 0;
    *(uint16_t*)0x20000504 = 0x18;
    *(uint16_t*)0x20000506 = 0xfa00;
    *(uint64_t*)0x20000508 = 1;
    *(uint64_t*)0x20000510 = 0x200004c0;
    *(uint16_t*)0x20000518 = 0x13f;
    *(uint8_t*)0x2000051a = 8;
    *(uint8_t*)0x2000051b = 0;
    *(uint8_t*)0x2000051c = 0;
    *(uint8_t*)0x2000051d = 0;
    *(uint8_t*)0x2000051e = 0;
    *(uint8_t*)0x2000051f = 0;
    res = syscall(__NR_write, r[0], 0x20000500, 0x20);
    if (res != -1)
      r[1] = *(uint32_t*)0x200004c0;
    break;
  case 2:
    *(uint32_t*)0x20000780 = 8;
    *(uint16_t*)0x20000784 = 0x120;
    *(uint16_t*)0x20000786 = 0xfa00;
    *(uint64_t*)0x20000788 = 4;
    *(uint32_t*)0x20000790 = 7;
    *(uint32_t*)0x20000794 = 5;
    memcpy((void*)0x20000798,
           "\xc0\xc3\x75\xca\xc6\x1e\x68\x62\xf1\x97\xeb\x0f\xe1\x45\xd6\x16"
           "\x30\x54\x42\x3a\xd9\x4f\xb0\xd7\xcc\x84\x98\x74\xd0\x55\x5a\xd3"
           "\x8b\x01\x57\xbc\xec\xa7\x4a\x01\xca\x68\x21\x48\x73\xb7\xc4\x6a"
           "\xa2\xd4\x6e\x1b\x5b\xbf\xd2\x8a\x37\xe1\x78\x55\xdb\x62\x3b\xba"
           "\xf8\xd1\xea\x16\xc5\x00\x32\xb3\xb4\x1b\xf2\x1f\x80\xbb\xe6\x59"
           "\xc4\x7d\xdf\x19\x9f\x7f\x34\x61\x73\xc6\xc6\x8d\x21\xc5\x6e\x8f"
           "\xd9\xe6\xaa\x11\x44\x46\xcf\xab\x37\xd5\x66\x2b\x53\x08\xd4\xee"
           "\xe6\x32\x07\x37\xc7\x6d\x4b\x72\xeb\xab\x0a\x85\x59\xeb\xab\xf1"
           "\x44\x25\x25\x84\x54\x3e\x9c\xc4\x37\x2b\x55\x1a\xb5\x3a\x19\xde"
           "\x5a\x30\x53\xd3\x8f\xe4\xce\x92\xbf\x56\x94\x16\xd4\xa0\xa4\xbb"
           "\x80\x69\xbd\x07\xed\x74\xb1\x7a\x79\x26\xfd\x1f\x13\xf9\x21\xe8"
           "\x92\x3f\x08\xd2\xab\x35\x0f\x0b\x88\x91\x61\x77\xc5\xb1\xdb\x4a"
           "\x56\x9f\x67\x67\xc7\xe9\xe6\x18\x28\x41\xe0\xb0\xbe\x54\x47\xc9"
           "\xe2\xe2\xa5\x74\x7c\x61\x9f\xc5\xc8\xf9\x5a\x89\x26\x9c\x7a\xad"
           "\x7d\x1b\xbc\x7f\x06\xb0\x1f\xac\x8e\x14\x47\x2f\xd5\x8f\x1e\x4b"
           "\x4f\xfa\x7c\xc8\xaa\x66\x8b\x3f\xc9\x77\xe1\xc8\xe4\x42\xb7\x79",
           256);
    *(uint8_t*)0x20000898 = 0xb3;
    *(uint8_t*)0x20000899 = 1;
    *(uint8_t*)0x2000089a = 6;
    *(uint8_t*)0x2000089b = 6;
    *(uint8_t*)0x2000089c = 6;
    *(uint8_t*)0x2000089d = 0x20;
    *(uint8_t*)0x2000089e = 0x31;
    *(uint8_t*)0x2000089f = 1;
    *(uint32_t*)0x200008a0 = r[1];
    *(uint32_t*)0x200008a4 = 0;
    syscall(__NR_write, r[0], 0x20000780, 0x128);
    break;
  case 3:
    syscall(__NR_close, r[0]);
    break;
  }
}

void test()
{
  execute(4);
  collide = 1;
  execute(4);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
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