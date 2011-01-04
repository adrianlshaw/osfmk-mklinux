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
 * Copyright (c) 1990,1991 The University of Utah and
 * the Center for Software Science (CSS).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the Center
 * for Software Science at the University of Utah.''
 *
 * Copyright (c) 1990 mt Xinu, Inc.
 * This file may be freely distributed in any form as long as
 * this copyright notice is included.
 * MTXINU, THE UNIVERSITY OF UTAH, AND CSS PROVIDE THIS SOFTWARE ``AS
 * IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * CSS requests users of this software to return to css-dist@cs.utah.edu any
 * improvements that they make and grant CSS redistribution rights.
 *
 * 	Utah $Hdr: pdc.h 1.8 92/05/22$
 *	Author: Jeff Forys (CSS), Dave Slattengren (mtXinu)
 */

#ifndef	_MACHINE_PDC_H_
#define _MACHINE_PDC_H_

#ifndef ASSEMBLER
#include <mach/time_value.h>
#include <mach/machine/vm_types.h>
#include <types.h>
#include <hp_pa/HP700/iotypes.h>
#endif /* ASSEMBLER */

/*
 * Definitions for interaction with "Processor Dependent Code",
 * which is a set of ROM routines used to provide information to the OS.
 * Also includes definitions for the layout of "Page Zero" memory when
 * boot code is invoked.
 *
 * Glossary:
 *	PDC:	Processor Dependent Code (ROM or copy of ROM).
 *	IODC:	I/O Dependent Code (module-type dependent code).
 *	IPL:	Boot program (loaded into memory from boot device).
 *	HPA:	Hard Physical Address (hardwired address).
 *	SPA:	Soft Physical Address (reconfigurable address).
 */

/*
 * Our Initial Memory Module is laid out as follows.
 *
 *	0x000		+--------------------+
 *			|     Page Zero      |
 *	0x800		+--------------------+
 *			|                    |
 *			|                    |
 *			|        PDC         |
 *			|                    |
 *			|                    |
 *	MEM_FREE	+--------------------+
 *			|                    |
 *              	|    Console IODC    |
 *			|                    |
 *	MEM_FREE+16k	+--------------------+
 *			|                    |
 *              	|  Boot Device IODC  |
 *			|                    |
 *	IPL_START	+--------------------+
 *			|                    |
 *			| IPL Code or Kernel |
 *			|                    |
 *			+--------------------+
 *
 * Restrictions:
 *	MEM_FREE (pagezero.mem_free) can be no greater than 32K.
 *	The PDC may use up to MEM_FREE + 32K (for Console & Boot IODC).
 *	IPL_START must be less than or equal to 64K.
 *
 * The IPL (boot) Code is immediately relocated to RELOC (check
 * "../hp800stand/Makefile") to make way for the Kernel.
 */

#define	IODC_MAXSIZE	(16 * 1024)	/* maximum size of IODC */


/*
 * The "Top 15 PDC Entry Points" and their arguments...
 */

#define	PDC_POW_FAIL	1	/* prepare for power failure */
#define PDC_POW_FAIL_DFLT	0

#define	PDC_CHASSIS	2	/* update chassis display (see below) */
#define	PDC_CHASSIS_DISP	0	/* update display */
#define	PDC_CHASSIS_WARN	1	/* return warnings */
#define	PDC_CHASSIS_ALL		2	/* update display & return warnings */

#define	PDC_PIM		3	/* access Processor Internal Memory */
#define	PDC_PIM_HPMC		0	/* read High Pri Mach Chk data */
#define	PDC_PIM_SIZE		1	/* return size */
#define	PDC_PIM_LPMC		2	/* read Low Pri Mach Chk data */
#define	PDC_PIM_SOFT		3	/* read Soft Boot data */
#define	PDC_PIM_TOC		4	/* read Transfer of Control data */

