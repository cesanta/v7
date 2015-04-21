# Copyright (c) 2014 Cesanta Software
# All rights reserved

DOCGEN=../../tools/docgen.py
DOCGEN_DIR=../../tools/html_doc_files
GEN_ADOCS = c_api.adoc
ADOCS = $(wildcard *.adoc) $(GEN_ADOCS)

all: index.html $(ADOCS)

c_api.adoc: ../v7.h
	$(DOCGEN) -o $@ $<

# deps:
# gem install pygments.rb
# gem install pygmentize
index.html: $(ADOCS) Makefile $(DOCGEN_DIR)/style.css index-docinfo.html
	asciidoctor -b html5 \
	  -d book \
	  -a icons \
	  -a data-uri \
	  -a toc \
	  -a imagesdir=$(DOCGEN_DIR)/images \
	  -a stylesheet=$(DOCGEN_DIR)/style.css \
	  index.adoc -o index.html

index-docinfo.html: $(DOCGEN_DIR)/doc.js $(DOCGEN_DIR)/jquery.min.js Makefile
	(echo '<script>'; cat $(DOCGEN_DIR)/jquery.min.js $(DOCGEN_DIR)/jquery.scrollTo.min.js $(DOCGEN_DIR)/doc.js ; echo '</script>') > $@

clean:
	rm -rf $(GEN_ADOCS) index-docinfo.html
