#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "nds_header.h"
#include "patch.h"
#include "tonccpy.h"

extern u32 iUncompressedSize;
bool patchedCardIrqEnable = false;

// irq enable
static const u32 irqEnableStartSignature1[4]        = {0xE59FC028, 0xE3A01000, 0xE1DC30B0, 0xE59F2020};					// SDK <= 3
static const u32 irqEnableStartSignature2Alt[4]     = {0xE92D000F, 0xE92D4030, 0xE24DD004, 0xEBFFFFDB};					// SDK 2
static const u32 irqEnableStartSignature4[4]        = {0xE59F3024, 0xE3A01000, 0xE1D320B0, 0xE1C310B0};					// SDK >= 4
static const u32 irqEnableStartSignature4Debug[4]   = {0xE92D000F, 0xE92D4038, 0xEBFFFFE5, 0xE1A05000};					// SDK >= 4 (DEBUG)
static const u32 irqEnableStartSignatureThumb[5]    = {0x4D07B430, 0x2100882C, 0x4B068029, 0x1C11681A, 0x60194301};		// SDK <= 3
static const u32 irqEnableStartSignatureThumbAlt[4] = {0x4C07B418, 0x88232100, 0x32081C22, 0x68118021};					// SDK >= 3

static u32* a9FindCardIrqEnableOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams, bool* usesThumb) {
	dbg_printf("findCardIrqEnableOffset:\n");
	
	const u32* irqEnableStartSignature = irqEnableStartSignature1;
	if (moduleParams->sdk_version > 0x4008000) {
		irqEnableStartSignature = irqEnableStartSignature4;
	}

	u32* cardIrqEnableOffset = findOffset(
		(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
		irqEnableStartSignature, 4
	);
	if (cardIrqEnableOffset) {
		dbg_printf("irq enable found: ");
	} else {
		dbg_printf("irq enable not found\n");
	}

	if (!cardIrqEnableOffset && moduleParams->sdk_version < 0x2008000) {
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
            irqEnableStartSignature2Alt, 4
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable SDK 2 alt found: ");
		} else {
			dbg_printf("irq enable SDK 2 alt not found\n");
		}
	}

	if (!cardIrqEnableOffset && moduleParams->sdk_version > 0x3000000 && moduleParams->sdk_version < 0x4008000) {
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
            irqEnableStartSignature4, 4
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable SDK 4 found: ");
		} else {
			dbg_printf("irq enable SDK 4 not found\n");
		}
	}

	if (!cardIrqEnableOffset && moduleParams->sdk_version > 0x4000000) {
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
            irqEnableStartSignature4Debug, 4
		);
		if (cardIrqEnableOffset) {
			dbg_printf("irq enable SDK 4 debugger found: ");
		} else {
			dbg_printf("irq enable SDK 4 debugger not found\n");
		}
	}

	if (!cardIrqEnableOffset && moduleParams->sdk_version < 0x4008000) {
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
            irqEnableStartSignatureThumb, 5
		);
		if (cardIrqEnableOffset) {
			*usesThumb = true;
			dbg_printf("irq enable thumb found: ");
		} else {
			dbg_printf("irq enable thumb not found\n");
		}
	}

	if (!cardIrqEnableOffset) {
		cardIrqEnableOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
            irqEnableStartSignatureThumbAlt, 4
		);
		if (cardIrqEnableOffset) {
			*usesThumb = true;
			dbg_printf("irq enable thumb alt found: ");
		} else {
			dbg_printf("irq enable thumb alt not found\n");
		}
	}

	if (cardIrqEnableOffset) {
		dbg_hexa((u32)cardIrqEnableOffset);
		dbg_printf("\n");
	}

	dbg_printf("\n");
	return cardIrqEnableOffset;
}

bool a9PatchCardIrqEnable(cardengineArm9* ce9, const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	const char* romTid = getRomTid(ndsHeader);

	if (strncmp(romTid, "AWD", 3) == 0 // Diddy Kong Racing - Fix corrupted 3D model bug
	 || strncmp(romTid, "CP3", 3) == 0 // Viva Pinata - Fix touch and model rendering bug
	 || strncmp(romTid, "BO5", 3) == 0 // Golden Sun: Dark Dawn - Fix black screen on boot
	 || strncmp(romTid, "Y8L", 3) == 0 // Golden Sun: Dark Dawn (Demo Version) - Fix black screen on boot
	 || strncmp(romTid, "B8I", 3) == 0 // Spider-Man: Edge of Time - Fix white screen on boot
	 || strncmp(romTid, "TAM", 3) == 0 // The Amazing Spider-Man - Fix white screen on boot
	) {
		return true;
	}

	bool usesThumb = patchOffsetCache.a9CardIrqIsThumb;

	// Card irq enable
	u32* cardIrqEnableOffset = patchOffsetCache.a9CardIrqEnableOffset;
	if (!patchOffsetCache.a9CardIrqEnableOffset) {
		cardIrqEnableOffset = a9FindCardIrqEnableOffset(ndsHeader, moduleParams, &usesThumb);
		if (cardIrqEnableOffset) {
			patchOffsetCache.a9CardIrqEnableOffset = cardIrqEnableOffset;
			patchOffsetCache.a9CardIrqIsThumb = usesThumb;
		}
	}
	if (!cardIrqEnableOffset) {
		return false;
	}
	const u32 newCardIrqEnable = (u32)ce9->patches->card_irq_enable;
	if (usesThumb) {
		u16* offsetThumb = (u16*)cardIrqEnableOffset;
		offsetThumb[0] = 0xB540; // push {r6, lr}
		offsetThumb[1] = 0x4E01; // ldr r6, =newCardIrqEnable
		offsetThumb[2] = 0x47B0; // blx r6
		offsetThumb[3] = 0xBD40; // pop {r6, pc}
		cardIrqEnableOffset[2] = newCardIrqEnable;
	} else {
		cardIrqEnableOffset[0] = 0xE51FF004; // ldr pc, =newCardIrqEnable
		cardIrqEnableOffset[1] = newCardIrqEnable;
	}
    dbg_printf("cardIrqEnable location : ");
    dbg_hexa((u32)cardIrqEnableOffset);
    dbg_printf("\n\n");
	patchedCardIrqEnable = true;
	return true;
}
