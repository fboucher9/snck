# See LICENSE for license details

SNCK_SRC_PATH = .

# LINENOISE = 1

FEED = 1

ifdef FEED
# PIC=0
# DBG=0
FEED_SRC_PATH = ../feed/
include $(FEED_SRC_PATH)feed_project.mak
endif

include $(SNCK_SRC_PATH)/snck_project.mak
