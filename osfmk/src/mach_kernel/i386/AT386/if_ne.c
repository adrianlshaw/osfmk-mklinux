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
/*-
 * Copyright (c) 1990, 1991 William F. Jolitz.
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)if_ne.c     7.4 (Berkeley) 5/21/91
 */

/*
 * NE2000 Ethernet driver
 *
 * Parts inspired from Tim Tucker's if_wd driver for the wd8003,
 * insight on the ne2000 gained from Robert Clements PC/FTP driver.
 */

/*
 * Hacked for the OSF Mach Microkernel by Terrence Mitchem
 */


#include <ne.h>

#include	<platforms.h>
#include	<cpus.h>
#include 	<mp_v1_1.h>

#include	<string.h>
#include	<kern/time_out.h>
#include	<kern/spl.h>
#include	<kern/misc_protos.h>
#include	<device/device_types.h>
#include	<device/errno.h>
#include	<device/io_req.h>
#include	<device/if_hdr.h>
#include	<device/if_ether.h>
#include	<device/net_status.h>
#include	<device/net_io.h>
#include	<device/misc_protos.h>
#include	<device/ds_routines.h>
#include	<i386/ipl.h>
#include	<i386/pio.h>
#include	<chips/busses.h>
#include	<i386/AT386/ds8390.h>
#include	<i386/AT386/if_ne.h>
#include	<i386/AT386/if_ne_entries.h>
#include	<i386/AT386/misc_protos.h>
#include	<machine/endian.h>


#include <i386/AT386/mp/mp.h>
#ifdef	CBUS
#include <busses/cbus/cbus.h>
#endif	/* CBUS */
#if	MP_V1_1
#include <i386/AT386/mp/mp_v1_1.h>
#endif	/* MP_V1_1 */


static caddr_t ne_std[NNE] = {0};
static struct bus_device *ne_info[NNE];
struct  bus_driver nedriver =
        {(probe_t) neprobe, 0, neattach, 0, ne_std, "ne", ne_info, 0, 0, 0 };

#define ETHER_MIN_LEN 64
#define ETHER_MAX_LEN 1536
#define SPLNET splnet
/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ns_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
typedef struct {

#ifdef	MACH_KERNEL
	struct  ifnet ds_if;
	u_char  ds_addr[6];
#else	/* MACH_KERNEL */
        struct  arpcom ns_ac;           /* Ethernet common part */
#define ds_if   ns_ac.ac_if             /* network-visible interface */
#define ds_addr ns_ac.ac_enaddr         /* hardware Ethernet address */
#endif  /* MACH_KERNEL */

        int     ns_flags;
#define DSF_LOCK        1               /* block re-entering enstart */
#define DSF_RUNNING	2
        int     ns_oactive ;
        int     ns_mask ;
        int     ns_ba;                  /* byte addr in buffer ram of inc pkt */
        int     ns_cur;                 /* current page being filled */
        struct  prhdr   ns_ph;          /* hardware header of incoming packet*/
        struct  ether_header ns_eh;     /* header of incoming packet */
        u_char  ns_pb[2048 /*ETHERMTU+sizeof(long)*/];
        short   ns_txstart;             /* transmitter buffer start */
        short   ns_rxend;               /* recevier buffer end */
        short   ns_rxbndry;             /* recevier buffer boundary */
        caddr_t ns_port;                /* i/o port base */
        short   ns_mode;                /* word/byte mode */
	int	mode;
	short	card_present;
#ifndef MACH_KERNEL
        ihandler_t      handler;
        ihandler_id_t   *handler_id;
#endif	/* MACH_KERNEL */

} ne_softc_t;
ne_softc_t ne_softc[NNE];

#define PAT(n)  (0xa55a + 37*(n))

u_short boarddata[16];


/* Forward */
int  nerecv(ne_softc_t *);
void nestart(int);
int  neinit(int);
void nefetch (ne_softc_t *, int *, int, int);
void neput (ne_softc_t *, int *, int, int);

/*
 * Fetch from onboard ROM/RAM
 */
