#
# Makefile
#
# OpenAL cross platform audio library
#
# Copyright (C) 1999-2000 by Authors.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA.
#

NAME = libAL.so
TYPE = SHARED

WARNINGS = ALL
OPTIMIZE = FULL

SYSTEM_INCLUDE_PATHS = ../include

LIBS = media be

SRCS = \
	Memory.c Table.c State.c Listener.c Buffer.c Source.c Environment.c \
	Context.c Extension.c Math.c Mixer.c Main.c alut.c \
	Thread.c Player.cpp

include /boot/develop/etc/makefile-engine

install: $(TARGET)
	cp $(TARGET) $(HOME)/config/lib/$(NAME)
