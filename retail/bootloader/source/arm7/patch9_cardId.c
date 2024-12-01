#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u32 iUncompressedSize;
extern bool isPawsAndClaws(const tNDSHeader* ndsHeader);

// Card id
static const u32 cardIdEndSignature[2]            = {0x040001A4, 0x04100010};
static const u32 cardIdEndSignature5[4]           = {0xE8BD8010, 0x02FFFAE0, 0x040001A4, 0x04100010}; // SDK 5
static const u32 cardIdEndSignature5Alt[3]        = {0x02FFFAE0, 0x040001A4, 0x04100010};             // SDK 5
static const u32 cardIdEndSignatureDebug5[4]      = {0x0AFFFFFA, 0xE59F0008, 0xE5900000, 0xE8BD8010};             // SDK 5
static const u16 cardIdEndSignatureThumb[6]       = {0xFFFF, 0xF8FF, 0x01A4, 0x0400, 0x0010, 0x0410};
static const u16 cardIdEndSignatureThumbAlt[6]    = {0xFFFF, 0xF8FF, 0x0000, 0xA700, 0xE000, 0xFFFF};
static const u16 cardIdEndSignatureThumb5[8]      = {0xFAE0, 0x02FF, 0xFFFF, 0xF8FF, 0x01A4, 0x0400, 0x0010, 0x0410}; // SDK 5
static const u32 cardIdStartSignature[1]          = {0xE92D4000};
static const u32 cardIdStartSignatureAlt1[1]      = {0xE92D4008};
static const u32 cardIdStartSignatureAlt2[1]      = {0xE92D4010};
static const u32 cardIdStartSignature5[2]         = {0xE92D4010, 0xE3A050B8}; // SDK 5
static const u32 cardIdStartSignature5Alt[2]      = {0xE92D4038, 0xE3A050B8}; // SDK 5
static const u16 cardIdStartSignatureThumb[2]     = {0xB500, 0xB081};
static const u16 cardIdStartSignatureThumbAlt1[2] = {0xB508, 0x202E};
static const u16 cardIdStartSignatureThumbAlt2[2] = {0xB508, 0x20B8};
static const u16 cardIdStartSignatureThumbAlt3[2] = {0xB510, 0x24B8};

static u32* findCardIdEndOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams, const u32* cardReadEndOffset) {
	if (!isPawsAndClaws(ndsHeader) && !cardReadEndOffset) {
		return NULL;
	}

	dbg_printf("findCardIdEndOffset:\n");

	u32* cardIdEndOffset = NULL;

	if (isSdk5(moduleParams)) {
		if (cardReadEndOffset) {
			cardIdEndOffset = findOffsetBackwards(
				(u32*)cardReadEndOffset, 0x800,
				cardIdEndSignature5, 4
			);
		} else {
			cardIdEndOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,
				cardIdEndSignature5, 4
			);
		}
		if (cardIdEndOffset) {
			dbg_printf("Card ID end SDK 5 found: ");
		} else {
			dbg_printf("Card ID end SDK 5 not found\n");
		}

		if (!cardIdEndOffset) {
			cardIdEndOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,
				cardIdEndSignature5Alt, 3
			);
			if (cardIdEndOffset) {
				dbg_printf("Card ID end SDK 5 alt found: ");
			} else {
				dbg_printf("Card ID end SDK 5 alt not found\n");
			}
		}

		if (!cardIdEndOffset) {
			if (cardReadEndOffset) {
				cardIdEndOffset = findOffsetBackwards(
					(u32*)cardReadEndOffset, 0x800,
					cardIdEndSignatureDebug5, 4
				);
			} else {
				cardIdEndOffset = findOffset(
					(u32*)ndsHeader->arm9destination, iUncompressedSize,
					cardIdEndSignatureDebug5, 4
				);
			}
			if (cardIdEndOffset) {
				dbg_printf("Card ID end SDK 5 DEBUG found: ");
			} else {
				dbg_printf("Card ID end SDK 5 DEBUG not found\n");
			}
		}
	} else {
		if (cardReadEndOffset) {
			cardIdEndOffset = findOffset(
				(u32*)cardReadEndOffset + 0x10, iUncompressedSize,
				cardIdEndSignature, 2
			);
		}
		if (!cardIdEndOffset) {
			cardIdEndOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,
				cardIdEndSignature, 2
			);
		}
		if (cardIdEndOffset) {
			cardIdEndOffset[0] = 0; // Prevent being searched again
			dbg_printf("Card ID end found: ");
		} else {
			dbg_printf("Card ID end not found\n");
		}
	}

	if (cardIdEndOffset) {
		dbg_hexa((u32)cardIdEndOffset);
		dbg_printf("\n");
	}

	dbg_printf("\n");
	return cardIdEndOffset;
}

