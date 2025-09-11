# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external

ifneq (,$(shell which python3))
PYTHON	:= python3
else ifneq (,$(shell which python2))
PYTHON	:= python2
else ifneq (,$(shell which python))
PYTHON	:= python
else
$(error "Python not found in PATH, please install it.")
endif

# User config
# ===========

NAME		:= nds-bootstrap

GAME_TITLE	:= Combined ARM7+ARM9 template
GAME_SUBTITLE	:= Built with BlocksDS
GAME_AUTHOR	:= github.com/blocksds/sdk
GAME_ICON	:= icon.bmp

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:= sdroot
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:= image.bin

# Source code paths
# -----------------

# List of folders to combine into the root of NitroFS:
NITROFSDIR	:=

# Tools
# -----

MAKE		:= make
RM		:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Directories
# -----------

ARM9DIR		:= arm9
ARM7DIR		:= arm7

# Build artfacts
# --------------

ROM		:= $(NAME).nds

# Targets
# -------

.PHONY: all clean arm9 arm7 dldipatch sdimage

all: $(ROM)

# Include the targets after the `all` target so that the default target builds the complete package
include Makefile.targets

clean:
	@echo "  CLEAN"
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory
	$(V)$(foreach target,$(CARDENGINE_ARM7_DEPS),$(call cardengine7_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINE_ARM9_DEPS),$(call cardengine9_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINE_ARM9_IGM_DEPS),$(call cardengine9_igm_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM7_DEPS),$(call cardengine7i_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM7_DSIWARE_DEPS),$(call cardengine7i_dsiware_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM7_CHEAT_DEPS),$(call cardengine7i_cheat_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM9_DEPS) ,$(call cardengine9i_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM9_COLORLUT_DEPS),$(call cardengine9i_colorlut_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM9_DSIWARE_DEPS),$(call cardengine9i_dsiware_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(CARDENGINEI_ARM9_IGM_DEPS),$(call cardengine9i_igm_clean,$(subst cardengine_,,$(target))))
	$(V)$(foreach target,$(ASSETS),$(call assets_clean,$(target)))
	$(V)$(RM) $(ROM) build $(SDIMAGE)

arm9:
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory

arm7:
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory

ifneq ($(strip $(NITROFSDIR)),)
# Additional arguments for ndstool
NDSTOOL_ARGS	:= -d $(NITROFSDIR)

# Make the NDS ROM depend on the filesystem only if it is needed
$(ROM): $(NITROFSDIR)
endif

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

$(ROM): $(SUBTARGETS) arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_ARGS)

sdimage:
	@echo "  MKFATIMG $(SDIMAGE) $(SDROOT)"
	$(V)$(BLOCKSDS)/tools/mkfatimg/mkfatimg -t $(SDROOT) $(SDIMAGE)

dldipatch: $(ROM)
	@echo "  DLDIPATCH $(ROM)"
	$(V)$(BLOCKSDS)/tools/dldipatch/dldipatch patch \
		$(BLOCKSDS)/sys/dldi_r4/r4tf.dldi $(ROM)