void nefetch (ne_softc_t *ns, int *up, int ad, int len) 
{
  u_char cmd;
  i386_ioport_t nec = (i386_ioport_t)(int)(ns->ns_port);
  int counter = 10000;
  int t_len;
  unsigned char last_word[2];
  char odd;

  cmd = inb (nec + ds_cmd);
  outb(nec + ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);

  /* Setup remote dma */
  outb (nec + ds0_isr, DSIS_RDC);

  t_len = len;
  if ((ns->ns_mode & DSDC_WTS) && len&1) {
    odd=1;
    t_len++;          /* roundup to words */
  } else {
    odd=0;
  }

  outb (nec+ds0_rbcr0, t_len);
  outb (nec+ds0_rbcr1, t_len>>8);
  outb (nec+ds0_rsar0, ad);
  outb (nec+ds0_rsar1, ad>>8);

  /* Execute & extract from card */
  outb ((nec+ds_cmd), DSCM_RREAD|DSCM_PG0|DSCM_START);

  if (ns->ns_mode & DSDC_WTS)
    if (odd) {
      linw (nec+ne_data, (int *)up, len/2);
      *(last_word) = inw(nec+ne_data); /* get last word */
      *(up+len-1) = last_word[0]; /* last byte */
    } else {
      linw (nec+ne_data, up, len/2);
    }
  else
    linb (nec+ne_data, (char *)up, len);


  /* Wait till done, then shutdown feature */
  while ((inb(nec+ds0_isr & DSIS_RDC) == 0) && counter-- > 0);

  outb (nec+ds0_isr, DSIS_RDC);
  outb (nec+ds_cmd, cmd);
}

/*
 * Put to onboard RAM
 */
void
neput (ne_softc_t *ns, int *up, int ad, int len)
{
        u_char cmd;
        i386_ioport_t nec = (i386_ioport_t) (int)ns->ns_port;
        int counter = 10000;
		int t_len;
		int odd;
		unsigned char last_word[2];

        cmd = inb(nec+ds_cmd);
        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);

        /* Setup for remote dma */
        outb (nec+ds0_isr, DSIS_RDC);

		t_len = len;
        if ((ns->ns_mode & DSDC_WTS) && len&1) {
				odd = 1;
                t_len++;          /* roundup to words */
		} else odd = 0;

        outb (nec+ds0_rbcr0, t_len);
        outb (nec+ds0_rbcr1, t_len>>8);
        outb (nec+ds0_rsar0, ad);
        outb (nec+ds0_rsar1, ad>>8);

        /* Execute & stuff to card */
        outb (nec+ds_cmd, DSCM_RWRITE|DSCM_PG0|DSCM_START);
        if (ns->ns_mode & DSDC_WTS) {
				if (odd) {
	                loutw (nec+ne_data, up, len/2);
					last_word[0] = *(up+len-1);
					outw (nec+ne_data, (unsigned short) *(last_word));
				}
				else {
	                loutw (nec+ne_data, up, len/2);
				}
		}
        else
                loutb (nec+ne_data, (char *)up, len);


        /* Wait till done, then shutdown feature */
        while ((inb (nec+ds0_isr) & DSIS_RDC) == 0 && counter-- > 0)
                ;

        outb (nec+ds0_isr, DSIS_RDC);
        outb (nec+ds_cmd, cmd);
}


int
neprobe(port, dev)
caddr_t port;
struct bus_device *dev;
{
        int val, i, sum, bytemode = 1, pat;
	int unit = dev->unit;
        ne_softc_t *ns = &ne_softc[unit];
        i386_ioport_t nec;

        if ((unsigned) unit >= NNE)
                return(0);

	ns->ns_port = (caddr_t) dev->address;
        nec = (i386_ioport_t)(int) dev->address;

	if (ns->card_present) {
		printf("ne%s : card already present in port %x\n",
			unit, nec);
		return(0);
	}

