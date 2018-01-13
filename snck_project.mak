# See LICENSE for license details

ifndef DBG
DBG = 0
endif

ifeq ($(DBG),1)
SNCK_DBG = 1
endif

ifndef SNCK_CC
SNCK_CC = $(CC)
endif

ifdef SNCK_DBG
SNCK_OBJ_PATH = $(SNCK_DST_PATH).objchk/
else
SNCK_OBJ_PATH = $(SNCK_DST_PATH).objfre/
endif

SNCK_SRCS = \
    $(SNCK_OBJ_PATH)snck_main.o \
    $(SNCK_OBJ_PATH)snck_info.o \
    $(SNCK_OBJ_PATH)snck_string.o \
    $(SNCK_OBJ_PATH)snck_heap.o \
    $(SNCK_OBJ_PATH)snck_passwd.o \
    $(SNCK_OBJ_PATH)snck_line.o \
    $(SNCK_OBJ_PATH)snck_prompt.o \
    $(SNCK_OBJ_PATH)snck_list.o \
    $(SNCK_OBJ_PATH)snck_history.o \
    $(SNCK_OBJ_PATH)snck_env.o \
    $(SNCK_OBJ_PATH)snck_file.o \
    $(SNCK_OBJ_PATH)snck_token.o \
    $(SNCK_OBJ_PATH)snck_opts.o \
    $(SNCK_OBJ_PATH)snck_suggest.o \
    $(SNCK_OBJ_PATH)snck_os.o

SNCK_LIBS =

SNCK_CFLAGS = -Wall -Wextra -pedantic -I$(SNCK_OBJ_PATH)

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
all : $(SNCK_OBJ_PATH)snck

$(SNCK_OBJ_PATH)snck : $(SNCK_SRCS)
	@echo linking $@
	@echo -o $@ $(SNCK_CFLAGS) $(SNCK_SRCS) $(SNCK_LIBS) > $(SNCK_OBJ_PATH)snck.cmd
	@$(SNCK_CC) @$(SNCK_OBJ_PATH)snck.cmd

# Build each object file
$(SNCK_OBJ_PATH)%.o : $(SNCK_SRC_PATH)%.c
	@echo compiling $@
	@echo -c -o $@ $(SNCK_CFLAGS) -MT $@ -MMD -MP -MF $@.d $< > $@.cmd
	@$(SNCK_CC) @$@.cmd

# Build the precompiled header
$(SNCK_OBJ_PATH)snck : $(SNCK_OBJ_PATH)snck_os.h.gch
$(SNCK_SRCS) : $(SNCK_OBJ_PATH)snck_os.h.gch

$(SNCK_OBJ_PATH)snck_os.h.gch : $(SNCK_SRC_PATH)snck_os.h
	@echo generating $@
	@$(SNCK_CC) -c -o $@ $(SNCK_CFLAGS) $(SNCK_SRC_PATH)snck_os.h

# Dependency on obj folder
$(SNCK_OBJ_PATH)snck : | $(SNCK_OBJ_PATH)
$(SNCK_SRCS) : | $(SNCK_OBJ_PATH)
$(SNCK_OBJ_PATH)snck_os.h.gch : | $(SNCK_OBJ_PATH)
$(SNCK_OBJ_PATH) :
	@echo create $(SNCK_OBJ_PATH) folder
	-@mkdir -p $(SNCK_OBJ_PATH)

# Dependency on makefile
$(SNCK_OBJ_PATH)snck : $(SNCK_SRC_PATH)snck_project.mak
$(SNCK_SRCS) : $(SNCK_SRC_PATH)snck_project.mak
$(SNCK_OBJ_PATH)snck_os.h.gch : $(SNCK_SRC_PATH)snck_project.mak

.PHONY : clean
clean: snck_clean

.PHONY: snck_clean
snck_clean:
	@echo cleanup snck
	-rm -r -f $(SNCK_OBJ_PATH)

ifdef FEED
# Dependency on external library
$(SNCK_DST_PATH)/snck : $(FEED_OBJ_PATH)libfeed.a
endif

-include $(SNCK_DST_PATH)/_obj_*.o.d
