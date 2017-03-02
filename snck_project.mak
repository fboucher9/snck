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
    $(SNCK_SRC_PATH)/snck_os.c

.PHONY : all
all : $(SNCK_DST_PATH)/snck

$(SNCK_DST_PATH)/snck : $(SNCK_SRCS)
	$(SNCK_CC) -g -O0 -o $@ $(SNCK_SRCS) -llinenoise

.PHONY : clean
clean:
	-rm $(SNCK_DST_PATH)/snck
