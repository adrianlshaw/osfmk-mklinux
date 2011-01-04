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

#ifndef	_PPC_POWERMAC_MP_MP_H_
#define _PPC_POWERMAC_MP_MP_H_

#include <cpus.h>

#if	NCPUS > 1

#ifndef	ASSEMBLER
#include <kern/lock.h>
extern int real_ncpus;				/* real number of cpus */
extern int wncpu;				/* wanted number of cpus */
decl_simple_lock_data(extern, kdb_lock)		/* kdb lock */

extern int kdb_cpu;				/* current cpu running kdb */
extern int kdb_debug;
extern int kdb_is_slave[];
extern int kdb_active[];
#endif	/* ASSEMBLER */

#endif	/* NCPUS > 1 */

#endif	/* _PPC_POWERMAC_MP_MP_H_ */
