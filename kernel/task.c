#include <debug/headers/sched.h>

#define __DEBUG_SUB_HEADER__  "TASK"

#include <debug/qemu.h>
#include <kernel/config.h>
#include <kernel/task.h>
#include <kernel/sched.h>
#include <sys/x86.h>
#include <sys/tss.h>
#include <mm/vmm.h>
#include <mm/mmu.h>
#include <mm/kheap.h>
#include <panic.h>
#include <stddef.h>
#include <string.h>

static struct task *ptable;
static int32_t kpid = -1;

extern void reschedule(void);

void
task_init(void)
{
  ptable = (struct task*) kmalloc(sizeof(*ptable) * NTASKS);
  dprintf("ptable initialized.\n");
}

struct task*
task_alloc(void)
{
  struct task *task = ptable;

  for (; task < ptable + NTASKS; task++)
    {
      if (task->state == UNUSED)
        {
          task->state = EMBRYO;
          pr_extra("allocated\n");
          return task;
        }
    }

  PANIC("ptable :: Out of memory\n");

  return NULL;
}

void
task_free(struct task *task)
{
  task->state = UNUSED;
}

static void
task_init_kstack(struct task *task, void (*fn)(void))
{
  uint32_t *kstack = (uint32_t*) task->kstack;

  *--kstack = 0x202;  /* eflags */
  *--kstack = 0x8;    /* cs */
  *--kstack = (uint32_t) fn;  /* eip */

  *--kstack = 0x0;  /* error code */
  *--kstack = 0x0;  /* intr number */

  *--kstack = 0x7;  /* eax */
  *--kstack = 0x6;  /* ebx */
  *--kstack = 0x5;  /* ecx */
  *--kstack = 0x4;  /* edx */
  *--kstack = 0x3;  /* edi */
  *--kstack = 0x2;  /* esi */
  *--kstack = 0x1;  /* ebp */

  *--kstack = 0x10;  /* ds */
  *--kstack = 0x10;  /* es */
  *--kstack = 0x10;  /* gs */
  *--kstack = 0x10;  /* fs */

  *--kstack = cpu_read_cr3();  /* cr3 */

  task->kesp = (uint32_t) kstack;
}

static void
task_init_ustack(struct task *task, void (*fn)(void))
{
  uint32_t *kstack = (uint32_t*) task->kstack;

  *--kstack = 0x23;
  *--kstack = (uint32_t) task->uesp;

  *--kstack = 0x202;  /* eflags */
  *--kstack = 0x1b;    /* cs */
  *--kstack = (uint32_t) fn;  /* eip */

  *--kstack = 0x0;  /* error code */
  *--kstack = 0x0;  /* intr number */

  *--kstack = 0x7;  /* eax */
  *--kstack = 0x6;  /* ebx */
  *--kstack = 0x5;  /* ecx */
  *--kstack = 0x4;  /* edx */
  *--kstack = 0x3;  /* edi */
  *--kstack = 0x2;  /* esi */
  *--kstack = 0x1;  /* ebp */

  *--kstack = 0x23;  /* ds */
  *--kstack = 0x23;  /* es */
  *--kstack = 0x23;  /* gs */
  *--kstack = 0x23;  /* fs */

  *--kstack = cpu_read_cr3();  /* cr3 */

  task->kesp = (uint32_t) kstack;
}

void
task_create_kworker(const char *name, void (*fn)(void))
{
  struct task *task = task_alloc();

  strcpy(task->name, name);
  task->state = RUNNING;
  task->pid = --kpid;
  task->kstack = kstack_alloc();
  task_init_kstack(task, fn);

  sched_enqueue(task);

  dprintf("create kernel worker '%s' pid %d\n", task->name, task->pid);
}

void
task_create_init_proc(const char *name, void (*fn)(void))
{
  struct task *task = task_alloc();

  strcpy(task->name, name);
  task->state = RUNNING;
  task->pid = 0;
  task->kstack = kstack_alloc();
  task->ustack = 0x0C000000 - PAGE_SIZE;
  task->uesp = PGROUNDUP(task->ustack) - 0x10;
  task_init_ustack(task, fn);
  tss_set_stack((void*) task->kesp);

  sched_enqueue(task);

  dprintf("create first init proc '%s' pid %d\n", task->name, task->pid);
}

void
yield(void)
{
  struct task *task = sched_get_current();
  uint32_t kesp = task->kesp;
  reschedule();
  task->kesp = kesp;
}

void
sleep(void)
{
  struct task *task = sched_get_current();
  task->state = SLEEP;
  yield();
}

void
wakeup(struct task *task)
{
  task->state = RUNNING;
  sched_enqueue(task);
}

void
task_save_context(uint32_t esp)
{
  struct task *task = sched_get_current();
  task->regs = (struct regs*) esp;
  task->kesp = esp;

  if (task->regs->cs != 0x8)
    tss_set_stack((void*) task->kesp);

  pr_extra("save current context task '%s' pid %d esp %p\n",
    task->name, task->pid, task->kesp);
}

void*
task_restore_context(void)
{
  struct task *task = sched_get_current();
  task->regs = (struct regs*) task->kesp;

  pr_extra("restore current context task '%s' pid %d context %p\n",
    task->name, task->pid, task->kesp);

  pr_extra("update tss stack %p\n", task->kesp);

  if (task->regs->cs != 0x8)
    {
      task->kesp = (uint32_t) tss_get_stack();
      tss_set_stack((void*) (task->kesp + 0x4c));
    }

  return (void*) task->kesp;
}
