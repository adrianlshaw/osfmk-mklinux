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
 * 
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
/* $Source: /u1/osc/rcs/mach_kernel/hp_pa/spmath/dfsqrt.c,v $
 * $Revision: 1.1.2.1 $	$Author: bruel $
 * $State: Exp $   	$Locker:  $
 * $Date: 1995/11/02 14:48:37 $
 */

#include "../spmath/float.h"
#include "../spmath/dbl_float.h"

/*
 *  Double Floating-point Square Root
 */

/*ARGSUSED*/
dbl_fsqrt(srcptr,nullptr,dstptr,status)

dbl_floating_point *srcptr, *dstptr;
unsigned int *nullptr, *status;
{
	register unsigned int srcp1, srcp2, resultp1, resultp2;
	register unsigned int newbitp1, newbitp2, sump1, sump2;
	register int src_exponent;
	register boolean guardbit = FALSE, even_exponent;

	Dbl_copyfromptr(srcptr,srcp1,srcp2);
        /*
         * check source operand for NaN or infinity
         */
        if ((src_exponent = Dbl_exponent(srcp1)) == DBL_INFINITY_EXPONENT) {
                /*
                 * is signaling NaN?
                 */
                if (Dbl_isone_signaling(srcp1)) {
                        /* trap if INVALIDTRAP enabled */
                        if (Is_invalidtrap_enabled()) return(INVALIDEXCEPTION);
                        /* make NaN quiet */
                        Set_invalidflag();
                        Dbl_set_quiet(srcp1);
                }
                /*
                 * Return quiet NaN or positive infinity.
		 *  Fall thru to negative test if negative infinity.
                 */
		if (Dbl_iszero_sign(srcp1) || 
		    Dbl_isnotzero_mantissa(srcp1,srcp2)) {
                	Dbl_copytoptr(srcp1,srcp2,dstptr);
                	return(NOEXCEPTION);
		}
        }

        /*
         * check for zero source operand
         */
	if (Dbl_iszero_exponentmantissa(srcp1,srcp2)) {
		Dbl_copytoptr(srcp1,srcp2,dstptr);
		return(NOEXCEPTION);
	}

        /*
         * check for negative source operand 
         */
	if (Dbl_isone_sign(srcp1)) {
		/* trap if INVALIDTRAP enabled */
		if (Is_invalidtrap_enabled()) return(INVALIDEXCEPTION);
		/* make NaN quiet */
		Set_invalidflag();
		Dbl_makequietnan(srcp1,srcp2);
		Dbl_copytoptr(srcp1,srcp2,dstptr);
		return(NOEXCEPTION);
	}

	/*
	 * Generate result
	 */
	if (src_exponent > 0) {
		even_exponent = Dbl_hidden(srcp1);
		Dbl_clear_signexponent_set_hidden(srcp1);
	}
	else {
		/* normalize operand */
		Dbl_clear_signexponent(srcp1);
		src_exponent++;
		Dbl_normalize(srcp1,srcp2,src_exponent);
		even_exponent = src_exponent & 1;
	}
	if (even_exponent) {
		/* exponent is even */
		/* Add comment here.  Explain why odd exponent needs correction */
		Dbl_leftshiftby1(srcp1,srcp2);
	}
	/*
	 * Add comment here.  Explain following algorithm.
	 * 
	 * Trust me, it works.
	 *
	 */
	Dbl_setzero(resultp1,resultp2);
	Dbl_allp1(newbitp1) = 1 << (DBL_P - 32);
	Dbl_setzero_mantissap2(newbitp2);
	while (Dbl_isnotzero(newbitp1,newbitp2) && Dbl_isnotzero(srcp1,srcp2)) {
		Dbl_addition(resultp1,resultp2,newbitp1,newbitp2,sump1,sump2);
		if(Dbl_isnotgreaterthan(sump1,sump2,srcp1,srcp2)) {
			Dbl_leftshiftby1(newbitp1,newbitp2);
			/* update result */
			Dbl_addition(resultp1,resultp2,newbitp1,newbitp2,
			 resultp1,resultp2);  
			Dbl_subtract(srcp1,srcp2,sump1,sump2,srcp1,srcp2);
			Dbl_rightshiftby2(newbitp1,newbitp2);
		}
		else {
			Dbl_rightshiftby1(newbitp1,newbitp2);
		}
		Dbl_leftshiftby1(srcp1,srcp2);
	}
	/* correct exponent for pre-shift */
	if (even_exponent) {
		Dbl_rightshiftby1(resultp1,resultp2);
	}

	/* check for inexact */
	if (Dbl_isnotzero(srcp1,srcp2)) {
		if (!even_exponent & Dbl_islessthan(resultp1,resultp2,srcp1,srcp2)) {
			Dbl_increment(resultp1,resultp2);
		}
		guardbit = Dbl_lowmantissap2(resultp2);
		Dbl_rightshiftby1(resultp1,resultp2);

		/*  now round result  */
		switch (Rounding_mode()) {
		case ROUNDPLUS:
		     Dbl_increment(resultp1,resultp2);
		     break;
		case ROUNDNEAREST:
		     /* stickybit is always true, so guardbit 
		      * is enough to determine rounding */
		     if (guardbit) {
			    Dbl_increment(resultp1,resultp2);
		     }
		     break;
		}
		/* increment result exponent by 1 if mantissa overflowed */
		if (Dbl_isone_hiddenoverflow(resultp1)) src_exponent+=2;

		if (Is_inexacttrap_enabled()) {
			Dbl_set_exponent(resultp1,
			 ((src_exponent-DBL_BIAS)>>1)+DBL_BIAS);
			Dbl_copytoptr(resultp1,resultp2,dstptr);
			return(INEXACTEXCEPTION);
		}
		else Set_inexactflag();
	}
	else {
		Dbl_rightshiftby1(resultp1,resultp2);
	}
	Dbl_set_exponent(resultp1,((src_exponent-DBL_BIAS)>>1)+DBL_BIAS);
	Dbl_copytoptr(resultp1,resultp2,dstptr);
	return(NOEXCEPTION);
}
