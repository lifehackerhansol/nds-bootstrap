#include <nds/system.h>
#include <nds/bios.h>
#include "nds_header.h"
#include "module_params.h"
#include "patch.h"
#include "find.h"
#include "common.h"
#include "value_bits.h"
#include "locations.h"
#include "tonccpy.h"
#include "cardengine_header_arm7.h"
#include "debug_file.h"

extern u8 arm7newUnitCode;
extern u32 newArm7binarySize;
extern u32 arm7mbk;

u32 savePatchV1(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, u32 saveFileCluster);
u32 savePatchV2(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, u32 saveFileCluster);
u32 savePatchUniversal(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, u32 saveFileCluster);
u32 savePatchInvertedThumb(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, u32 saveFileCluster);
u32 savePatchV5(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, u32 saveFileCluster); // SDK 5

u32 generateA7Instr(int arg1, int arg2) {
	return (((u32)(arg2 - arg1 - 8) >> 2) & 0xFFFFFF) | 0xEB000000;
}

void setB(int arg1, int arg2) {
	*(u32*)arg1 = (((u32)(arg2 - arg1 - 8) >> 2) & 0xFFFFFF) | 0xEA000000;
}

void setBL(int arg1, int arg2) {
	*(u32*)arg1 = (((u32)(arg2 - arg1 - 8) >> 2) & 0xFFFFFF) | 0xEB000000;
}

void setBLX(int arg1, int arg2) {
	*(u32*)arg1 = (((u32)(arg2 - arg1 - 10) >> 2) & 0xFFFFFF) | 0xFB000000;
}

u32* getOffsetFromBL(u32* blOffset) {
	if (*blOffset < 0xEB000000 && *blOffset >= 0xEC000000) {
		return NULL;
	}
	u32 opCode = (*blOffset) - 0xEB000000;

	if (opCode >= 0x00800000 && opCode <= 0x00FFFFFF) {
		u32 offset = (u32)blOffset + 8;
		for (u32 i = opCode; i <= 0x00FFFFFF; i++) {
			offset -= 4;
		}

		return (u32*)offset;
	}
	return (u32*)((u32)blOffset + (opCode*4) + 8);
}

u32* getOffsetFromBLX(u32* blxOffset) {
	if (*blxOffset < 0xFB000000 && *blxOffset >= 0xFC000000) {
		return NULL;
	}
	u32 opCode = (*blxOffset) - 0xFB000000;

	if (opCode >= 0x00800000 && opCode <= 0x00FFFFFF) {
		u32 offset = (u32)blxOffset + 10;
		for (u32 i = opCode; i <= 0x00FFFFFF; i++) {
			offset -= 4;
		}

		return (u32*)offset;
	}
	return (u32*)((u32)blxOffset + (opCode*4) + 10);
}

const u16* generateA7InstrThumb(int arg1, int arg2) {
	static u16 instrs[2];

	// 23 bit offset
	u32 offset = (u32)(arg2 - arg1 - 4);
	//dbg_printf("generateA7InstrThumb offset\n");
	//dbg_hexa(offset);

	// 1st instruction contains the upper 11 bit of the offset
	instrs[0] = ((offset >> 12) & 0x7FF) | 0xF000;

	// 2nd instruction contains the lower 11 bit of the offset
	instrs[1] = ((offset >> 1) & 0x7FF) | 0xF800;

	return instrs;
}

void setBLThumb(int arg1, int arg2) {
	u16 instrs[2];

	// 23 bit offset
	u32 offset = (u32)(arg2 - arg1 - 4);
	//dbg_printf("generateA7InstrThumb offset\n");
	//dbg_hexa(offset);

	// 1st instruction contains the upper 11 bit of the offset
	instrs[0] = ((offset >> 12) & 0x7FF) | 0xF000;

	// 2nd instruction contains the lower 11 bit of the offset
	instrs[1] = ((offset >> 1) & 0x7FF) | 0xF800;

	*(u16*)arg1 = instrs[0];
	*(u16*)(arg1 + 2) = instrs[1];
}

u16* getOffsetFromBLThumb(u16* blOffset) {
	s16 codeOffset = blOffset[1];

	return (u16*)((u32)blOffset + (codeOffset*2) + 4);
}

u32 vAddrOfRelocSrc = 0;
u32 relocDestAtSharedMem = 0;

