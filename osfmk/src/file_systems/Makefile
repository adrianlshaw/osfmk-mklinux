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
#
# MkLinux

PURE_MACH		= 1

VPATH			= ${${TARGET_MACHINE}_VPATH}:${TARGET_MACHINE}

EXPINC_SUBDIRS		= include

.if exists(${TARGET_MACHINE}/Makefile)
EXPLIB_SUBDIRS		= ${TARGET_MACHINE}
SUBDIRS			= ${TARGET_MACHINE}
.endif

INCFLAGS		+= -I.

LIBRARIES		= libsa_fs.a

USE_STATIC_LIBRARIES	=

EXPLIB_TARGETS		= export_libsa_fs.a

ILIST			= ${LIBRARIES}
IDIR			= ${MACH3_LIBRARY_IDIR}
IMODE			= 644

CFLAGS			= ${${OBJECT_FORMAT}_EXTRA_WARNINGS} ${WERROR}

OFILES			= file_system.o \
			  ${${TARGET_MACHINE}_OFILES}

DEPENDENCIES		=

.if exists(${TARGET_MACHINE}/machdep.mk)
.include "${TARGET_MACHINE}/machdep.mk"
.endif
.include <${RULES_MK}>

.if exists(depend.mk)
.include "depend.mk"
.endif



