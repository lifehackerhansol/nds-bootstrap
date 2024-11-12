# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

export BLOCKSDS			?= /opt/blocksds/core
export BLOCKSDSEXT		?= /opt/blocksds/external

export WONDERFUL_TOOLCHAIN	?= /opt/wonderful
ARM_NONE_EABI_PATH	?= $(WONDERFUL_TOOLCHAIN)/toolchain/gcc-arm-none-eabi/bin/

# Source code paths
# -----------------

CARDENGINE	?= arm9_extmem
SOURCEDIRS	:= cardengine/arm9/source common/source common/source_general
INCLUDEDIRS	:= cardengine/arm9/include common/include common/include_fat
BINDIRS		:=

# Defines passed to all files
# ---------------------------

DEFINES		:= -DARM9 -DB4DS -DNO_CARDID -D_NO_SDMMC -DNOGETBOOTCLUSTER -DEXTMEM

# Libraries
# ---------

LIBS		:=
LIBDIRS		:= $(BLOCKSDS)/libs/libnds

# Build artifacts
# -----------------

NAME		:= cardengine_$(CARDENGINE)
BUILDDIR	:= build/$(NAME)
LZ77    := nitrofiles/$(NAME).lz77
BIN     := build/$(NAME).bin
ELF		:= build/$(NAME).elf
DUMP		:= build/$(NAME).dump
MAP		:= build/$(NAME).map

# Tools
# -----

PREFIX		:= $(ARM_NONE_EABI_PATH)arm-none-eabi-
CC		:= $(PREFIX)gcc
CXX		:= $(PREFIX)g++
LD		:= $(PREFIX)gcc
OBJCOPY		:= $(PREFIX)objcopy
OBJDUMP		:= $(PREFIX)objdump
MKDIR		:= mkdir
RM		:= rm -rf
LZSS	:=  $(WONDERFUL_TOOLCHAIN)/bin/wf-nnpack-lzss

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Source files
# ------------

ifneq ($(BINDIRS),)
    SOURCES_BIN	:= $(shell find -L $(BINDIRS) -name "*.bin")
    INCLUDEDIRS	+= $(addprefix $(BUILDDIR)/,$(BINDIRS))
endif

SOURCES_S	:= $(shell find -L $(SOURCEDIRS) -name "*.s")
SOURCES_C	:= $(shell find -L $(SOURCEDIRS) -name "*.c")
SOURCES_CPP	:= $(shell find -L $(SOURCEDIRS) -name "*.cpp")

# Compiler and linker flags
# -------------------------

ARCH		:= -mthumb -mthumb-interwork -mcpu=arm946e-s+nofp

WARNFLAGS	:= -Wall

INCLUDEFLAGS	:= $(foreach path,$(INCLUDEDIRS),-I$(path)) \
		   $(foreach path,$(LIBDIRS),-I$(path)/include)

LIBDIRSFLAGS	:= $(foreach path,$(LIBDIRS),-L$(path)/lib)

ASFLAGS		+= -x assembler-with-cpp $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH)

CFLAGS		+= -std=gnu17 $(WARNFLAGS) $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH) -Os

CXXFLAGS	+= -std=gnu++17 $(WARNFLAGS) $(DEFINES) $(INCLUDEFLAGS) \
		   $(ARCH) -Os \
		   -fno-exceptions -fno-rtti

LDFLAGS		:= $(ARCH) $(LIBDIRSFLAGS) -Wl,-Map,$(MAP) $(DEFINES) \
		   -Wl,--start-group $(LIBS) -Wl,--end-group -nostartfiles

# Intermediate build files
# ------------------------

OBJS_ASSETS	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

HEADERS_ASSETS	:= $(patsubst %.bin,%_bin.h,$(addprefix $(BUILDDIR)/,$(SOURCES_BIN)))

OBJS_SOURCES	:= $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_S))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_C))) \
		   $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(SOURCES_CPP)))

OBJS		:= $(OBJS_ASSETS) $(OBJS_SOURCES)

DEPS		:= $(OBJS:.o=.d)

# Targets
# -------

.PHONY: all clean dump

all: $(LZ77)

$(LZ77): $(BIN)
	@echo "  LZSS.9    $@"
	$(V)$(LZSS) -ewn $< $@

$(BIN): $(ELF)
	@echo "  OBJCOPY.9 $@"
	$(V)$(OBJCOPY) -O binary $< $@

$(ELF): $(BUILDDIR)/cardengine.ld $(OBJS)
	@echo "  LD.9    $@"
	$(V)$(LD) -o $@ $(OBJS) $(LDFLAGS) -T $(BUILDDIR)/cardengine.ld

$(DUMP): $(ELF)
	@echo "  OBJDUMP.9 $@"
	$(V)$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	@echo "  CLEAN.9"
	$(V)$(RM) $(LZ77) $(BIN) $(ELF) $(DUMP) $(MAP) $(BUILDDIR)

# Rules
# -----

$(BUILDDIR)/cardengine.ld:	cardengine/$(CARDENGINE)/cardengine.ld.in
	@echo "  CPP.9   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CPP) $(INCLUDEFLAGS) $< > $@

$(BUILDDIR)/%.s.o : %.s
	@echo "  AS.9    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.c.o : %.c
	@echo "  CC.9    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.c.o : %.arm.c
	@echo "  CC.9    $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.cpp.o : %.cpp
	@echo "  CXX.9   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILDDIR)/%.arm.cpp.o : %.arm.cpp
	@echo "  CXX.9   $<"
	@$(MKDIR) -p $(@D)
	$(V)$(CXX) $(CXXFLAGS) -MMD -MP -marm -mlong-calls -c -o $@ $<

$(BUILDDIR)/%.bin.o $(BUILDDIR)/%_bin.h : %.bin
	@echo "  BIN2C.9 $<"
	@$(MKDIR) -p $(@D)
	$(V)$(BLOCKSDS)/tools/bin2c/bin2c $< $(@D)
	$(V)$(CC) $(CFLAGS) -MMD -MP -c -o $(BUILDDIR)/$*.bin.o $(BUILDDIR)/$*_bin.c

# All assets must be built before the source code
# -----------------------------------------------

$(SOURCES_S) $(SOURCES_C) $(SOURCES_CPP): $(HEADERS_ASSETS)

# Include dependency files if they exist
# --------------------------------------

-include $(DEPS)