#define	PDC_MODEL	4	/* processor model number info */
#define	PDC_MODEL_INFO		0	/* processor model number info */
#define	PDC_MODEL_BOOTID	1	/* set BOOT_ID of processor */
#define	PDC_MODEL_COMP		2	/* return component version numbers */
#define	PDC_MODEL_MODEL		3	/* return system model information */
#define	PDC_MODEL_ENSPEC	4	/* enable product-specific instrs */
#define	PDC_MODEL_DISPEC	5	/* disable product-specific instrs */

#define	PDC_CACHE	5	/* return cache and TLB params */
#define	PDC_CACHE_DFLT		0

#define	PDC_HPA		6	/* return HPA of processor */
#define	PDC_HPA_DFLT		0

#define	PDC_COPROC	7	/* return co-processor configuration */
#define	PDC_COPROC_DFLT		0

#define	PDC_IODC	8	/* talk to IODC */
#define	PDC_IODC_READ		0	/* read IODC entry point */
#define	PDC_IODC_NINIT		2	/* non-destructive init */
#define	PDC_IODC_DINIT		3	/* destructive init */
#define	PDC_IODC_MEMERR		4	/* check for memory errors */
#define	PDC_IODC_INDEX_DATA	0	/* get first 16 bytes from mod IODC */

#define	PDC_TOD		9	/* access time-of-day clock */
#define	PDC_TOD_READ		0	/* read TOD clock */
#define	PDC_TOD_WRITE		1	/* write TOD clock */
#define	PDC_TOD_ITIMER		2	/* calibrate Interval Timer (CR16) */

#define	PDC_STABLE	10	/* access Stable Storage (SS) */
#define	PDC_STABLE_READ		0	/* read SS */
#define	PDC_STABLE_WRITE	1	/* write SS */
#define	PDC_STABLE_SIZE		2	/* return size of SS */
#define	PDC_STABLE_VRFY		3	/* verify contents of SS */
#define	PDC_STABLE_INIT		4	/* initialize SS */

#define	PDC_NVM		11	/* access Non-Volatile Memory (NVM) */
#define	PDC_NVM_READ		0	/* read NVM */
#define	PDC_NVM_WRITE		1	/* write NVM */
#define	PDC_NVM_SIZE		2	/* return size of NVM */
#define	PDC_NVM_VRFY		3	/* verify contents of NVM */
#define	PDC_NVM_INIT		4	/* initialize NVM */

#define	PDC_ADD_VALID	12	/* check address for validity */
#define	PDC_ADD_VALID_DFLT	0

#define	PDC_BUS_BAD	13	/* verify Error Detection Circuitry (EDC) */
#define	PDC_BUS_BAD_DLFT	0

#define	PDC_DEBUG	14	/* return address of PDC debugger */
#define	PDC_DEBUG_DFLT		0

#define	PDC_INSTR	15	/* return instr that invokes PDCE_CHECK */
#define	PDC_INSTR_DFLT		0

#define	PDC_INSTR	15	/* return instr that invokes PDCE_CHECK */
#define	PDC_INSTR_DFLT		0

#define	PDC_PROC	16	/* stop currently executing processor */
#define	PDC_PROC_DFLT		0

#define	PDC_CONF	17	/* (de)configure a module */
#define	PDC_CONF_DECONF		0	/* deconfigure module */
#define	PDC_CONF_RECONF		1	/* reconfigure module */
#define	PDC_CONF_INFO		2	/* get config informaion */

#define PDC_BLOCK_TLB	18	/* Manage Block TLB entries (BTLB) */
#define PDC_BTLB_DEFAULT	0	/* Return BTLB configuration info  */
#define PDC_BTLB_INSERT		1	/* Insert a BTLB entry             */
#define PDC_BTLB_PURGE		2	/* Purge a BTLB entry              */
#define PDC_BTLB_PURGE_ALL	3	/* Purge all BTLB entries          */

#define PDC_TLB		19	/* Manage Hardware TLB handling */
#define PDC_TLB_INFO		0	/* Return HW-TLB configuration info  */
#define PDC_TLB_CONFIG		1	/* Set HW-TLB pdir base and size */

