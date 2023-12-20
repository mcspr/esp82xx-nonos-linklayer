include makefiles/include/common.mk
include makefiles/include/version.mk

.PHONY: upstream-arduino arduino upstream-open-sdk open-sdk upstream-lwip clean-lwip patch-lwip patch-lwip-open all
.DEFAULT: all

LWIP_GIT_REPO = https://github.com/lwip-tcpip/lwip
PATCHES = $(wildcard $(ROOT)/patches/*.patch)

all: arduino

upstream-lwip:
	@if test ! -e "$(LWIP_ROOT)" ; then \
		@mkdir $(LWIP_ROOT); \
		@git -C $(LWIP_ROOT) init ; \
		@git -C $(LWIP_ROOT) remote add origin $(LWIP_GIT_REPO) ; \
	fi

	@git -C $(LWIP_ROOT) fetch origin $(UPSTREAM_VERSION)

clean-lwip:
	@git -C $(LWIP_ROOT) clean -f
	@git -C $(LWIP_ROOT) checkout -f $(UPSTREAM_VERSION)

ifeq ($(V), 0)
VERBPATCH = @echo "PATCH $$p";
else
VERBPATCH =
endif

PATCH = $(VERBPATCH) patch

patch-lwip:
	@for p in $(PATCHES); do \
		$(PATCH) -d $(LWIP_ROOT) -p1 < $$p; \
	done

patch-lwip-open:
	$(PATCH) -d $(LWIP_ROOT) -p1 < $(ROOT)/patches/open/sdk-mem-macros.patch

lwip:
	$(MAKE) update-lwip
	$(MAKE) patch-lwip

upstream-open-sdk:
	$(MAKE) upstream-lwip
	$(MAKE) clean-lwip
	$(MAKE) patch-lwip
	$(MAKE) patch-lwip-open
	$(MAKE) open-sdk

open-sdk:
	$(MAKE) -f Makefile.open clean
	$(MAKE) -f Makefile.open install

upstream-arduino:
	$(MAKE) upstream-lwip
	$(MAKE) clean-lwip
	$(MAKE) patch-lwip
	$(MAKE) arduino

arduino:
	$(MAKE) -f Makefile.arduino clean
	$(MAKE) -f Makefile.arduino install
