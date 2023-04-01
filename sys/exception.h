#ifndef _EXCEPTION_H
#define _EXCEPTION_H

/* all definitions contains in sys/isr.S */

extern void exc_divide_error(void);     /* 0x0 */
extern void exc_debug(void);            /* 0x1 */
extern void exc_nmi(void);              /* 0x2 */
extern void exc_breakpoint(void);       /* 0x3 */
extern void exc_overflow(void);         /* 0x4 */
extern void exc_boundrange(void);       /* 0x5 */
extern void exc_inv_opcode(void);       /* 0x6*/
extern void exc_no_dev(void);           /* 0x7 */
extern void exc_double_fault(void);     /* 0x8, error code*/

extern void exc_inv_tss(void);          /* 0xA, error code */
extern void exc_seg_not_present(void);  /* 0xB, error code */
extern void exc_ss(void);               /* 0xC, error code */
extern void exc_general_prot(void);     /* 0xD, error code */
extern void exc_page_fault(void);

extern void exc_x87(void);              /* 0x10 */
extern void exc_aligned_check(void);    /* 0x11, error code */
extern void exc_machine_check(void);    /* 0x12 */
extern void exc_simd(void);             /* 0x13 */

#endif /* NOT _EXCEPTION_H */