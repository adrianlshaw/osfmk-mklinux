# Copyright 1991-1998 by Open Software Foundation, Inc. 
#              All Rights Reserved 
#  
# Permission to use, copy, modify, and distribute this software and 
# its documentation for any purpose and without fee is hereby granted, 
# provided that the above copyright notice appears in all copies and 
# that both the copyright notice and this permission notice appear in 
# supporting documentation. 
#  
# OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE. 
#  
# IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
# LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
# NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
# 
# MkLinux

${TARGET_MACHINE}_VPATH		= HP700:hp_pa

${TARGET_MACHINE}_OFILES	= set_regs.o boot_dep.o som.o elf.o \
				  formats.o _start.o ${SFILES}

${SFILES:.o=.S}: $${.TARGET:.S=.s}
	${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET:.S=.pp}
	${CP} ${${.TARGET:.S=.s}:P} ${.TARGET}
	${_traditional_CC_} -E ${_CCFLAGS_} -DASSEMBLER ${.TARGET} \
		> ${.TARGET:.S=.pp}
	${RM} ${_RMFLAGS_} ${.TARGET}
	${MV} ${.TARGET:.S=.pp} ${.TARGET}

${SFILES}: $${.TARGET:.o=.S}
	${AS} ${.TARGET:.o=.S} -o ${.TARGET}




