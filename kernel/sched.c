#include <debug/headers/sched.h>

#include <debug/qemu.h>

#include <kernel/sched.h>
#include <sys/pic_i8259.h>
#include <sys/x86.h>
#include <mm/kheap.h>
#include <klib/list.h>

struct sched_runq {
  struct task *current;
  struct task *idle;

  struct list queue;
};

static struct sched_runq *runq;

extern void intr_leave(void); /* sys/isr.S */

static void
idle_kworker(void)
{
  while (1)
    cpu_halt();
}

void
sched_init(void)
{
  task_init();
  runq = (struct sched_runq*) kmalloc(sizeof(*runq));
  list_init(&runq->queue);

  task_create_kworker("cpu idle", &idle_kworker);
  runq->idle = sched_dequeue();

  dprintf("initialized.\n");
}

void
sched_enqueue(struct task *task)
{
  list_add_tail(&runq->queue, &task->node);
}

struct task*
sched_dequeue(void)
{
  struct task *task = list_first_entry(&runq->queue, struct task, node);
  list_remove(&task->node);

  return task;
}

struct task*
sched_get_current(void)
{
  return runq->current;
}

static inline struct task*
sched_get_prev(void)
{
  return runq->current;
}

static struct task*
sched_get_next(void)
{
  if (list_empty(&runq->queue))
    return runq->idle;

  return sched_dequeue();
}

static void
sched_put_prev(struct task *task)
{
  sched_enqueue(task);
}

static void
switch_to(struct task *task)
{
  runq->current = task;
  dprintf("switch to task '%s' pid %d\n", task->name, task->pid);
}

void
schedule(void)
{
  struct task *prev = sched_get_prev();

  if (prev->state == RUNNING && prev != runq->idle)
    sched_put_prev(prev);

  struct task *next = sched_get_next();

  if (prev != next)
    switch_to(next);
}

void
sched_enable(void)
{
  schedule();
  intr_leave();
}