static u16* findCardIdEndOffsetThumb(const tNDSHeader* ndsHeader, const module_params_t* moduleParams, const u16* cardReadEndOffset) {
	if (!cardReadEndOffset) {
		return NULL;
	}

	dbg_printf("findCardIdEndOffsetThumb:\n");

	//if (usesThumb) {
	
	u16* cardIdEndOffset = findOffsetThumb(
		(u16*)ndsHeader->arm9destination, iUncompressedSize,
		cardIdEndSignatureThumb, 6
	);
	if (cardIdEndOffset) {
		dbg_printf("Card ID end thumb found: ");
	} else {
		dbg_printf("Card ID end thumb not found\n");
	}

	if (!cardIdEndOffset && moduleParams->sdk_version < 0x5000000) {
		// SDK <= 4
		cardIdEndOffset = findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,
			cardIdEndSignatureThumbAlt, 6
		);
		if (cardIdEndOffset) {
			dbg_printf("Card ID end thumb alt found: ");
		} else {
			dbg_printf("Card ID end thumb alt not found\n");
		}
	}

	if (!cardIdEndOffset && isSdk5(moduleParams)) {
		// SDK 5
		cardIdEndOffset = findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,
			cardIdEndSignatureThumb5, 8
		);
		if (cardIdEndOffset) {
			dbg_printf("Card ID end SDK 5 thumb found: ");
		} else {
			dbg_printf("Card ID end SDK 5 thumb not found\n");
		}
	}

	if (cardIdEndOffset) {
		dbg_hexa((u32)cardIdEndOffset);
		dbg_printf("\n");
	}

	dbg_printf("\n");
	return cardIdEndOffset;
}

static u32* findCardIdStartOffset(const module_params_t* moduleParams, const u32* cardIdEndOffset) {
	if (!cardIdEndOffset) {
		return NULL;
	}

	dbg_printf("findCardIdStartOffset:\n");

	u32* cardIdStartOffset = NULL;

	if (isSdk5(moduleParams)) {
		// SDK 5
		cardIdStartOffset = findOffsetBackwards2(
			(u32*)cardIdEndOffset, 0x200,
			cardIdStartSignature5, cardIdStartSignature5Alt, 2
		);
		if (cardIdStartOffset) {
			dbg_printf("Card ID start SDK 5 found\n");
		} else {
			dbg_printf("Card ID start SDK 5 not found\n");
		}

		if (!cardIdStartOffset) {
			cardIdStartOffset = findOffsetBackwards2(
				(u32*)cardIdEndOffset, 0x100,
				cardIdStartSignatureAlt1, cardIdStartSignatureAlt2, 1
			);
			if (cardIdStartOffset) {
				dbg_printf("Card ID start alt found\n");
			} else {
				dbg_printf("Card ID start alt not found\n");
			}
		}
	} else {
		cardIdStartOffset = findOffsetBackwards3(
			(u32*)cardIdEndOffset, 0x100,
			cardIdStartSignature, cardIdStartSignatureAlt1, cardIdStartSignatureAlt2, 1
		);
		if (cardIdStartOffset) {
			dbg_printf("Card ID start found\n");
		} else {
			dbg_printf("Card ID start not found\n");
		}
	}

	dbg_printf("\n");
	return cardIdStartOffset;
}

static u16* findCardIdStartOffsetThumb(const module_params_t* moduleParams, const u16* cardIdEndOffset) {
	if (!cardIdEndOffset) {
		return NULL;
	}

	dbg_printf("findCardIdStartOffsetThumb:\n");

	//if (usesThumb) {
	
	u16* cardIdStartOffset = findOffsetBackwardsThumb4(
		(u16*)cardIdEndOffset, 0x50,
		cardIdStartSignatureThumb, cardIdStartSignatureThumbAlt1, cardIdStartSignatureThumbAlt2, cardIdStartSignatureThumbAlt3, 2
	);
	if (cardIdStartOffset) {
		dbg_printf("Card ID start thumb found\n");
	} else {
		dbg_printf("Card ID start thumb not found\n");
	}

	dbg_printf("\n");
	return cardIdStartOffset;
}

bool patchCardId(cardengineArm9* ce9, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, bool usesThumb, u32* cardReadEndOffset) {
	if (!isPawsAndClaws(ndsHeader) && !cardReadEndOffset) {
		return true;
	}

	// Card ID
	u32* cardIdStartOffset = patchOffsetCache.cardIdOffset;
	if (!patchOffsetCache.cardIdChecked) {
		cardIdStartOffset = NULL;
		u32* cardIdEndOffset = NULL;
		if (usesThumb) {
			cardIdEndOffset = (u32*)findCardIdEndOffsetThumb(ndsHeader, moduleParams, (u16*)cardReadEndOffset);
			cardIdStartOffset = (u32*)findCardIdStartOffsetThumb(moduleParams, (u16*)cardIdEndOffset);
		} else {
			cardIdEndOffset = findCardIdEndOffset(ndsHeader, moduleParams, cardReadEndOffset);
			cardIdStartOffset = findCardIdStartOffset(moduleParams, cardIdEndOffset);
		}
		if (cardIdStartOffset) {
			patchOffsetCache.cardIdOffset = cardIdStartOffset;
		}
		patchOffsetCache.cardIdChecked = true;
	}

	if (cardIdStartOffset) {
        // Patch
		extern u32 baseChipID;
		if (usesThumb) {
			cardIdStartOffset[0] = 0x47704800; // ldr r0, baseChipID + bx lr
			cardIdStartOffset[1] = baseChipID;
		} else {
			cardIdStartOffset[0] = 0xE59F0000; // ldr r0, baseChipID
			cardIdStartOffset[1] = 0xE12FFF1E; // bx lr
			cardIdStartOffset[2] = baseChipID;
		}
		dbg_printf("cardId location : ");
		dbg_hexa((u32)cardIdStartOffset);
		dbg_printf("\n\n");
	} else if (isSdk5(moduleParams)) {
		return false;
	}

	return true;
}
