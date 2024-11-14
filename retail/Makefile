#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

export TARGET := nds-bootstrap
export TOPDIR := $(CURDIR)

# GMAE_ICON is the image used to create the game icon, leave blank to use default rule
GAME_ICON :=

# specify a directory which contains the nitro filesystem
# this is relative to the Makefile
NITRO_FILES := nitrofiles

# These set the information text in the nds file
GAME_TITLE     := nds-bootstrap
GAME_SUBTITLE1 := ahezard & Rocket Robz

include $(DEVKITARM)/ds_rules

BIN		:=	bin

.PHONY: all checkarm7 checkarm9 dist release nightly bootloader bootloader1 bootloader2 bootloaderi bootloaderi1 bootloaderi2 cardengine_arm7 cardengine_arm7_music cardengine_arm9_igm cardengine_arm9_igm_extmem cardengine_arm9 cardengine_arm9_32 cardengine_arm9_start cardengine_arm9_start_foto cardengine_arm9_alt cardengine_arm9_alt_gsdd cardengine_arm9_alt2 cardengine_arm9_alt3 cardengine_arm9_extmem cardengine_arm9_extmem_gsdd cardengine_arm9_extmem_start cardengine_arm9_extmem_foto cardenginei_arm7 cardenginei_arm7_alt cardenginei_arm7_twlsdk cardenginei_arm7_twlsdk3 cardenginei_arm7_dsiware cardenginei_arm7_dsiware3 cardenginei_arm7_cheat cardenginei_arm9_igm cardenginei_arm9 cardenginei_arm9_sdk20 cardenginei_arm9_dlp cardenginei_arm9_gsdd cardenginei_arm9_dldi cardenginei_arm9_sdk20_dldi cardenginei_arm9_gsdd_dldi cardenginei_arm9_twlsdk cardenginei_arm9_twlsdk3 cardenginei_arm9_twlsdk_dldi cardenginei_arm9_twlsdk3_dldi cardenginei_arm9_dsiware cardenginei_arm9_dsiware3 preLoadSettings apfix libnds32 arm7/$(TARGET).elf arm9/$(TARGET).elf clean

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: checkarm7 checkarm9 $(TARGET).nds

release:	$(TARGET).nds
	@rm -f $(CURDIR)/$(BIN)/$(TARGET)-release.nds
	@mv $< $(CURDIR)/$(BIN)/$(TARGET)-release.nds

nightly:	$(TARGET).nds
	@rm -f $(CURDIR)/$(BIN)/$(TARGET)-nightly.nds
	@mv $< $(CURDIR)/$(BIN)/$(TARGET)-nightly.nds

#---------------------------------------------------------------------------------
checkarm7:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
checkarm9:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
$(TARGET).nds : $(BIN) $(NITRO_FILES) arm7/$(TARGET).elf arm9/$(TARGET).elf
	ndstool	-c $(OUTPUT) -7 arm7/$(TARGET).elf -9 arm9/$(TARGET).elf -d $(NITRODATA) \
			-b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1)" \
			-g KBSE 01 "NDSBOOTSTRAP" -z 80040000 -u 00030004 -a 00000138 -p 00000001

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf: bootloader bootloader1 bootloader2 bootloaderi bootloaderi1 bootloaderi2 preLoadSettings apfix libnds32
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------		
libnds32:
	@$(MAKE) -C libnds32 release

#---------------------------------------------------------------------------------		
bootloader: cardengine_arm7 cardengine_arm7_music cardengine_arm9_igm cardengine_arm9_igm_extmem cardengine_arm9 cardengine_arm9_32 cardengine_arm9_start cardengine_arm9_start_foto cardengine_arm9_alt cardengine_arm9_alt_gsdd cardengine_arm9_alt2 cardengine_arm9_alt3 cardengine_arm9_extmem cardengine_arm9_extmem_gsdd cardengine_arm9_extmem_start cardengine_arm9_extmem_foto
	@$(MAKE) -C bootloader

#---------------------------------------------------------------------------------		
bootloader1:
	@$(MAKE) -C bootloader1

#---------------------------------------------------------------------------------		
bootloader2:
	@$(MAKE) -C bootloader2

#---------------------------------------------------------------------------------		
bootloaderi: cardenginei_arm7 cardenginei_arm7_alt cardenginei_arm7_twlsdk cardenginei_arm7_twlsdk3 cardenginei_arm7_dsiware cardenginei_arm7_dsiware3 cardenginei_arm7_cheat cardenginei_arm9_igm cardenginei_arm9 cardenginei_arm9_sdk20 cardenginei_arm9_dlp cardenginei_arm9_gsdd cardenginei_arm9_dldi cardenginei_arm9_sdk20_dldi cardenginei_arm9_gsdd_dldi cardenginei_arm9_twlsdk cardenginei_arm9_twlsdk3 cardenginei_arm9_twlsdk_dldi cardenginei_arm9_twlsdk3_dldi cardenginei_arm9_dsiware cardenginei_arm9_dsiware3
	@$(MAKE) -C bootloaderi