        if (bytemode) {
                /* Byte Transfers, Burst Mode Select, Fifo at 8 bytes */
                ns->ns_mode = DSDC_BMS|DSDC_FT1;
                ns->ns_txstart = TBUF8;
                ns->ns_rxend = RBUFEND8;
        } else {
word:
                /* Word Transfers, Burst Mode Select, Fifo at 8 bytes */
                ns->ns_mode = DSDC_WTS|DSDC_BMS|DSDC_FT1;
                ns->ns_txstart = TBUF16;
                ns->ns_rxend = RBUFEND16;
                bytemode = 0;
        }

        /* Reset the bastard */
        val = inb(nec + ne_reset);
        delay(200);
        outb(nec + ne_reset, val);
        delay(200);

        outb(nec + ds_cmd, DSCM_STOP|DSCM_NODMA);
        
        i = 10000;
        while ((inb(nec + ds0_isr) & DSIS_RESET) == 0 && i-- > 0);
        if (i < 0) return (0);

        outb(nec + ds0_isr, 0xff);
        outb(nec + ds0_dcr, ns->ns_mode);
        outb(nec + ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_STOP);
        delay(1000);

        /* Check cmd reg and fail if not right */
        if ((i = inb(nec + ds_cmd)) != (DSCM_NODMA|DSCM_PG0|DSCM_STOP))
                return(0);

        outb(nec + ds0_tcr, DSTC_LB0);
        outb(nec + ds0_rcr, DSRC_MON);
        outb(nec + ds0_pstart, ns->ns_txstart+PKTSZ);
        outb(nec + ds0_pstop, ns->ns_rxend);
        outb(nec + ds0_bnry, ns->ns_rxend);
        outb(nec + ds0_imr, 0);
        outb(nec + ds0_isr, 0);
        outb(nec + ds_cmd, DSCM_NODMA|DSCM_PG1|DSCM_STOP);
        outb(nec + ds1_curr, ns->ns_txstart+PKTSZ);
        outb(nec + ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_STOP);

#ifdef NEDEBUG
#define RCON    37
        {       int i, rom;

                rom=1;
                printf("ne ram ");
                
                for (i = 0; i < 0xfff0; i+=4) {
                        pat = PAT(i);
                        neput(ns, &pat,i,4);
                        nefetch(ns, &pat,i,4);
                        if (pat == PAT(i)) {
                                if (rom) {
                                        rom=0;
                                        printf(" %x", i);
                                }
                        } else {
                                if (!rom) {
                                        rom=1;
                                        printf("..%x ", i);
                                }
                        }
                        pat=0;
                        neput(ns, &pat,i,4);
                }
                printf("\n");
        }
#endif

        /*
         * <groan> detect difference between units
         * solely by where the RAM is decoded.
         */
        pat = PAT(0);
        neput  (ns, &pat, ns->ns_txstart*DS_PGSIZE, 4);
        nefetch(ns, &pat, ns->ns_txstart*DS_PGSIZE, 4);
        if (pat != PAT(0)) {
                if (bytemode)
                        goto word;
                else return (0);
        }


        /* Extract board address */
        nefetch (ns, (int *)boarddata, 0, sizeof(boarddata));

