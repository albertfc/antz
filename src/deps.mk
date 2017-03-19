#
# This file is part of Antz.
# Copyright (C) 2017  Albert Farres
#
# Antz is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Antz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Antz.  If not, see <http://www.gnu.org/licenses/>.
#
ifeq (,$(filter $(MAKECMDGOALS),clean distclean mostlyclean maintainer-clean TAGS)) # It's a logical OR ;)
IGNORE  := $(shell mkdir -p $(DEPDIR))
DEPS    := $(patsubst %.S,$(DEPDIR)/%.d,$(ASMSOURCES))
DEPS    += $(patsubst %.c,$(DEPDIR)/%.d,$(CSOURCES))
DEPS    += $(patsubst %.cpp,$(DEPDIR)/%.d,$(CXXSOURCES))
endif

