################ Build options #######################################

NAME		:= ustl
MAJOR		:= 2
MINOR		:= 5

#DEBUG		:= 1
#BUILD_SHARED	:= 1
BUILD_STATIC	:= 1
NOLIBSTDCPP	:= 1

################ Programs ############################################

CXX		:= avr-g++ -std=gnu++11
LD		:= avr-g++ -std=gnu++11
AR		:= ar
RANLIB		:= avr-ranlib
DOXYGEN		:= doxygen
INSTALL		:= install

INSTALLDATA	:= ${INSTALL} -D -p -m 644
INSTALLLIB	:= ${INSTALLDATA}
RMPATH		:= rmdir -p --ignore-fail-on-non-empty

################ Destination #########################################

INCDIR		:= ./include
LIBDIR		:= ./lib

################ Compiler options ####################################

WARNOPTS	:= -Wall -Wextra -Woverloaded-virtual -Wpointer-arith\
		-Wshadow -Wredundant-decls -Wcast-qual
TGTOPTS		:= -mmcu=atmega328p  -std=c++14 -I./include
INLINEOPTS	:= -fvisibility-inlines-hidden -fno-threadsafe-statics -fno-enforce-eh-specs

CXXFLAGS	:= ${WARNOPTS} ${TGTOPTS}
LDFLAGS		:= -L./lib
LIBS		:=
ifdef DEBUG
    CXXFLAGS	+= -O0 -g
    LDFLAGS	+= -rdynamic
else
    CXXFLAGS	+= -Os -g0 -DNDEBUG=1 -fomit-frame-pointer ${INLINEOPTS}
    LDFLAGS	+= -s -Wl,-gc-sections -static-libgcc
endif
ifdef NOLIBSTDCPP
    LD		:= /usr/bin/gcc
    STAL_LIBS	:= -lsupc++
    LIBS	:= ${STAL_LIBS}
endif
BUILDDIR	:= /tmp/Albert/make/${NAME}
# Warning: setting O makes a symlink under BUILDIR and it does not work under cygwin 
O		:=

slib_lnk	= lib$1.so
slib_son	= lib$1.so.${MAJOR}
slib_tgt	= lib$1.so.${MAJOR}.${MINOR}
slib_flags	= -shared -Wl,-soname=$1