#define PDC_TLB_CURRPDE		1	/* cr28 points to current pde on miss */
#define PDC_TLB_RESERVD		3	/* reserved */
#define PDC_TLB_NEXTPDE		5	/* cr28 points to next pde on miss */
#define PDC_TLB_WORD3		7	/* cr28 is word 3 of 16 byte pde */

#define PDC_MEM_MAP		128	/* s700: returns page information */
#define PDC_MEM_MAP_HPA		0	/* module # --> hpa */

#define PDC_EEPROM             129     /* Read/write EEPROM (no, 129 is not a spelling error) */
#define PDC_EEPROM_READ_WORD         0             /* Read data word */
#define PDC_EEPROM_WRITE_WORD        1             /* Write data word */
#define PDC_EEPROM_READ_BYTE         2             /* Read data byte */
#define PDC_EEPROM_WRITE_BYTE        3             /* Write data byte */

#define PDC_LAN_STATION_ID      138     /* Hversion dependent mechanism for */
#define PDC_LAN_STATION_ID_READ 0       /* getting the lan station address  */

#ifndef ASSEMBLER
union pdc_eeprom {
    int			word;
    unsigned char	byte;
    unsigned long	whole[6];
};

/*
 * Commonly used PDC calls and the structures they return.
 */

struct pdc_model {	/* PDC_MODEL */
    unsigned int hvers;		/* hardware version */
    unsigned int svers;		/* software version */
    unsigned int hw_id;		/* unique processor hardware identifier */
    unsigned int boot_id;	/* same as hw_id */
    unsigned int sw_id;		/* software security and licensing */
    unsigned int sw_cap;	/* O/S capabilities of processor */
    unsigned int arch_rev;	/* architecture revision */
    unsigned int pot_key;	/* potential key */
    unsigned int curr_key;	/* current key */
    int		 filler[23];
};

struct pdc_chassis {	/* PDC_CHASSIS */
    unsigned int warning;
    int		 filler[31];
};

struct pdc_pim {	/* PDC_PIM/PDC_PIM_SIZE */
    int		 size;
    int		 archsize;
    int		 filler[30];
};

struct pdc_sys_model {  /* PDC_MODEL/PDC_MODEL_SYS_MODEL */
    unsigned int mod_len;
    int		 filler[31];
};


/*
 * CPU types
 */

#define CPU_M808	0x103   /* Restricted 815. Use SW_CAP */
#define CPU_M810	0x100
#define CPU_M815	0x103
#define CPU_M820	0x101
#define CPU_M825	0x008
#define CPU_M827	0x102
#define CPU_M832	0x102
#define CPU_M834	0x00A	/* 835 2 user WS */
#define CPU_M835	0x00A
#define CPU_M836	0x00A	/* 835 2 user Server */
#define CPU_M840	0x004
#define CPU_M844	0x00B	/* 845 2 user WS */
#define CPU_M845	0x00B
#define CPU_M846	0x00B	/* 845 2 user Server */
#define CPU_M850	0x080
#define CPU_M850_OPDC	0x00C	/* Old number for 850 */
#define CPU_M855	0x081

#define CPU_UNRESTRICTED 0      /* unrestricted */
#define CPU_SERVER       1      /* 2 user server */     
#define CPU_WS           2      /* 2 user workstation */
#define CPU_RESTRICTED   1      /* general limited I/O slots */


struct cache_cf {	/* for "struct pdc_cache" (PDC_CACHE) */
    unsigned int cc_resv0: 4,
		 cc_block: 4,	/* used to determine most efficient stride */
		 cc_line : 3,	/* max data written by store (16-byte mults) */
		 cc_resv1: 2,	/* (reserved) */
		 cc_wt   : 1,	/* D-cache: write-back = 0, write-through = 1 */
		 cc_sh   : 2,	/* separate I and D = 0, shared I and D = 1 */
		 cc_cst  : 3,	/* D-cache: incoherent = 0, coherent = 1 */
		 cc_resv2: 5,	/* (reserved) */
		 cc_assoc: 8;	/* D-cache: associativity of cache */
};