static void patchRamClear(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	if (moduleParams->sdk_version < 0x5000000 || arm7newUnitCode == 0) {
		return;
	}

	u32* ramClearOffset = patchOffsetCache.ramClearOffset;
	if (!patchOffsetCache.ramClearOffset && !patchOffsetCache.ramClearChecked) {
		ramClearOffset = findRamClearOffset(ndsHeader);
		if (ramClearOffset) {
			patchOffsetCache.ramClearOffset = ramClearOffset;
		}
	}
	if (ramClearOffset) {
		// if (arm7newUnitCode > 0) {
			*(ramClearOffset) = 0x02FFF000;
			*(ramClearOffset + 1) = 0x02FFF000;
		// }
		// ramClearOffset[3] -= 0x1800; // Shrink hi heap

		dbg_printf("RAM clear location : ");
		dbg_hexa((u32)ramClearOffset);
		dbg_printf("\n\n");
	}
	patchOffsetCache.ramClearChecked = true;
}

static void patchPostBoot(const tNDSHeader* ndsHeader) {
	if (arm7mbk != 0x080037C0) {
		return;
	}

	u32* postBootOffset = patchOffsetCache.postBootOffset;
	if (!patchOffsetCache.postBootOffset) {
		postBootOffset = findPostBootOffset(ndsHeader);
		if (postBootOffset) {
			patchOffsetCache.postBootOffset = postBootOffset;
		}
	}
	if (postBootOffset) {
		const bool usesThumb = (*(u16*)postBootOffset == 0xB5F8);
		if (usesThumb) {
			*(u16*)postBootOffset = 0x4770;	// bx lr
		} else {
			*postBootOffset = 0xE12FFF1E;	// bx lr
		}
		dbg_printf("Post boot location : ");
		dbg_hexa((u32)postBootOffset);
		dbg_printf("\n\n");
	}
}

static bool patchCardIrqEnable(cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	// Card irq enable
	u32* cardIrqEnableOffset = patchOffsetCache.a7CardIrqEnableOffset;
	if (!patchOffsetCache.a7CardIrqEnableOffset) {
		cardIrqEnableOffset = findCardIrqEnableOffset(ndsHeader, moduleParams);
		if (cardIrqEnableOffset) {
			patchOffsetCache.a7CardIrqEnableOffset = cardIrqEnableOffset;
		}
	}
	if (!cardIrqEnableOffset) {
		return false;
	}
	const bool usesThumb = (*(u16*)cardIrqEnableOffset == 0xB510 || *(u16*)cardIrqEnableOffset == 0xB530);
	if (usesThumb) {
		u16* cardIrqEnablePatch = (u16*)ce7->patches->thumb_card_irq_enable_arm7;
		tonccpy(cardIrqEnableOffset, cardIrqEnablePatch, 0x20);
	} else {
		u32* cardIrqEnablePatch = ce7->patches->card_irq_enable_arm7;
		tonccpy(cardIrqEnableOffset, cardIrqEnablePatch, 0x30);
	}

    dbg_printf("cardIrqEnable location : ");
    dbg_hexa((u32)cardIrqEnableOffset);
    dbg_printf("\n\n");
	return true;
}

/*static void patchCardCheckPullOut(cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	// Card check pull out
	u32* cardCheckPullOutOffset = findCardCheckPullOutOffset(ndsHeader, moduleParams);
	if (cardCheckPullOutOffset) {
		u32* cardCheckPullOutPatch = ce7->patches->card_pull_out_arm9;
		tonccpy(cardCheckPullOutOffset, cardCheckPullOutPatch, 0x4);
	}
}*/

static void operaRamPatch(void) {
	// Opera RAM patch (ARM7)
	*(u32*)0x0238C7BC = 0xC400000;
	*(u32*)0x0238C7C0 = 0xC4000CE;

	//*(u32*)0x0238C950 = 0xC400000;
}

extern void rsetA7Cache(void);