        for(i=0; i < 6; i++)
                ns->ds_addr[i] = boarddata[i];
	ns->card_present = 1;
        return (1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  We get the ethernet address here.
 */
void neattach(dev)
struct bus_device *dev;
{
        short unit = dev->unit;
        ne_softc_t *ns = &ne_softc[unit];
        register struct ifnet *ifp = &(ns->ds_if);

        if ((unsigned) unit >= NNE)
                return;

#ifdef	MACH_KERNEL
	take_dev_irq(dev);
#else	/* MACH_KERNEL */
        /* setup intr handler */
        ns->handler.ih_level            = dev->dev_pic;
        ns->handler.ih_handler          = dev->dev_intr[0];
        ns->handler.ih_resolver         = i386_resolver;
        ns->handler.ih_rdev             = dev;
        ns->handler.ih_stats.intr_type  = INTR_DEVICE;
        ns->handler.ih_stats.intr_cnt   = 0;
        ns->handler.ih_hparam[0].intparam = unit;
        if ((ns->handler_id = handler_add(&ns->handler)) != NULL)
                handler_enable(ns->handler_id);
        else
                panic("Unable to add NE interrupt handler");
#endif	/* MACH_KERNEL */
	printf (", port = %x, spl = %d, pic = %d, [%s].",
		dev->address, dev->sysdep, dev->sysdep1,
		ether_sprintf(ns->ds_addr));
#ifndef	MACH_KERNEL
	ns->ns_ac.ac_bcastaddr	= (u_char *)etherbroadcastaddr;
	ns->ns_ac.ac_arphrd	= ARPHRD_ETHER;
#endif	/* MACH_KERNEL */
	ns->ns_flags	= 0;
	ns->mode	= 0;
        ifp->if_unit	= unit;
        ifp->if_mtu	= ETHERMTU;
        ifp->if_flags	= IFF_BROADCAST;
#ifdef	MACH_KERNEL
	ifp->if_header_size = sizeof(struct ether_header);
	ifp->if_header_format = HDR_ETHERNET;
        ifp->if_address_size = 6;
	ifp->if_address = (char *)&ns->ds_addr[0];
	if_init_queues(ifp);
#else	/* MACH_KERNEL */
        ifp->if_name	= nedriver.driver_dname;
        ifp->if_init	= neinit;
        ifp->if_output	= ether_output;
        ifp->if_start	= nestart;
        ifp->if_ioctl	= neioctl;
        ifp->if_reset	= nereset;
        ifp->if_watchdog= 0;
        if_attach(ifp);
#endif	/* MACH_KERNEL */
}

/*
 * Initialization of interface; set up initialization block
 * and transmit/receive descriptor rings.
 */
int
neinit(int unit)
{
        ne_softc_t *ns = &ne_softc[unit];
        struct ifnet *ifp = &ns->ds_if;
        int i; char *cp;
	int oldpri;
        i386_ioport_t nec = (i386_ioport_t) (int)ns->ns_port;

#ifndef MACH_KERNEL
        if (ifp->if_addrlist == (struct ifaddr *)0) return;
#endif	/* MACH_KERNEL */

	oldpri = SPLNET();

        /* set physical address on ethernet */
        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG1|DSCM_STOP);
        for (i=0 ; i < 6 ; i++) outb(nec+ds1_par0+i,ns->ds_addr[i]);

        /* clr logical address hash filter for now */
        for (i=0 ; i < 8 ; i++) outb(nec+ds1_mar0+i,0xff);

        /* init regs */
        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_STOP);
        outb (nec+ds0_rbcr0, 0);
        outb (nec+ds0_rbcr1, 0);
        outb (nec+ds0_imr, 0);
        outb (nec+ds0_isr, 0xff);

        /* Word Transfer select, Burst Mode Select, Fifo at 8 bytes */
        outb(nec+ds0_dcr, ns->ns_mode);

        outb (nec+ds0_tcr, 0);
        outb (nec+ds0_rcr, DSRC_MON);
        outb (nec+ds0_tpsr, 0);
        outb (nec+ds0_pstart, ns->ns_txstart+PKTSZ);
        outb (nec+ds0_pstop, ns->ns_rxend);
        outb (nec+ds0_bnry, ns->ns_txstart+PKTSZ);
        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG1|DSCM_STOP);

        ns->ns_cur = ns->ns_txstart+PKTSZ + 1;
        outb (nec+ds1_curr, ns->ns_cur);

        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);
        outb (nec+ds0_rcr, DSRC_AB);
        outb (nec+ds0_dcr, ns->ns_mode);
        outb (nec+ds0_imr, 0xff);

        ns->ds_if.if_flags |= IFF_RUNNING;
        ns->ns_flags &= ~(DSF_LOCK|DSF_RUNNING);
        ns->ns_oactive = 0; ns->ns_mask = ~0;
        splx(oldpri);
        nestart(unit);
	return(1);
}

/*
 * Setup output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 * called only at splimp or interrupt level.
 */