struct tlb_cf {		/* for "struct pdc_cache" (PDC_CACHE) */
    unsigned int tc_resv1:12,	/* (reserved) */
		 tc_sh   : 2,	/* separate I and D = 0, shared I and D = 1 */
		 tc_hvers: 1,	/* H-VERSION dependent */
		 tc_upage: 1,	/* ITLB: 2K page size = 0, 4k page size = 1 */
				/* DTLB: is TLB U-bit implemented ? */
		 tc_cst  : 3,	/* incoherent = 0, coherent = 1 */
		 tc_resv2: 5,	/* (reserved) */
		 tc_assoc: 8;	/* associativity of TLB */
};

struct pdc_cache {	/* PDC_CACHE */
/* Instruction cache */
    unsigned int ic_size;	/* size of I-cache (in bytes) */
    struct cache_cf ic_conf;	/* cache configuration (see above) */
    unsigned int ic_base;	/* start addr of I-cache (for FICE flush) */
    unsigned int ic_stride;	/* addr incr per i_count iteration (flush) */
    unsigned int ic_count;	/* number of i_loop iterations (flush) */
    unsigned int ic_loop;	/* number of FICE's per addr stride (flush) */
/* Data cache */
    unsigned int dc_size;	/* size of D-cache (in bytes) */
    struct cache_cf dc_conf;	/* cache configuration (see above) */
    unsigned int dc_base;	/* start addr of D-cache (for FDCE flush) */
    unsigned int dc_stride;	/* addr incr per d_count iteration (flush) */
    unsigned int dc_count;	/* number of d_loop iterations (flush) */
    unsigned int dc_loop;	/* number of FDCE's per addr stride (flush) */
/* Instruction TLB */
    unsigned int it_size;	/* number of entries in I-TLB */
    struct tlb_cf it_conf;	/* I-TLB configuration (see above) */
    unsigned int it_sp_base;	/* start space of I-TLB (for PITLBE flush) */
    unsigned int it_sp_stride;	/* space incr per sp_count iteration (flush) */
    unsigned int it_sp_count;	/* number of off_count iterations (flush) */
    unsigned int it_off_base;	/* start offset of I-TLB (for PITLBE flush) */
    unsigned int it_off_stride;	/* offset incr per off_count iteration (flush)*/
    unsigned int it_off_count;	/* number of it_loop iterations/space (flush) */
    unsigned int it_loop;	/* number of PITLBE's per off_stride (flush) */
/* Data TLB */
    unsigned int dt_size;	/* number of entries in D-TLB */
    struct tlb_cf dt_conf;	/* D-TLB configuration (see above) */
    unsigned int dt_sp_base;	/* start space of D-TLB (for PDTLBE flush) */
    unsigned int dt_sp_stride;	/* space incr per sp_count iteration (flush) */
    unsigned int dt_sp_count;	/* number of off_count iterations (flush) */
    unsigned int dt_off_base;	/* start offset of D-TLB (for PDTLBE flush) */
    unsigned int dt_off_stride;	/* offset incr per off_count iteration (flush)*/
    unsigned int dt_off_count;	/* number of dt_loop iterations/space (flush) */
    unsigned int dt_loop;	/* number of PDTLBE's per off_stride (flush) */
    int	 	 filler[2];
};

struct pdc_hpa {	/* PDC_HPA */
    struct iomod *hpa;	/* HPA of processor */
    int		 filler[31];
};

struct pdc_coproc {	/* PDC_COPROC */
    unsigned int ccr_enable;	/* same format as CCR (CR 10) */
    int		 ccr_present;	/* which co-proc's are present (bitset) */
    int		 filler[30];
};

struct pdc_iodc {	/* PDC_IODC */
    int		 size;
    int		 filler1[15];
    int		 hver_resv1;
    int		 filler2[15];
};

struct pdc_iodc_read {	/* PDC_IODC, PDC_IODC_READ */
    int		 size;		/* number of bytes in selected entry point */
    int		 filler[31];
};

