# See LICENSE for license details

ifndef SNCK_DST_PATH
SNCK_DST_PATH = .
endif

ifndef SNCK_SRC_PATH
SNCK_SRC_PATH = .
endif

ifndef SNCK_CC
SNCK_CC = $(CC)
endif

SNCK_SRCS = \
    $(SNCK_SRC_PATH)/snck_main.c \
    $(SNCK_SRC_PATH)/snck_info.c \
    $(SNCK_SRC_PATH)/snck_string.c \
    $(SNCK_SRC_PATH)/snck_heap.c \
    $(SNCK_SRC_PATH)/snck_os.c

SNCK_LIBS =

SNCK_CFLAGS =

ifdef SNCK_DBG
SNCK_CFLAGS += -g -O0
else
SNCK_CFLAGS += -s -O2 -Os
endif

SNCK_CFLAGS += -DSNCK_HAVE_LINENOISE
SNCK_LIBS += -llinenoise

.PHONY : all
all : $(SNCK_DST_PATH)/snck

$(SNCK_DST_PATH)/snck : $(SNCK_SRCS)
	$(SNCK_CC) -o $@ $(SNCK_CFLAGS) $(SNCK_SRCS) $(SNCK_LIBS)

.PHONY : clean
clean:
	-rm $(SNCK_DST_PATH)/snck