void
nestart(unit)
int unit;
{
        ne_softc_t *ns = &ne_softc[unit];
        struct ifnet *ifp = &ns->ds_if;
        int buffer;
        int len, i, total,t;
        i386_ioport_t nec = (i386_ioport_t) (int)ns->ns_port;
#ifdef	MACH_KERNEL
	io_req_t m;

#else	/* MACH_KERNEL */
        struct mbuf *m0, *m;
#endif	/* MACH_KERNEL */

        /*
         * The DS8390 has only one transmit buffer, if it is busy we
         * must wait until the transmit interrupt completes.
         */
        outb(nec+ds_cmd,DSCM_NODMA|DSCM_START);

        if (ns->ns_flags & DSF_LOCK)
                goto done;

        if (inb(nec+ds_cmd) & DSCM_TRANS)
                goto done;

        if ((ns->ds_if.if_flags & IFF_RUNNING) == 0)
                goto done;

        IF_DEQUEUE(&ns->ds_if.if_snd, m);
 
        if (m == 0)
                goto done;

        /*
         * Copy the mbuf chain into the transmit buffer
         */

        ns->ns_flags |= DSF_LOCK;       /* prevent entering nestart */
        buffer = ns->ns_txstart*DS_PGSIZE;
#ifdef	MACH_KERNEL
	total = m->io_count;
	neput(ns, (int *)m->io_data, buffer, total);
#else	/* MACH_KERNEL */
        t = 0; len = i = 0;
        for (m0 = m; m != 0; m = m->m_next)
                t += m->m_len;
                
        m = m0;
        total = t;
        for (m0 = m; m != 0; ) {
                
                if (m->m_len&1 && t > m->m_len) {
                        neput(ns, mtod(m, caddr_t), buffer, m->m_len - 1);
                        t -= m->m_len - 1;
                        buffer += m->m_len - 1;
                        m->m_data += m->m_len - 1;
                        m->m_len = 1;
                        m = m_pullup(m, 2);
                } else {
                        neput(ns, mtod(m, caddr_t), buffer, m->m_len);
                        buffer += m->m_len;
                        t -= m->m_len;
                        MFREE(m, m0);
                        m = m0;
                }
        }
#endif	/* MACH_KERNEL */
        /*
         * Init transmit length registers, and set transmit start flag.
         */

        len = total;
        if (len < ETHER_MIN_LEN) len = ETHER_MIN_LEN;
        outb(nec+ds0_tbcr0,len&0xff);
        outb(nec+ds0_tbcr1,(len>>8)&0xff);
        outb(nec+ds0_tpsr, ns->ns_txstart);
        outb(nec+ds_cmd, DSCM_TRANS|DSCM_NODMA|DSCM_START);

#ifdef	MACH_KERNEL
	iodone(m);
	m = 0;
done:
	return;
#endif	/* MACH_KERNEL */
}

/* buffer successor/predecessor in ring? */
#define succ(n) (((n)+1 >= ns->ns_rxend) ? (ns->ns_txstart+PKTSZ) : (n)+1)
#define pred(n) (((n)-1 < (ns->ns_txstart+PKTSZ)) ? ns->ns_rxend-1 : (n)-1)

/*
 * Controller interrupt.
 */