struct pdc_mem {	/* PDC_IODC/READ */
    unsigned int status;
    int		 max_spa;
    int		 max_mem;
    int		 filler[29];

};

struct pdc_iodc_id_chief {	/* PDC_IODC_ID_CHIEFTAIN */
    struct io_module *chieftain;
    int		 filler[31];
};

struct pdc_iodc_minit {	/* PDC_IODC, PDC_IODC_NINIT or PDC_IODC_DINIT */
    unsigned int stat;		/* HPA.io_status style error returns */
    unsigned int max_spa;	/* size of SPA (in bytes) > max_mem+map_mem */
    unsigned int max_mem;	/* size of "implemented" memory (in bytes) */
    unsigned int map_mem;	/* size of "mapable-only" memory (in bytes) */
    int		 filler[28];
};

struct pdc_iodc_clr {	/* PDC_IODC/PDC_IODC_MEM_CLEAR */
    unsigned int stat;
    unsigned int sadd;
    unsigned int info;
    unsigned int madd;
    int		 filler[28];
};

struct pdc_time {	/* PDC_TOD */
    struct time_value time;
    int		 filler[30];
};

struct pdc_tod_calib {	/* PDC_TOD */
    double	it_freq;
    int		todr_acc;
    int		it_acc;
    int		filler[28];
};

struct pdc_ss_size {	/* PDC_STABLE */
    int		 count;
    int		 filler[31];
};

struct pdc_nvm_size {	/* PDC_NVOLATIVE */
    int		 count;
    int		 filler[31];
};

struct pdc_memory_map { 	/* PDC_MEMORY_MAP */
    unsigned int hpa;		/* mod's register set address 	*/
    unsigned int more_pgs;	/* number of additional I/O pgs */
};

struct btlb_info {		/* for "struct pdc_btlb" (PDC_BTLB) */
    unsigned int resv0: 8,	/* (reserved) */
		 num_i: 8,	/* Number of instruction slots */
		 num_d: 8,	/* Number of data slots */
		 num_c: 8;	/* Number of combined slots */
};

struct pdc_btlb {	/* PDC_BLOCK_TLB */
    unsigned int min_size;	/* Min size in pages */
    unsigned int max_size;	/* Max size in pages */
    struct btlb_info finfo;	/* Fixed range info */
    struct btlb_info vinfo;	/* Variable range info */
    int 	 filler[28];
};

struct pdc_hwtlb {	/* PDC_TLB */
    unsigned int min_size;	/* What do these mean? */
    unsigned int max_size;
    int		 filler[30];
};

struct pdc_hwtlb_parms { /* PDC_TLB/RTN_PARMS */
    unsigned int min_size;
    unsigned int max_size;
    int		 filler[30];
};

struct pdc_memmap {	/* PDC_MEMMAP */
    unsigned int hpa;		/* HPA for module */
    unsigned int morepages;	/* additional IO pages */
    int		 filler[30];
};

struct pdc_hwtlb_state { /* PDC_TLB/STATE_PARMS */
    unsigned int state;
    int		 filler[31];
};

#define	PDC_LAN_STATION_ID_SIZE	6
struct pdc_lan_station_id {	/* PDC_LAN_STATION_ID */
    unsigned char addr[PDC_LAN_STATION_ID_SIZE];
    unsigned char filler1[2];
    int		 filler2[30];
};

/*
 * The PDC_CHASSIS is a strange bird.  The format for updating the display
 * is as follows:
 *
 *	0     11 12      14    15   16    19 20    23 24    27 28    31
 *	+-------+----------+-------+--------+--------+--------+--------+
 *	|   R   | OS State | Blank |  Hex1  |  Hex2  |  Hex3  |  Hex4  |
 *	+-------+----------+-------+--------+--------+--------+--------+
 *
 * Unfortunately, someone forgot to tell the hardware designers that
 * there was supposed to be a hex display somewhere.  The result is,
 * you can only toggle 5 LED's and the fault light.
 *
 * Interesting values for Hex1-Hex4 and the resulting LED displays:
 *
 *	FnFF			CnFF:
 *	 0	- - - - -		Counts in binary from 0x0 - 0xF 
 *	 2	o - - - -		for corresponding values of `n'.
 *	 4	o o - - -
 *	 6	o o o - -
 *	 8	o o o o -
 *	 A	o o o o o
 *
 * If the "Blank" bit is set, the display should be made blank.
 * The values for "OS State" are defined below.
 */

