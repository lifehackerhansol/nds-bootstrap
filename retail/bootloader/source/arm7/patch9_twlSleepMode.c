#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u32 iUncompressedSize;

// Sleep mode trigger (TWL)
static const u32 twlSleepModeEndSignatureEarly[4]      = {0xE2855001, 0xE3550003, 0xE286600C, 0x9AFFFFE2}; // SDK 5.1 & 5.2
static const u32 twlSleepModeEndSignature[3]           = {0xE2866001, 0xE3560003, 0x9AFFFFDF}; // SDK 5.3+
static const u16 twlSleepModeEndSignatureThumbEarly[7] = {0x9800, 0x1C64, 0x1D00, 0x350C, 0x9000, 0x2C03, 0xD9CE}; // SDK 5.1 & 5.2
static const u16 twlSleepModeEndSignatureThumb[3]      = {0x1C6D, 0x2D03, 0xD9D1}; // SDK 5.3+

static u32* findTwlSleepModeEndOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findTwlSleepModeEndOffset\n");
	
	u32* offset = findOffset(
		(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
		twlSleepModeEndSignatureEarly, 4
	);

	if (!offset) {
		offset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			twlSleepModeEndSignature, 3
		);
	}

	if (!offset) {
		offset = (u32*)findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			twlSleepModeEndSignatureThumbEarly, 7
		);
	}

	if (!offset) {
		offset = (u32*)findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			twlSleepModeEndSignatureThumb, 3
		);
	}

	if (offset) {
		dbg_printf("twlSleepModeEnd found\n");
	} else {
		dbg_printf("twlSleepModeEnd not found\n");
	}

	dbg_printf("\n");
	return offset;
}

void patchTwlSleepMode(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	extern u32 arm7mbk;
	if (arm7mbk != 0x080037C0 || moduleParams->sdk_version < 0x5010000) {
		return;
	}

	u32* offset = patchOffsetCache.twlSleepModeEndOffset;
	if (!patchOffsetCache.twlSleepModeEndChecked) {
		offset = findTwlSleepModeEndOffset(ndsHeader);
		if (offset) {
			patchOffsetCache.twlSleepModeEndOffset = offset;
		}
		patchOffsetCache.twlSleepModeEndChecked = true;
	}

	if (!offset) {
		return;
	}

	dbg_printf("twlSleepModeEnd location : ");
	dbg_hexa((u32)offset);
	dbg_printf("\n\n");

	if (*offset == 0xE2855001 || *offset == 0xE2866001) {
		offset--;
		*offset = 0xE1A00000; // nop
	} else {
		u16* offsetThumb = (u16*)offset;
		offsetThumb--;
		offsetThumb--;
		offsetThumb[0] = 0x46C0; // nop
		offsetThumb[1] = 0x46C0; // nop
	}
}
