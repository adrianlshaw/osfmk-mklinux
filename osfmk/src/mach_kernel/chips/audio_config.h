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
/* CMU_HIST */
/*
 * Revision 2.3  93/03/26  17:59:52  mrt
 * 	Don't define by default the register map.
 * 
 * Revision 2.2  93/03/18  10:37:01  mrt
 * 	Created.
 * 	[93/03/17  21:38:48  af]
 */
/* CMU_ENDHIST */

/*
 *  Here platform specific code to define sample_t & co
 *  [to cope with weird DMA engines], and other customs
 */
#if	defined(__alpha)
#define splaudio	splbio
#define	sample_t	unsigned char	/* later */
#define	samples_to_chars	bcopy
#define	chars_to_samples	bcopy
/* Sparse space ! */
typedef struct {
	volatile unsigned long	cr;	/* command register (wo) */
/*#define ir cr				/* interrupt register (ro) */
	volatile unsigned long	dr;	/* data register (rw) */
	volatile unsigned long	dsr1;	/* D-channel status register 1 (ro) */
	volatile unsigned long	der;	/* D-channel error register (ro) */
	volatile unsigned long	dctb;	/* D-channel transmit register (wo) */
/*#define dcrb dctb			/* D-channel receive register (ro) */
	volatile unsigned long	bbtb;	/* Bb-channel transmit register (wo) */
/*#define bbrb bbtb			/* Bb-channel receive register (ro) */
	volatile unsigned long	bctb;	/* Bc-channel transmit register (wo)*/
/*#define bcrb bctb			/* Bc-channel receive register (ro) */
	volatile unsigned long	dsr2;	/* D-channel status register 2 (ro) */
} amd79c30_padded_regs_t;

/* give the chip 400ns in between accesses */
#define	read_reg(r,v)				\
	{ (v) = ((r) >> 8) & 0xff; delay(1); }

#define write_reg(r,v)				\
	{ (r) = (((v) & 0xff) << 8) |		\
		 0x200000000L; /*bytemask*/	\
		delay(1); wbflush();		\
	}

/* Write 16 bits of data from variable v to the data port of the audio chip */
#define	WAMD16(regs, v)				\
	{ write_reg((regs)->dr,v);		\
	  write_reg((regs)->dr,v>>8); }

#define mb() wbflush()

#endif	/* defined(__alpha) */


#ifdef	MAXINE
#define splaudio	splhigh
typedef struct {
	volatile unsigned char	cr;	/* command register (wo) */
/*#define ir cr				/* interrupt register (ro) */
	char				pad0[63];
	volatile unsigned char	dr;	/* data register (rw) */
	char				pad1[63];
	volatile unsigned char	dsr1;	/* D-channel status register 1 (ro) */
	char				pad2[63];
	volatile unsigned char	der;	/* D-channel error register (ro) */
	char				pad3[63];
	volatile unsigned char	dctb;	/* D-channel transmit register (wo) */
/*#define dcrb dctb			/* D-channel receive register (ro) */
	char				pad4[63];
	volatile unsigned char	bbtb;	/* Bb-channel transmit register (wo) */
/*#define bbrb bbtb			/* Bb-channel receive register (ro) */
	char				pad5[63];
	volatile unsigned char	bctb;	/* Bc-channel transmit register (wo)*/
/*#define bcrb bctb			/* Bc-channel receive register (ro) */
	char				pad6[63];
	volatile unsigned char	dsr2;	/* D-channel status register 2 (ro) */
	char				pad7[63];
} amd79c30_padded_regs_t;

/* give the chip 400ns in between accesses */
#define	read_reg(r,v)				\
	{ (v) = (r); delay(1); }

#define write_reg(r,v)				\
	{ (r) = (v); delay(1); wbflush(); }

/* Write 16 bits of data from variable v to the data port of the audio chip */
#define	WAMD16(regs, v)				\
	{ write_reg((regs)->dr,v);		\
	  write_reg((regs)->dr,v>>8); }

#define mb()

#endif	/* MAXINE */


#ifndef	sample_t
#define	sample_t	unsigned char
#define	samples_to_chars	bcopy
#define	chars_to_samples	bcopy
#endif

/*
 * More architecture-specific customizations
 */
#ifdef	alpha
#define sample_rpt_int(x)	(((x)<<24)|((x)<<16)|((x)<<8)|((x)<<0))
#define sample_rpt_long(x)	((sample_rpt_int(x)<<32)|sample_rpt_int(x))
#endif

#ifndef	sample_rpt_long
#define sample_rpt_long(x)	(((x)<<24)|((x)<<16)|((x)<<8)|((x)<<0))
#endif