u32 patchCardNdsArm7(
	cardengineArm7* ce7,
	tNDSHeader* ndsHeader,
	const module_params_t* moduleParams,
	u32 saveFileCluster
) {
	arm7newUnitCode = ndsHeader->unitCode;
	newArm7binarySize = ndsHeader->arm7binarySize;

	if ((ndsHeader->unitCode > 0) ? (arm7mbk == 0x080037C0) : (memcmp(ndsHeader->gameCode, "AYI", 3) == 0 && ndsHeader->arm7binarySize == 0x25F70)) {
		// Replace incompatible ARM7 binary
		extern u32 donorFileCluster;	// SDK5
		extern u32 donorFileOffset;
		aFile donorRomFile;
		getFileFromCluster(&donorRomFile, donorFileCluster);
		if (donorRomFile.firstCluster == CLUSTER_FREE) {
			if (ndsHeader->gameCode[0] == 'D') {
				if (newArm7binarySize != patchOffsetCache.a7BinSize) {
					rsetA7Cache(); // Reset arm7 hook offsets
					patchOffsetCache.a7BinSize = newArm7binarySize;
				}
				dbg_printf("ERR_NONE\n\n");
				return ERR_NONE;
			} else {
				dbg_printf("ERR_LOAD_OTHR\n\n");
				return ERR_LOAD_OTHR;
			}
		}
		u32 arm7dst = 0;
		fileRead((char*)&arm7dst, &donorRomFile, donorFileOffset+0x38, 0x4);
		if (arm7dst == 0x02380000) {
			// Donor found within a ROM file
			u32 arm7src = 0;
			u32 arm7size = 0;
			fileRead((char*)&arm7newUnitCode, &donorRomFile, donorFileOffset+0x12, 1);
			fileRead((char*)&arm7src, &donorRomFile, donorFileOffset+0x30, 0x4);
			fileRead((char*)&arm7size, &donorRomFile, donorFileOffset+0x3C, 0x4);
			fileRead(ndsHeader->arm7destination, &donorRomFile, donorFileOffset+arm7src, arm7size);
			newArm7binarySize = arm7size;
		} else {
			// Standalone donor found
			extern u32 donorFileSize;
			fileRead(ndsHeader->arm7destination, &donorRomFile, 0, donorFileSize);
			newArm7binarySize = donorFileSize;

			u32 startOffset = (u32)ndsHeader->arm7destination;
			if (*(u32*)(startOffset + newArm7binarySize - 0xC) == 0x027E0000 || *(u32*)(startOffset + newArm7binarySize - 0x24) == 0x027E0000) {
				arm7newUnitCode = 0; // NTR ARM7 binary found
			}
		}
		if (memcmp(ndsHeader->gameCode, "KUB", 3) == 0 && arm7newUnitCode == 0) {
			dbg_printf("Donor incompatible with this ROM! Please use a DSi-Enhanced donor.\n\n");
			dbg_printf("ERR_LOAD_OTHR\n\n");
			return ERR_LOAD_OTHR;
		}
	}

	if (newArm7binarySize != patchOffsetCache.a7BinSize) {
		rsetA7Cache();
		patchOffsetCache.a7BinSize = newArm7binarySize;
	}

	if (!patchWramClear(ndsHeader)) {
		dbg_printf("ERR_LOAD_OTHR");
		return ERR_LOAD_OTHR;
	}

	patchPostBoot(ndsHeader);

	patchSleepMode(ndsHeader);
	patchSleepInputWrite(ndsHeader, moduleParams);

	patchRamClear(ndsHeader, moduleParams);

	const char* romTid = getRomTid(ndsHeader);

	if (!patchCardIrqEnable(ce7, ndsHeader, moduleParams)) {
		dbg_printf("ERR_LOAD_OTHR");
		return ERR_LOAD_OTHR;
	}

	//patchCardCheckPullOut(ce7, ndsHeader, moduleParams);

	const u32 cardId = getChipId(ndsHeader, moduleParams);
	u32* cardIdPatch = (u32*)ce7->patches->arm7Functions->cardId;
	u32* cardIdPatchThumb = (u32*)ce7->patches->arm7FunctionsThumb->cardId;
	cardIdPatch[2] = cardId;
	cardIdPatchThumb[1] = cardId;

	if (a7GetReloc(ndsHeader, moduleParams)) {
		u32 saveResult = 0;

		if (newArm7binarySize==0x2352C || newArm7binarySize==0x235DC || newArm7binarySize==0x23CAC || newArm7binarySize==0x245C0 || newArm7binarySize==0x245C4) {
			saveResult = savePatchInvertedThumb(ce7, ndsHeader, moduleParams, saveFileCluster);    
		} else if (isSdk5(moduleParams)) {
			// SDK 5
			saveResult = savePatchV5(ce7, ndsHeader, saveFileCluster);
		} else {
			if (patchOffsetCache.savePatchType == 0) {
				saveResult = savePatchV1(ce7, ndsHeader, moduleParams, saveFileCluster);
				if (!saveResult) {
					patchOffsetCache.savePatchType = 1;
				}
			}
			if (!saveResult && patchOffsetCache.savePatchType == 1) {
				saveResult = savePatchV2(ce7, ndsHeader, moduleParams, saveFileCluster);
				if (!saveResult) {
					patchOffsetCache.savePatchType = 2;
				}
			}
			if (!saveResult && patchOffsetCache.savePatchType == 2) {
				saveResult = savePatchUniversal(ce7, ndsHeader, moduleParams, saveFileCluster);
			}
		}
		if (!saveResult) {
			patchOffsetCache.savePatchType = 0;
		}
	}

	if (strcmp(romTid, "UBRP") == 0) {
		operaRamPatch();
	}

	fixForDSiBios(ce7, ndsHeader, moduleParams);

	dbg_printf("ERR_NONE\n\n");
	return ERR_NONE;
}
