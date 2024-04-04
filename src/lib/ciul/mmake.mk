# SPDX-License-Identifier: BSD-2-Clause
# X-SPDX-Copyright-Text: (c) Copyright 2003-2020 Xilinx, Inc.
ifeq ($(UNIX),1)
# Order important for GNU make 3.82 unless MMAKE_NO_DEPS=1. SFC bug 27495.
TARGETS		:= $(EFVI_LIB) $(CIUL_LIB)
TARGETS		+= $(CIUL_REALNAME) $(CIUL_SONAME) $(CIUL_LINKNAME)
else
TARGETS		:= $(CIUL_LIB)
endif
MMAKE_TYPE	:= LIB

# Standalone subset for descriptor munging only.
EFVI_SRCS	:=		\
		pt_tx.c		\
		pt_rx.c		\
		vi_init.c	\
		ef10_event.c	\
		ef10_vi.c	\
		ef100_event.c	\
		ef100_vi.c      \
		efxdp_vi.c      \
		efct_vi.c       \
		efct_kbufs.c    \
		ef10ct_vi.c

LIB_SRCS	:=		\
		$(EFVI_SRCS)	\
		ef10_evtimer.c	\
		logging.c \
		checksum.c

ifneq ($(DRIVER),1)
LIB_SRCS	+=		\
		open.c		\
		event_q.c	\
		event_q_put.c	\
		pt_endpoint.c	\
		filter.c	\
		vi_set.c	\
		memreg.c	\
		pd.c		\
		ef_app_cluster.c	\
		pio.c		\
		ef10_evtimer.c  \
		vi_layout.c	\
		vi_stats.c	\
		vi_prime.c	\
		capabilities.c	\
		ctpio.c         \
		shrub_pool.c    \
		shrub_server.c  \

# librt is needed on old glibc, e.g. on RHEL 6
MMAKE_DIR_LINKFLAGS	:= $(MMAKE_DIR_LINKFLAGS) -lrt
endif

ifdef MMAKE_USE_KBUILD
objd	:= $(obj)/
else
objd	:=
endif

ifndef MMAKE_NO_RULES
MMAKE_OBJ_PREFIX := ci_ul_
endif

################################################
# Autogenerated file with Onload version strings
#

OO_VERSION_HDR := $(objd)onload_version.h

OO_VERSION_GEN := $(SRCPATH)/../scripts/onload_version_gen


# In case of UL build dependency tracking is done by mmake, and it is
# unable to detect dependency for onload_version.h.
# So we remove onload_version.o and library when the version changes.
make_oo_version: $(OO_VERSION_GEN) FORCE
	@echo "  GENERATE $(OO_VERSION_HDR)"
	T=$$(mktemp -p $(objd).); \
	(cd $(SRCPATH); $(OO_VERSION_GEN)) > $$T; \
	if diff -q $(OO_VERSION_HDR) $$T 2>/dev/null; \
		then echo "Not updating onload_version.h"; \
		else echo "Updated onload_version.h"; \
		     mv $$T $(OO_VERSION_HDR); \
	fi; \
	rm -f $$T

# We want to re-build the version header every time
.PHONY: make_oo_version

FORCE:

$(OO_VERSION_HDR): make_oo_version
$(objd)$(MMAKE_OBJ_PREFIX)vi_init.o: $(OO_VERSION_HDR)


######################################################################
# Autogenerated header for checking user/kernel interface consistency.
#
_EFCH_INTF_HDRS	:= ci/efch/op_types.h etherfabric/internal/efct_uk_api.h
EFCH_INTF_HDRS	:= $(_EFCH_INTF_HDRS:%=$(SRCPATH)/include/%)

$(objd)efch_intf_ver.h: $(EFCH_INTF_HDRS)
	echo "  GENERATE $@"
	@md5=$$(cat $(EFCH_INTF_HDRS) | grep -v '^[ *]\*' | \
		md5sum | sed 's/ .*//'); \
	echo "#define EFCH_INTF_VER  \"$$md5\"" >"$@"

$(objd)$(MMAKE_OBJ_PREFIX)pt_endpoint.o: $(objd)efch_intf_ver.h
$(objd)$(MMAKE_OBJ_PREFIX)vi_init.o: $(objd)efch_intf_ver.h


######################################################
# UL library
#
ifndef MMAKE_NO_RULES

EFVI_OBJS	 := $(EFVI_SRCS:%.c=$(MMAKE_OBJ_PREFIX)%.o)
LIB_OBJS	 := $(LIB_SRCS:%.c=$(MMAKE_OBJ_PREFIX)%.o)


all: $(TARGETS)

lib: $(TARGETS)

clean:
	@$(MakeClean)
	rm -f efch_intf_ver.h $(OO_VERSION_HDR)

$(CIUL_LIB): $(LIB_OBJS)
	$(MMakeLinkStaticLib)

$(EFVI_LIB): $(EFVI_OBJS)
	$(MMakeLinkStaticLib)

$(CIUL_REALNAME): $(LIB_OBJS)
	@(soname="$(CIUL_SONAME)"; $(MMakeLinkDynamicLib))

$(CIUL_SONAME): $(CIUL_REALNAME)
	ln -fs $(shell basename $^) $@

$(CIUL_LINKNAME): $(CIUL_REALNAME)
	ln -fs $(shell basename $^) $@

endif


######################################################
# linux kbuild support
#
ifdef MMAKE_USE_KBUILD

lib_obj = ci_ul_lib.o
lib_obj_path = $(BUILDPATH)/lib/ciul

lib_obj_cmd = $(LD) -r $(LIB_SRCS:%.c=%.o) -o $(lib_obj)
all:
	$(MAKE) $(MMAKE_KBUILD_ARGS) KBUILD_BUILTIN=1 KBUILD_EXTMOD=$(lib_obj_path) $(KBUILD_LIB_MAKE_TRG)
	$(lib_obj_cmd)
	echo "cmd_$(lib_obj_path)/$(lib_obj) := $(lib_obj_cmd)" > .$(lib_obj).cmd
clean:
	@$(MakeClean)
	rm -f ci_ul_lib.o efch_intf_ver.h $(OO_VERSION_HDR)
endif

ifdef MMAKE_IN_KBUILD
LIB_OBJS := $(LIB_SRCS:%.c=%.o)
obj-y    := $(LIB_OBJS)
endif