int
neintr(unit)
{
        ne_softc_t *ns = &ne_softc[unit];
        u_char cmd,isr;
        i386_ioport_t nec = (i386_ioport_t) (int)ns->ns_port;


        /* Save cmd, clear interrupt */
        cmd = inb (nec+ds_cmd);
        isr = inb (nec+ds0_isr);
loop:
        outb(nec+ds_cmd,DSCM_NODMA|DSCM_START);
        outb(nec+ds0_isr, isr);

        /* Receiver error */
        if (isr & DSIS_RXE) {
                /* need to read these registers to clear status */
                (void) inb(nec+ ds0_rsr);
                (void) inb(nec+ 0xD);
                (void) inb(nec + 0xE);
                (void) inb(nec + 0xF);
                ns->ds_if.if_ierrors++;
        }

        /* We received something; rummage thru tiny ring buffer */
        if (isr & (DSIS_RX|DSIS_RXE|DSIS_ROVRN)) {
                u_char pend,lastfree;

                outb(nec+ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG1);
                pend = inb(nec+ds1_curr);
                outb(nec+ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG0);

                /* Something in the buffer? */
                while (pend != ns->ns_cur) {
                        /* Extract header from microcephalic board */
                        nefetch(ns, 
				(int *)&ns->ns_ph,
				ns->ns_cur*DS_PGSIZE,
                                sizeof(ns->ns_ph));
                        ns->ns_ba = ns->ns_cur*DS_PGSIZE+sizeof(ns->ns_ph);

                        /* Incipient paranoia */
                        if (ns->ns_ph.pr_status == DSRS_RPC ||
                                /* for dequna's */
                                ns->ns_ph.pr_status == 0x21) {
                                if (nerecv(ns))
					ns->ns_cur = ns->ns_ph.pr_nxtpg ;
				else {
                        		outb(nec+ds0_bnry, pred(ns->ns_cur));
					goto short_load;
				}
			}
#ifdef NEDEBUG
                        else  {
                                printf("cur %x pnd %x lfr %x ",
                                        ns->ns_cur, pend, lastfree);
                                printf("nxt %x len %x ", ns->ns_ph.pr_nxtpg,
                                        (ns->ns_ph.pr_sz1<<8)+ ns->ns_ph.pr_sz0);
                                printf("Bogus Sts %x\n", ns->ns_ph.pr_status);  
				ns->ns_cur = pend;
                        }
#endif
                        outb(nec+ds0_bnry, pred(ns->ns_cur));
                        outb(nec+ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG1);
                        pend = inb(nec+ds1_curr);
                        outb(nec+ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG0);
                }
short_load:
                outb(nec+ds_cmd, DSCM_START|DSCM_NODMA);
        }

        /* Transmit error */
        if (isr & DSIS_TXE) {

                ns->ns_flags &= ~DSF_LOCK;
                /* Need to read these registers to clear status */
                ns->ds_if.if_collisions += inb(nec+ds0_tbcr0);
                ns->ds_if.if_oerrors++;
        }

        /* Packet Transmitted */
        if (isr & DSIS_TX) {
                ns->ns_flags &= ~DSF_LOCK;
                ++ns->ds_if.if_opackets;
                ns->ds_if.if_collisions += inb(nec+ds0_tbcr0);
        }

        /* Receiver ovverun? */
        if (isr & DSIS_ROVRN) {
                outb(nec+ds0_rbcr0, 0);
                outb(nec+ds0_rbcr1, 0);
                outb(nec+ds0_tcr, DSTC_LB0);
                outb(nec+ds0_rcr, DSRC_MON);
                outb(nec+ds_cmd, DSCM_START|DSCM_NODMA);
                outb(nec+ds0_rcr, DSRC_AB);
                outb(nec+ds0_tcr, 0);
        }

        /* Any more to send? */
        outb (nec+ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);
        nestart(unit);
        outb (nec+ds_cmd, cmd);
        outb (nec+ds0_imr, 0xff);

        /* Still more to do? */
        isr = inb (nec+ds0_isr);
        if(isr) goto loop;
	return 0;
}

/*
 * Ethernet interface receiver interface.
 * If input error just drop packet.
 * Otherwise examine packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
int 
nerecv(ns)
        ne_softc_t *ns;
{
#ifdef	MACH_KERNEL
	ipc_kmsg_t	new_kmsg;
	struct ether_header *ehp;
	struct packet_header *pkt;
	register struct ifnet *ifp = &ns->ds_if;
#else	/* MACH_KERNEL */
        struct mbuf *top, **mp, *m, *p;
#endif	/* MACH_KERNEL */
        int len, l;
        int epkt;

        ns->ds_if.if_ipackets++;
        len = ns->ns_ph.pr_sz0 + (ns->ns_ph.pr_sz1<<8);
        if(len < ETHER_MIN_LEN || len > ETHER_MAX_LEN)
                return 0;

        nefetch(ns, (int *)&ns->ns_eh, ns->ns_ba, sizeof(struct ether_header));
