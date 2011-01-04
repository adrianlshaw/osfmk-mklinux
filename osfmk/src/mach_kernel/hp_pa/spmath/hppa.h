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
 * (c) Copyright 1986 HEWLETT-PACKARD COMPANY
 *
 * To anyone who acknowledges that this file is provided "AS IS" 
 * without any express or implied warranty:
 *     permission to use, copy, modify, and distribute this file 
 * for any purpose is hereby granted without fee, provided that 
 * the above copyright notice and this notice appears in all 
 * copies, and that the name of Hewlett-Packard Company not be 
 * used in advertising or publicity pertaining to distribution 
 * of the software without specific, written prior permission.  
 * Hewlett-Packard Company makes no representations about the 
 * suitability of this software for any purpose.
 */
/*
 * @(#)hppa.h: $Revision: 1.1.2.1 $ $Date: 1995/11/02 14:49:55 $
 * $Locker:  $
 */

/* amount is assumed to be a constant between 0 and 32 (non-inclusive) */
#define Shiftdouble(left,right,amount,dest)			\
    /* int left, right, amount, dest; */			\
    dest = ((left) << (32-(amount))) | ((unsigned int)(right) >> (amount))

/* amount must be less than 32 */
#define Variableshiftdouble(left,right,amount,dest)		\
    /* unsigned int left, right;  int amount, dest; */		\
    if (amount == 0) dest = right;				\
    else dest = ((((unsigned) left)&0x7fffffff) << (32-(amount))) |	\
          ((unsigned) right >> (amount))

/* amount must be between 0 and 32 (non-inclusive) */
#define Variable_shift_double(left,right,amount,dest)		\
    /* unsigned int left, right;  int amount, dest; */		\
    dest = (left << (32-(amount))) | ((unsigned) right >> (amount))