#define	PDC_CHASSIS_BAR	0xF0FF	/* create a bar graph with LEDs */
#define	PDC_CHASSIS_CNT	0xC0FF	/* count with LEDs */

#define	PDC_OSTAT(os)	(((os) & 0x7) << 17)
#define	PDC_OSTAT_OFF	0x0	/* all off */
#define	PDC_OSTAT_FAULT	0x1	/* the red LED of death */
#define	PDC_OSTAT_TEST	0x2	/* self test */
#define	PDC_OSTAT_BOOT	0x3	/* boot program running */
#define	PDC_OSTAT_SHUT	0x4	/* shutdown in progress */
#define	PDC_OSTAT_WARN	0x5	/* battery dying, etc */
#define	PDC_OSTAT_RUN	0x6	/* OS running */
#define	PDC_OSTAT_ON	0x7	/* all on */


/*
 * Device path specifications used by PDC.
 */
struct device_path {
    unsigned char dp_flags;	/* see bit definitions below */
    char	  dp_bc[6];	/* Bus Converter routing info to a specific */
				/* I/O adaptor (< 0 means none, > 63 resvd) */
    unsigned char dp_mod;	/* fixed field of specified module */
    int		  dp_layers[6];	/* device-specific info (ctlr #, unit # ...) */
};


/* dp_flags */
#define	PF_AUTOBOOT	0x80	/* These two are PDC flags for how to locate */
#define	PF_AUTOSEARCH	0x40	/*	the "boot device" */
#define	PF_TIMER	0x0f	/* power of 2 # secs "boot timer" (0 == dflt) */

/*
 * A processors Stable Storage is accessed through the PDC.  There are
 * at least 96 bytes of stable storage (the device path information may
 * or may not exist).  However, as far as I know, processors provide at
 * least 192 bytes of stable storage.
 */
struct stable_storage {
    struct device_path	ss_pri_boot;	/* (see above) */
    char		ss_filenames[32];
    unsigned short	ss_os_version;	/* 0 == none, 1 == HP-UX, 2 == MPE-XL */
    char		ss_os[22];	/* OS-dependant information */
    char		ss_pdc[7];	/* reserved */
    char		ss_fast_size;	/* how much memory to test: */
					/* 0xf == all or else it's */
					/* (256KB << ss_fast_size) */
    struct device_path	ss_console;
    struct device_path	ss_alt_boot;
    struct device_path	ss_keyboard;
};


/*
 * Recoverable error indications provided to boot code by the PDC.
 * Any non-zero value indicates error.
 */
struct boot_err {
    unsigned int be_resv : 10,	/* (reserved) */
		 be_fixed:  6,	/* module that produced error */
		 be_chas : 16;	/* error code (interpret as 4 hex digits) */
};


/*
 * The PDC uses the following structure to completely define an I/O
 * module and the interface to its IODC.
 */
struct pz_device {
    struct device_path	pz_dp;
#define	pz_flags	pz_dp.dp_flags
#define	pz_bc		pz_dp.dp_bc
#define	pz_mod		pz_dp.dp_mod
#define	pz_layers	pz_dp.dp_layers
    struct iomod       *pz_hpa;	/* HPA base address of device */
    caddr_t		pz_spa;	/* SPA base address (zero if no SPA exists) */
    int	(*pz_iodc_io)(struct iomod*, ...);
				/* entry point of device's driver routines */
    short		pz_resv;/* (reserved) */
    unsigned short	pz_class;/* (see below) */
};

