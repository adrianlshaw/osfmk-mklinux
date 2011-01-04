#
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

HP700_VPATH		= HP700
HP700_INCDIRS		= HP700

SFILES			= _start.o
CFILES                  = strcpy.o strlen.o strcmp.o strncmp.o \
			  strcat.o strncpy.o memcpy.o memmove.o memset.o \
			  strncat.o \
			  strspn.o strtol.o strtoul.o strtod.o \
			  ctype.o __main.o

.if ${OBJECT_FORMAT}=="ELF"
SFILES += milli.o
CFLAGS += -mportable-runtime 
.endif

${TARGET_MACHINE}_CFLAGS	= -DOLD_COMPAT

${TARGET_MACHINE}_OFILES	= ${SFILES} ${CFILES}

${SFILES:.o=.S}: $${.TARGET:.S=.s}
	${RM} ${_RMFLAGS_} ${.TARGET} ${.TARGET:.S=.pp}
	${CP} ${${.TARGET:.S=.s}:P} ${.TARGET}
	${_CC_} -E ${_CCFLAGS_} -DASSEMBLER ${.TARGET} \
		| ${SED} '/^#/d' > ${.TARGET:.S=.pp}	
	${RM} ${_RMFLAGS_} ${.TARGET}
	${MV} ${.TARGET:.S=.pp} ${.TARGET}

${SFILES}: $${.TARGET:.o=.S}
	${AS} ${.TARGET:.o=.S} -o ${.TARGET}