#---------------------------------------------------------------------------------		
bootloaderi1:
	@$(MAKE) -C bootloaderi1

#---------------------------------------------------------------------------------		
bootloaderi2:
	@$(MAKE) -C bootloaderi2

#---------------------------------------------------------------------------------		
cardengine_arm7:
	@$(MAKE) -C cardengine/arm7

#---------------------------------------------------------------------------------		
cardengine_arm7_music:
	@$(MAKE) -C cardengine/arm7_music

#---------------------------------------------------------------------------------		
cardengine_arm9_igm:
	@$(MAKE) -C cardengine/arm9_igm

#---------------------------------------------------------------------------------		
cardengine_arm9_igm_extmem:
	@$(MAKE) -C cardengine/arm9_igm_extmem

#---------------------------------------------------------------------------------		
cardengine_arm9:
	@$(MAKE) -C cardengine/arm9

#---------------------------------------------------------------------------------		
cardengine_arm9_32:
	@$(MAKE) -C cardengine/arm9_32

#---------------------------------------------------------------------------------		
cardengine_arm9_start:
	@$(MAKE) -C cardengine/arm9_start

#---------------------------------------------------------------------------------		
cardengine_arm9_start_foto:
	@$(MAKE) -C cardengine/arm9_start_foto

#---------------------------------------------------------------------------------		
cardengine_arm9_alt:
	@$(MAKE) -C cardengine/arm9_alt

#---------------------------------------------------------------------------------		
cardengine_arm9_alt_gsdd:
	@$(MAKE) -C cardengine/arm9_alt_gsdd

#---------------------------------------------------------------------------------		
cardengine_arm9_alt2:
	@$(MAKE) -C cardengine/arm9_alt2

#---------------------------------------------------------------------------------		
cardengine_arm9_alt3:
	@$(MAKE) -C cardengine/arm9_alt3

#---------------------------------------------------------------------------------		
cardengine_arm9_extmem:
	@$(MAKE) -C cardengine/arm9_extmem

#---------------------------------------------------------------------------------		
cardengine_arm9_extmem_gsdd:
	@$(MAKE) -C cardengine/arm9_extmem_gsdd

#---------------------------------------------------------------------------------		
cardengine_arm9_extmem_start:
	@$(MAKE) -C cardengine/arm9_extmem_start

#---------------------------------------------------------------------------------		
cardengine_arm9_extmem_foto:
	@$(MAKE) -C cardengine/arm9_extmem_foto

#---------------------------------------------------------------------------------		
cardenginei_arm7:
	@$(MAKE) -C cardenginei/arm7

#---------------------------------------------------------------------------------		
cardenginei_arm7_alt:
	@$(MAKE) -C cardenginei/arm7_alt

#---------------------------------------------------------------------------------		
cardenginei_arm7_twlsdk:
	@$(MAKE) -C cardenginei/arm7_twlsdk

#---------------------------------------------------------------------------------		
cardenginei_arm7_twlsdk3:
	@$(MAKE) -C cardenginei/arm7_twlsdk3

#---------------------------------------------------------------------------------		
cardenginei_arm7_dsiware:
	@$(MAKE) -C cardenginei/arm7_dsiware

#---------------------------------------------------------------------------------		
cardenginei_arm7_dsiware3:
	@$(MAKE) -C cardenginei/arm7_dsiware3

#---------------------------------------------------------------------------------		
cardenginei_arm7_cheat:
	@$(MAKE) -C cardenginei/arm7_cheat

#---------------------------------------------------------------------------------		
cardenginei_arm9_igm:
	@$(MAKE) -C cardenginei/arm9_igm

#---------------------------------------------------------------------------------		
cardenginei_arm9:
	@$(MAKE) -C cardenginei/arm9

#---------------------------------------------------------------------------------		
cardenginei_arm9_sdk20:
	@$(MAKE) -C cardenginei/arm9_sdk20

#---------------------------------------------------------------------------------		
cardenginei_arm9_dlp:
	@$(MAKE) -C cardenginei/arm9_dlp

#---------------------------------------------------------------------------------		
cardenginei_arm9_gsdd:
	@$(MAKE) -C cardenginei/arm9_gsdd

#---------------------------------------------------------------------------------		
cardenginei_arm9_dldi:
	@$(MAKE) -C cardenginei/arm9_dldi

#---------------------------------------------------------------------------------		
cardenginei_arm9_sdk20_dldi:
	@$(MAKE) -C cardenginei/arm9_sdk20_dldi

#---------------------------------------------------------------------------------		
cardenginei_arm9_gsdd_dldi:
	@$(MAKE) -C cardenginei/arm9_gsdd_dldi

