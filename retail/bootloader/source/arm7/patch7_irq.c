#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "nds_header.h"
#include "patch.h"
#include "tonccpy.h"

extern u32 newArm7binarySize;
extern u32 iUncompressedSize;

// irq enable
// ARM7
static const u32 a7IrqEnableStartSignature1[4]      = {0xE59FC028, 0xE1DC30B0, 0xE3A01000, 0xE1CC10B0}; // SDK <= 3
static const u32 a7IrqEnableStartSignature4[4]      = {0xE92D4010, 0xE1A04000, 0xEBFFFFF6, 0xE59FC020}; // SDK >= 4
static const u32 a7IrqEnableStartSignature4Alt[4]   = {0xE92D4010, 0xE1A04000, 0xEBFFFFE9, 0xE59FC020}; // SDK 5
static const u16 a7IrqEnableStartSignatureThumb[5]  = {0xB530, 0xB081, 0x4D07, 0x882C, 0x2100};
static const u16 a7IrqEnableStartSignatureThumb3[5] = {0xB510, 0x1C04, 0xF7FF, 0xFFF4, 0x4B05}; // SDK 3
static const u16 a7IrqEnableStartSignatureThumb5[5] = {0xB510, 0x1C04, 0xF7FF, 0xFFE4, 0x4B05}; // SDK 5

static u32* findCardIrqEnableOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	dbg_printf("findCardIrqEnableOffset:\n");
	
	const u32* a7IrqEnableStartSignature = a7IrqEnableStartSignature1;
	if (ndsHeader->arm7binarySize != 0x289C0 && moduleParams->sdk_version > 0x4000000) {
		a7IrqEnableStartSignature = a7IrqEnableStartSignature4;
	}

	u32* cardIrqEnableOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		a7IrqEnableStartSignature, 4
	);
	if (cardIrqEnableOffset) {
		dbg_printf("irq enable found\n");
	} else {
		dbg_printf("irq enable not found\n");
	}

	if (!cardIrqEnableOffset && moduleParams->sdk_version < 0x4000000) {
		// SDK 4
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize,
            a7IrqEnableStartSignature4, 4
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable SDK 4 found\n");
		} else {
			dbg_printf("irq enable SDK 4 not found\n");
		}
	}

	if (!cardIrqEnableOffset) {
		// SDK 5
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize,
            a7IrqEnableStartSignature4Alt, 4
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable alt found\n");
		} else {
			dbg_printf("irq enable alt not found\n");
		}
	}

	if (!cardIrqEnableOffset) {
		cardIrqEnableOffset = (u32*)findOffsetThumb(
			(u16*)ndsHeader->arm7destination, newArm7binarySize,
            a7IrqEnableStartSignatureThumb, 5
		);
		if (cardIrqEnableOffset) {
			// Find again
			cardIrqEnableOffset = (u32*)findOffsetThumb(
				(u16*)cardIrqEnableOffset+4, newArm7binarySize,
				a7IrqEnableStartSignatureThumb, 5
			);
		}
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable thumb found\n");
		} else {
			dbg_printf("irq enable thumb not found\n");
		}
	}

	if (!cardIrqEnableOffset) {
		// SDK 3
		cardIrqEnableOffset = (u32*)findOffsetThumb(
			(u16*)ndsHeader->arm7destination, newArm7binarySize,
            a7IrqEnableStartSignatureThumb3, 5
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable thumb SDK 3 found\n");
		} else {
			dbg_printf("irq enable thumb SDK 3 not found\n");
		}
	}

	if (!cardIrqEnableOffset && isSdk5(moduleParams)) {
		// SDK 5
		cardIrqEnableOffset = (u32*)findOffsetThumb(
			(u16*)ndsHeader->arm7destination, newArm7binarySize,
            a7IrqEnableStartSignatureThumb5, 5
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable thumb SDK 5 found\n");
		} else {
			dbg_printf("irq enable thumb SDK 5 not found\n");
		}
	}

	dbg_printf("\n");
	return cardIrqEnableOffset;
}

bool a7PatchCardIrqEnable(cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
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
