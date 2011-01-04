/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */

#ifndef	_PPC_DB_MACHDEP_H_
#define	_PPC_DB_MACHDEP_H_

/*
 * Machine-dependent defines for new kernel debugger.
 */

#include <kern/kern_types.h>
#include <mach/ppc/vm_types.h>
#include <mach/ppc/vm_param.h>
#include <ppc/thread.h>		/* for thread_status */
#include <ppc/trap.h>
#include <ppc/proc_reg.h>

typedef	vm_offset_t	db_addr_t;	/* address - unsigned */
typedef	int		db_expr_t;	/* expression - signed */

typedef struct ppc_saved_state db_regs_t;
db_regs_t	ddb_regs;	/* register state */
#define	DDB_REGS	(&ddb_regs)
extern int	db_active;	/* ddb is active */
extern char	db_stack_store[];

#define	PC_REGS(regs)	((db_addr_t)(regs)->srr0)

#define	BKPT_INST	0x7c810808	/* breakpoint instruction */
#define	BKPT_SIZE	(4)		/* size of breakpoint inst */
#define	BKPT_SET(inst)	(BKPT_INST)

#define db_clear_single_step(regs)	((regs)->srr1 &= ~MASK(MSR_SE))
#define db_set_single_step(regs)	((regs)->srr1 |= MASK(MSR_SE))

#define	IS_BREAKPOINT_TRAP(type, code)	(FALSE)
#define IS_WATCHPOINT_TRAP(type, code)	(FALSE)

#define	inst_trap_return(ins)	(FALSE)
#define	inst_return(ins)	(FALSE)
#define	inst_call(ins)		(FALSE)

int db_inst_load(unsigned long);
int db_inst_store(unsigned long);

/* access capability and access macros */

#define DB_ACCESS_LEVEL	DB_ACCESS_ANY	/* any space */
#define DB_CHECK_ACCESS(addr,size,task)				\
	db_check_access(addr,size,task)
#define DB_PHYS_EQ(task1,addr1,task2,addr2)			\
	db_phys_eq(task1,addr1,task2,addr2)
#define DB_VALID_KERN_ADDR(addr)				\
	((addr) >= VM_MIN_KERNEL_ADDRESS && 			\
	 (addr) < VM_MAX_KERNEL_ADDRESS)
#define DB_VALID_ADDRESS(addr,user)				\
	((!(user) && DB_VALID_KERN_ADDR(addr)) ||		\
	 ((user) && (addr) < VM_MAX_ADDRESS))

/*
 * Given pointer to ppc_saved_state, determine if it represents
 * a thread executing a) in user space, b) in the kernel, or c)
 * in a kernel-loaded task.  Return true for cases a) and c).
 */
#define IS_USER_TRAP(regs, etext)	\
     (USER_MODE(regs->srr1) ||		\
      (current_act() && 		\
       current_act()->kernel_loaded &&	\
       ((char *)(regs)->srr0 > (etext))))

extern boolean_t	db_check_access(
				vm_offset_t	addr,
				int		size,
				task_t		task);
extern boolean_t	db_phys_eq(
				task_t		task1,
				vm_offset_t	addr1,
				task_t		task2,
				vm_offset_t	addr2);
extern db_addr_t	db_disasm(
				db_addr_t	loc,
				boolean_t	altfmt,
				task_t		task);
extern void		db_read_bytes(
				vm_offset_t	addr,
				int		size,
				char		*data,
				task_t		task);
extern void		db_write_bytes(
				vm_offset_t	addr,
				int		size,
				char		*data,
				task_t		task);
extern void		db_stack_trace_cmd(
				db_expr_t	addr,
				boolean_t	have_addr,
				db_expr_t	count,
				char		*modif);
extern void		db_reboot(
				db_expr_t	addr,
				boolean_t	have_addr,
				db_expr_t	count,
				char		*modif);
extern void		db_low_trace(
				db_expr_t	addr,
				int		have_addr,
				db_expr_t	count,
				char 		*modif);

/* macros for printing OS server dependent task name */

#define DB_TASK_NAME(task)	db_task_name(task)
#define DB_TASK_NAME_TITLE	"COMMAND                "
#define DB_TASK_NAME_LEN	23
#define DB_NULL_TASK_NAME	"?                      "

extern void		db_task_name(
				task_t			task);

/* macro for checking if a thread has used floating-point */

#define db_act_fp_used(act)	(FALSE)

extern int		kdb_trap(
				int			type,
				int			code,
				struct ppc_saved_state	*regs);
extern boolean_t	db_trap_from_asm(
				struct ppc_saved_state *regs);
extern void		kdb_on(
				int			cpu);
extern void		cnpollc(
				boolean_t		on);
				
extern void		kdb_kintr(void);

extern boolean_t	db_is_alive(void);
#endif	/* _PPC_DB_MACHDEP_H_ */
