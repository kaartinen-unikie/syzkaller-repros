// INFO: task hung in jbd2_journal_commit_transaction
// https://syzkaller.appspot.com/bug?id=b8afb3d95297388287141f3721edb7a94b0280c5
// status:dup
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000180,
           "\x73\x74\x61\x74\x09\xc0\xc2\xfe\xbc\xf9\xdf\x2d\xea\xc8\xc1\x77"
           "\xff\x17\x12\x48\xe9\x11\x93\x51\x30\x49\xf8\x31\x55\x0d\x6f\x7d"
           "\xe6\x6c\xf6\x37\xbd\xbf\x13\x11\x92\x0c\x8a\x26\xed\xa4\xdc\xc3"
           "\x78\x3f\x9d\xb5\x11\x6b\x34\xd3\x1b\x05\x12\xa5\x60\x8a\xaf\xf0"
           "\x1e\x79\x52\x34\x0c\xd6\xfd\x00\x00\x00\x00",
           75);
    res = syscall(__NR_openat, 0xffffff9c, 0x20000180, 0x275a, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x200005c0, "\xf7", 1);
    syscall(__NR_pwrite64, r[0], 0x200005c0, 1, 0);
    break;
  case 2:
    memcpy((void*)0x20000240, "memory.stat", 12);
    res = syscall(__NR_openat, 0xffffff9c, 0x20000240, 0x275a, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    sprintf((char*)0x20000100, "0x%016llx", (long long)0);
    syscall(__NR_write, r[1], 0x20000100, 0x12);
    break;
  case 4:
    *(uint16_t*)0x20000080 = 0;
    *(uint16_t*)0x20000082 = 0;
    *(uint64_t*)0x20000088 = 0;
    *(uint64_t*)0x20000090 = 0x405c92ec;
    *(uint32_t*)0x20000098 = 0;
    *(uint32_t*)0x2000009c = 0;
    *(uint32_t*)0x200000a0 = 0;
    *(uint32_t*)0x200000a4 = 0;
    *(uint32_t*)0x200000a8 = 0;
    *(uint32_t*)0x200000ac = 0;
    syscall(__NR_ioctl, r[1], 0x40305828, 0x20000080);
    break;
  case 5:
    *(uint16_t*)0x20000140 = 0;
    *(uint16_t*)0x20000142 = 0;
    *(uint64_t*)0x20000148 = 0x56cd4216;
    *(uint64_t*)0x20000150 = 0x10001;
    *(uint32_t*)0x20000158 = 0;
    *(uint32_t*)0x2000015c = 0;
    *(uint32_t*)0x20000160 = 0;
    *(uint32_t*)0x20000164 = 0;
    *(uint32_t*)0x20000168 = 0;
    *(uint32_t*)0x2000016c = 0;
    syscall(__NR_ioctl, r[0], 0x40305828, 0x20000140);
    break;
  case 6:
    *(uint32_t*)0x20000300 = 0;
    *(uint32_t*)0x20000304 = r[1];
    *(uint64_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0xfffa931c;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 0;
    syscall(__NR_ioctl, r[0], 0xc028660f, 0x20000300);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}