/* pz_class */
#define	PCL_NULL	0	/* illegal */
#define	PCL_RANDOM	1	/* random access (disk) */
#define	PCL_SEQU	2	/* sequential access (tape) */
#define	PCL_DUPLEX	7	/* full-duplex point-to-point (RS-232, Net) */
#define	PCL_KEYBD	8	/* half-duplex input (HIL Keyboard) */
#define	PCL_DISPL	9	/* half-duplex ouptput (display) */


/*
 * The first 2K of Soft Physical Address space on the Initial Memory Module
 * is aptly called "page zero".  The following structure defines the format
 * of page zero.  Individual members of this structure should be accessed
 * as "PAGE0->member".
 */

#define	PAGE0	((struct pagezero *)0)	/* can't get any lower than this! */

struct pagezero {
    /* [0x000] Initialize Vectors */
    int	ivec_special;		/* must be zero */
    int	(*ivec_mempf)(void);	/* powerfail recovery software */
    int	(*ivec_toc)(void);	/* exec'd after Transfer Of Control */
    int	ivec_toclen;		/* bytes of ivec_toc code */
    int	(*ivec_rendz)(void);	/* exec'd after Rendezvous Signal */
    int	ivec_mempflen;		/* bytes of ivec_mempf code */
    int	ivec_resv[10];		/* (reserved) must be zero */

    /* [0x040] Processor Dependent */
    union {
	volatile int	pd_Resv1[112];	/* (reserved) processor dependent */
	struct	{	/* Viper */
	    volatile int	  v_Resv1[39];
	    volatile unsigned int v_Ctrlcpy;	/* copy of Viper `vi_control' */
	    volatile int	  v_Resv2[72];
	} pd_Viper;
    } pz_Pdep;

    /* [0x200] Reserved */
    int	resv1[84];		/* (reserved) */

    /* [0x350] Memory Configuration */
    int	memc_cont;		/* bytes of contiguous valid memory */
    int	memc_phsize;		/* bytes of valid physical memory */
    int	memc_adsize;		/* bytes of SPA space used by PDC */
    int	memc_resv;		/* (reserved) */

    /* [0x360] Miscellaneous */
    struct boot_err mem_be[8];	/* boot errors (see above) */
    int	mem_free;		/* first free phys. memory location */
    struct iomod *mem_hpa;	/* HPA of CPU */
    int	(*mem_pdc)(int, ...);	/* PDC entry point */
    unsigned int mem_10msec;	/* # of Interval Timer ticks in 10msec*/

    /* [0x390] Initial Memory Module */
    struct iomod *imm_hpa;	/* HPA of Initial Memory module */
    int	imm_soft_boot;		/* 0 == hard boot, 1 == soft boot */
    int	imm_spa_size;		/* bytes of SPA in IMM */
    int	imm_max_mem;		/* bytes of mem in IMM (<= spa_size) */

    /* [0x3A0] Boot Console/Display, Device, and Keyboard */
    struct pz_device mem_cons;	/* description of console device */
    struct pz_device mem_boot;	/* description of boot device */
    struct pz_device mem_kbd;	/* description of keyboard device */

    /* [0x430] Reserved */
    int	resv2[116];		/* (reserved) */

    /* [0x600] Processor Dependent */
    int	pd_resv2[128];		/* (reserved) processor dependent */
};

#define lightshow(val) { \
              static int data; \
              data = (val << 8) | 0xC0FF; \
              (*PAGE0->mem_pdc)(PDC_CHASSIS, PDC_CHASSIS_DISP, data); \
              delay(5000000); \
}

/*
 * Prototypes.
 */
extern int pdc_iodc(int (*)(int, ...), ...);
extern int (*pdc)(int, ...);
extern void pdc_console_init(void);
extern void ptlball(struct pdc_cache *);
extern void cpuinfo(struct pdc_cache *);
extern int pdcgetc(void);
extern void pdcputc(char);
extern void pdcboot(int, void (*)(int));


#endif /* ASSEMBLER */

/*
 * MEM_PDC is needed by the assembly routine that handles pdc break calls
 */

#define	MEM_PDC		0x388

#define BOOT_VERSION 2

#endif /* _MACHINE_PDC_H_ */
