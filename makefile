# See LICENSE for license details

SNCK_SRC_PATH =

LINENOISE = 1

# FEED = 1

ifdef FEED
FEED_SRC_PATH = $(SNCK_SRC_PATH)../feed/
include $(FEED_SRC_PATH)feed_project.mak
endif

include $(SNCK_SRC_PATH)snck_project.mak
