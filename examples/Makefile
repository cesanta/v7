# Copyright (c) 2015 Cesanta Software
# All rights reserved

SUBDIRS = $(sort $(dir $(wildcard */)))

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@

clean:
	for d in $(SUBDIRS) ; do $(MAKE) -C $$d clean ; done