#ifndef MACH_KERNEL
        ns->ns_eh.ether_type = ntohs((u_short)ns->ns_eh.ether_type);
#endif	/* MACH_KERNEL */
	ns->ns_ba += sizeof(struct ether_header);

        /* don't forget checksum! */
        len -= (sizeof(struct ether_header) + sizeof(long));
	new_kmsg = net_kmsg_get();

	if (new_kmsg == IKM_NULL) {
		ns->ds_if.if_rcvdrops++;
		return(0);
	}

	ehp = (struct ether_header *) (&net_kmsg(new_kmsg)->header[0]);
	pkt = (struct packet_header *)(&net_kmsg(new_kmsg)->packet[0]);
	*ehp = ns->ns_eh;

       	nefetch(ns, (int *)(char *) (pkt + 1), ns->ns_ba, len);

	pkt->type = ehp->ether_type;
	pkt->length = len + sizeof(struct packet_header);
	net_packet(ifp,
		   new_kmsg,
		   pkt->length,
		   ethernet_priority(new_kmsg),
		   (io_req_t)0);

	return 1;
}

io_return_t
neopen(dev, flag, ior)
dev_t dev;
dev_mode_t flag;
io_req_t ior;
{
	register int unit = minor(dev);

	if (!ne_softc[unit].card_present || unit < 0 || unit >= NNE)
		return (ENXIO);

	ne_softc[unit].ds_if.if_flags |= IFF_UP;
	neinit(unit);
	return(0);
}

io_return_t
neoutput(dev, ior)
dev_t dev;
io_req_t ior;
{
	register int unit = minor(dev);

	if (!ne_softc[unit].card_present || unit < 0 || unit >= NNE)
		return (ENXIO);

	return (net_write(&ne_softc[unit].ds_if, nestart, ior));
}

io_return_t
nesetinput(dev, receive_port, priority, filter, filter_count, device)
dev_t		dev;
ipc_port_t	receive_port;
int		priority;
filter_t	filter[];
unsigned int	filter_count;
device_t	device;
{
	register int unit = minor(dev);

	if (!ne_softc[unit].card_present || unit < 0 || unit >= NNE)
		return (ENXIO);

	return (net_set_filter(&ne_softc[unit].ds_if,
				receive_port, priority,
				filter, filter_count, device));
}

io_return_t
negetstat(dev, flavor, status, count)
dev_t		dev;
dev_flavor_t	flavor;
dev_status_t	status;
unsigned int	*count;
{
	register int unit = minor(dev);

	if (!ne_softc[unit].card_present || unit < 0 || unit >= NNE)
		return (ENXIO);

	return (net_getstat(&ne_softc[unit].ds_if,
			    flavor,
			    status,
			    count));
}

io_return_t
nesetstat(dev, flavor, status, count)
dev_t		dev;
dev_flavor_t	flavor;
dev_status_t	status;
unsigned int	count;
{
	register int unit = minor(dev);
	register ne_softc_t *ns;

	if (!ne_softc[unit].card_present || unit < 0 || unit >= NNE)
		return (ENXIO);

	ns = &ne_softc[unit];

	switch(flavor) {
	case NET_STATUS: {
		register struct net_status *s = (struct net_status *)status;
		int mode = 0;
		if (count < NET_STATUS_COUNT)
			return(D_INVALID_SIZE);
#define MOD_ENAL 1
#define MOD_PROM 2
		if (s->flags & IFF_ALLMULTI)
			mode |= MOD_ENAL;
		if (s->flags & IFF_PROMISC)
			mode |= MOD_PROM;

		if (ns->mode != mode) {
			ns->mode = mode;
			if (ns->ns_flags & DSF_RUNNING) {
				ns->ns_flags &= ~(DSF_LOCK | DSF_RUNNING);
				neinit(unit);
			}
		}
		break;
		}
	default :
		return (D_INVALID_OPERATION);
	}
	return (D_SUCCESS);
}


