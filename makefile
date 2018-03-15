# See LICENSE for license details

DBG ?= 0

PIC ?= 1

# LINENOISE = 1

FEED = 1

SNCK_SRC_PATH =

ifdef FEED
FEED_SRC_PATH = $(SNCK_SRC_PATH)../feed/
include $(FEED_SRC_PATH)feed_project.mak
endif

include $(SNCK_SRC_PATH)snck_project.mak
