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
    $(SNCK_SRC_PATH)/_obj_snck_main.o \
    $(SNCK_SRC_PATH)/_obj_snck_info.o \
    $(SNCK_SRC_PATH)/_obj_snck_string.o \
    $(SNCK_SRC_PATH)/_obj_snck_heap.o \
    $(SNCK_SRC_PATH)/_obj_snck_passwd.o \
    $(SNCK_SRC_PATH)/_obj_snck_line.o \
    $(SNCK_SRC_PATH)/_obj_snck_prompt.o \
    $(SNCK_SRC_PATH)/_obj_snck_list.o \
    $(SNCK_SRC_PATH)/_obj_snck_history.o \
    $(SNCK_SRC_PATH)/_obj_snck_env.o \
    $(SNCK_SRC_PATH)/_obj_snck_file.o \
    $(SNCK_SRC_PATH)/_obj_snck_token.o \
    $(SNCK_SRC_PATH)/_obj_snck_opts.o \
    $(SNCK_SRC_PATH)/_obj_snck_suggest.o \
    $(SNCK_SRC_PATH)/_obj_snck_os.o

SNCK_LIBS =

SNCK_CFLAGS = -Wall -Wextra -pedantic -I$(SNCK_DST_PATH)

ifdef SNCK_DBG
SNCK_CFLAGS += -g -O0 -DSNCK_DBG
else
SNCK_CFLAGS += -s -O2 -Os
endif

ifdef LINENOISE
SNCK_CFLAGS += -DSNCK_HAVE_LINENOISE
SNCK_LIBS += -llinenoise
endif

ifdef FEED
SNCK_CFLAGS += -DSNCK_HAVE_FEED -I$(FEED_SRC_PATH)
SNCK_LIBS += $(FEED_OBJ_PATH)libfeed.a
endif

.PHONY : all
all : $(SNCK_DST_PATH)/snck

$(SNCK_DST_PATH)/snck : $(SNCK_SRCS)
	@echo linking $@
	@echo -o $@ $(SNCK_CFLAGS) $(SNCK_SRCS) $(SNCK_LIBS) > $(SNCK_DST_PATH)/_obj_snck.cmd
	@$(SNCK_CC) @$(SNCK_DST_PATH)/_obj_snck.cmd

# Build each object file
$(SNCK_DST_PATH)/_obj_%.o : $(SNCK_SRC_PATH)/%.c
	@echo compiling $@
	@echo -c -o $@ $(SNCK_CFLAGS) -MT $@ -MMD -MP -MF $@.d $< > $@.cmd
	@$(SNCK_CC) @$@.cmd

# Build the precompiled header
$(SNCK_DST_PATH)/snck : $(SNCK_DST_PATH)/snck_os.h.gch
$(SNCK_SRCS) : $(SNCK_DST_PATH)/snck_os.h.gch

$(SNCK_DST_PATH)/snck_os.h.gch : $(SNCK_SRC_PATH)/snck_os.h
	@echo generating $@
	@$(SNCK_CC) -c -o $@ $(SNCK_CFLAGS) $(SNCK_SRC_PATH)/snck_os.h

.PHONY : clean
clean: snck_clean

.PHONY: snck_clean
snck_clean:
	@echo cleanup snck
	-@rm -f $(SNCK_DST_PATH)/snck
	-@rm -f $(SNCK_DST_PATH)/_obj_*
	-@rm -f $(SNCK_DST_PATH)/*.gch

ifdef FEED
# Dependency on external library
$(SNCK_DST_PATH)/snck : $(FEED_OBJ_PATH)libfeed.a
endif

-include $(SNCK_DST_PATH)/_obj_*.o.d