#---------------------------------------------------------------------------------		
cardenginei_arm9_twlsdk:
	@$(MAKE) -C cardenginei/arm9_twlsdk

#---------------------------------------------------------------------------------		
cardenginei_arm9_twlsdk3:
	@$(MAKE) -C cardenginei/arm9_twlsdk3

#---------------------------------------------------------------------------------		
cardenginei_arm9_twlsdk_dldi:
	@$(MAKE) -C cardenginei/arm9_twlsdk_dldi

#---------------------------------------------------------------------------------		
cardenginei_arm9_twlsdk3_dldi:
	@$(MAKE) -C cardenginei/arm9_twlsdk3_dldi

#---------------------------------------------------------------------------------		
cardenginei_arm9_dsiware:
	@$(MAKE) -C cardenginei/arm9_dsiware

#---------------------------------------------------------------------------------		
cardenginei_arm9_dsiware3:
	@$(MAKE) -C cardenginei/arm9_dsiware3

#---------------------------------------------------------------------------------		
preLoadSettings:
	@$(MAKE) -C preLoadSettings

#---------------------------------------------------------------------------------		
apfix:
	@$(MAKE) -C apfix

#---------------------------------------------------------------------------------
#$(BUILD):
#	@[ -d $@ ] || mkdir -p $@
#	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).nds.orig.nds $(TARGET).arm9 $(BIN)
	@$(MAKE) -C libnds32 clean
	@$(MAKE) -C arm7 clean
	@$(MAKE) -C arm9 clean
	@$(MAKE) -C cardengine/arm7 clean
	@$(MAKE) -C cardengine/arm7_music clean
	@$(MAKE) -C cardengine/arm9_igm clean
	@$(MAKE) -C cardengine/arm9_igm_extmem clean
	@$(MAKE) -C cardengine/arm9 clean
	@$(MAKE) -C cardengine/arm9_start clean
	@$(MAKE) -C cardengine/arm9_start_foto clean
	@$(MAKE) -C cardengine/arm9_alt clean
	@$(MAKE) -C cardengine/arm9_alt_gsdd clean
	@$(MAKE) -C cardengine/arm9_alt2 clean
	@$(MAKE) -C cardengine/arm9_alt3 clean
	@$(MAKE) -C cardengine/arm9_extmem clean
	@$(MAKE) -C cardengine/arm9_extmem_gsdd clean
	@$(MAKE) -C cardengine/arm9_extmem_start clean
	@$(MAKE) -C cardengine/arm9_extmem_foto clean
	@$(MAKE) -C cardenginei/arm7 clean
	@$(MAKE) -C cardenginei/arm7_alt clean
	@$(MAKE) -C cardenginei/arm7_twlsdk clean
	@$(MAKE) -C cardenginei/arm7_twlsdk3 clean
	@$(MAKE) -C cardenginei/arm7_dsiware clean
	@$(MAKE) -C cardenginei/arm7_dsiware3 clean
	@$(MAKE) -C cardenginei/arm7_cheat clean
	@$(MAKE) -C cardenginei/arm7_cheatonly clean
	@$(MAKE) -C cardenginei/arm9_igm clean
	@$(MAKE) -C cardenginei/arm9 clean
	@$(MAKE) -C cardenginei/arm9_sdk20 clean
	@$(MAKE) -C cardenginei/arm9_dlp clean
	@$(MAKE) -C cardenginei/arm9_gsdd clean
	@$(MAKE) -C cardenginei/arm9_dldi clean
	@$(MAKE) -C cardenginei/arm9_sdk20_dldi clean
	@$(MAKE) -C cardenginei/arm9_gsdd_dldi clean
	@$(MAKE) -C cardenginei/arm9_twlsdk clean
	@$(MAKE) -C cardenginei/arm9_twlsdk_dldi clean
	@$(MAKE) -C cardenginei/arm9_twlsdk3 clean
	@$(MAKE) -C cardenginei/arm9_twlsdk3_dldi clean
	@$(MAKE) -C cardenginei/arm9_dsiware clean
	@$(MAKE) -C cardenginei/arm9_dsiware3 clean
	@rm -f $(NITRODATA)/preLoadSettingsMEP.pck
	@rm -f $(NITRODATA)/preLoadSettingsDSi.pck
	@rm -f $(NITRODATA)/preLoadSettings3DS.pck
	@rm -f $(NITRODATA)/apfix.pck
	@$(MAKE) -C bootloader clean
	@$(MAKE) -C bootloader1 clean
	@$(MAKE) -C bootloader2 clean
	@$(MAKE) -C bootloaderi clean
	@$(MAKE) -C bootloaderi1 clean
	@$(MAKE) -C bootloaderi2 clean

$(BIN):
	@mkdir -p $(BIN)
