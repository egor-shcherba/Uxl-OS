#ifndef _DEBUG_CONFIG_H
#define _DEBUG_CONFIG_H

#define __LOG_ALL__                 0xFFFFFFFF

#define __LOG_KERNEL__              (1 << 0)
#define __LOG_CPU__                 (1 << 1)
#define __LOG_MEMORY__              (1 << 2)
#define __LOG_SCHED__               (1 << 3)
#define __LOG_SYSCALL__             (1 << 4)

#define __LOG_EXTRA__               ( 0 )

#define __DEBUG_LOG_SYSTEM__        ( __LOG_ALL__ )

#define __LOG_SUB_CPU_GDT__         (1 << 0)
#define __LOG_SUB_CPU_IDT__         (1 << 1)
#define __LOG_SUB_CPU_EXCEPTION__   (1 << 2)
#define __LOG_SUB_CPU_PIC__         (1 << 3)
#define __LOG_SUB_CPU_PIT__         (1 << 4)

#define __DEBUG_SUB_CPU__           ( __LOG_ALL__ )

#define __LOG_SUB_MEM_PMM__         (1 << 0)
#define __LOG_SUB_MEM_VMM__         (1 << 1)
#define __LOG_SUB_MEM_KMALLOC__     (2 << 2)

#define __DEBUG_SUB_MEMORY__        ( __LOG_ALL__ )

#define __LOG_SUB_SCHED_TASK__      (1 << 0)

#define __DEBUG_SUB_SCHED__         ( __LOG_ALL__ )

#endif /* NOT _DEBUG_CONFIG_H */
