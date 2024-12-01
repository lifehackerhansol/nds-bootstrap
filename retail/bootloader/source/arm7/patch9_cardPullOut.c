#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u32 iUncompressedSize;

// Card pull out
static const u32 cardPullOutSignature1[4]         = {0xE92D4000, 0xE24DD004, 0xE201003F, 0xE3500011}; // SDK <= 3
static const u32 cardPullOutSignature1Elab[5]     = {0xE92D4000, 0xE24DD004, 0xE201003F, 0xE3500011, 0x1A00000F}; // SDK 2
static const u32 cardPullOutSignature2Alt[4]      = {0xE92D000F, 0xE92D4030, 0xE24DD004, 0xE59D0014}; // SDK 2
static const u32 cardPullOutSignature4[4]         = {0xE92D4008, 0xE201003F, 0xE3500011, 0x1A00000D}; // SDK >= 4
static const u32 cardPullOutSignatureDebug[5]     = {0xE92D000F, 0xE92D4038, 0xE59D0014, 0xE200503F, 0xE3550011}; // SDK 4 (DEBUG)
static const u32 cardPullOutSignature5[4]         = {0xE92D4010, 0xE201003F, 0xE3500011, 0x1A000012}; // SDK 5
static const u32 cardPullOutSignature5Alt[4]      = {0xE92D4038, 0xE201003F, 0xE3500011, 0x1A000011}; // SDK 5
static const u32 cardPullOutSignatureDebug5[5]    = {0xE92D000F, 0xE92D4038, 0xE59D0014, 0xE200403F, 0xE3540011}; // SDK 5 (DEBUG)
static const u16 cardPullOutSignatureThumb[5]     = {0xB508, 0x203F, 0x4008, 0x2811, 0xD10E};
static const u16 cardPullOutSignatureThumbAlt[4]  = {0xB500, 0xB081, 0x203F, 0x4001};
static const u16 cardPullOutSignatureThumbAlt2[4] = {0xB5F8, 0x203F, 0x4008, 0x2811};
static const u16 cardPullOutSignatureThumb5[4]    = {0xB510, 0x203F, 0x4008, 0x2811};                 // SDK 5
static const u16 cardPullOutSignatureThumb5Alt[4] = {0xB538, 0x203F, 0x4008, 0x2811};                 // SDK 5

static u32* findCardPullOutOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	dbg_printf("findCardPullOutOffset:\n");

	//if (!usesThumb) {
	
	u32* cardPullOutOffset = 0;
	if (moduleParams->sdk_version > 0x5000000) { // SDK 5
		cardPullOutOffset = findOffset(
			(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			cardPullOutSignature5, 4
		);
		if (cardPullOutOffset) {
			dbg_printf("Card pull out handler SDK 5 found\n");
		} else {
			dbg_printf("Card pull out handler SDK 5 not found\n");
		}

		if (!cardPullOutOffset) {
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature5Alt, 4
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler SDK 5 alt found\n");
			} else {
				dbg_printf("Card pull out handler SDK 5 alt not found\n");
			}
		}

		if (!cardPullOutOffset) {
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignatureDebug5, 5
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler SDK 5 DEBUG found\n");
			} else {
				dbg_printf("Card pull out handler SDK 5 DEBUG not found\n");
			}
		}
	} else {
		if (moduleParams->sdk_version > 0x2008000 && moduleParams->sdk_version < 0x3000000) {
			// SDK 2
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature1Elab, 5
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler SDK 2 elaborate found\n");
			} else {
				dbg_printf("Card pull out handler SDK 2 elaborate not found\n");
			}
		}

		if (!cardPullOutOffset && moduleParams->sdk_version < 0x4000000) {
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature1, 4
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler found\n");
			} else {
				dbg_printf("Card pull out handler not found\n");
			}
		}

		if (!cardPullOutOffset && moduleParams->sdk_version < 0x2008000) {
			// SDK 2
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature2Alt, 4
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler SDK 2 alt found\n");
			} else {
				dbg_printf("Card pull out handler SDK 2 alt not found\n");
			}
		}

		if (!cardPullOutOffset) {
			// SDK 4
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature4, 4
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler SDK 4 found\n");
			} else {
				dbg_printf("Card pull out handler SDK 4 not found\n");
			}
		}

		if (!cardPullOutOffset && moduleParams->sdk_version > 0x4000000) {
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignature1, 4
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler found\n");
			} else {
				dbg_printf("Card pull out handler not found\n");
			}
		}

		if (!cardPullOutOffset) {
			cardPullOutOffset = findOffset(
				(u32*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
				cardPullOutSignatureDebug, 5
			);
			if (cardPullOutOffset) {
				dbg_printf("Card pull out handler DEBUG found\n");
			} else {
				dbg_printf("Card pull out handler DEBUG not found\n");
			}
		}
	}

	dbg_printf("\n");
	return cardPullOutOffset;
}

static u16* findCardPullOutOffsetThumb(const tNDSHeader* ndsHeader) {
	dbg_printf("findCardPullOutOffsetThumb:\n");

	//if (usesThumb) {
	
	u16* cardPullOutOffset = findOffsetThumb(
		(u16*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
		cardPullOutSignatureThumb, 5
	);
	if (cardPullOutOffset) {
		dbg_printf("Card pull out handler thumb found\n");
	} else {
		dbg_printf("Card pull out handler thumb not found\n");
	}

	if (!cardPullOutOffset) {
		cardPullOutOffset = findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			cardPullOutSignatureThumbAlt, 4
		);
		if (cardPullOutOffset) {
			dbg_printf("Card pull out handler thumb alt found\n");
		} else {
			dbg_printf("Card pull out handler thumb alt not found\n");
		}
	}

	if (!cardPullOutOffset) {
		cardPullOutOffset = findOffsetThumb(
			(u16*)ndsHeader->arm9destination, iUncompressedSize,//ndsHeader->arm9binarySize,
			cardPullOutSignatureThumbAlt2, 4
		);
		if (cardPullOutOffset) {
			dbg_printf("Card pull out handler thumb alt 2 found\n");
		} else {
			dbg_printf("Card pull out handler thumb alt 2 not found\n");
		}
	}

	dbg_printf("\n");
	return cardPullOutOffset;
}

// SDK 5
static u16* findCardPullOutOffsetThumb5Type0(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	if (moduleParams->sdk_version < 0x5000000) {
		return NULL;
	}

	dbg_printf("findCardPullOutOffsetThumbType0:\n");
	
	u16* cardPullOutOffset = findOffsetThumb(
		(u16*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
		cardPullOutSignatureThumb5, 4
	);
	if (cardPullOutOffset) {
		dbg_printf("Card pull out handler SDK 5 thumb (type 0) found\n");
	} else {
		dbg_printf("Card pull out handler SDK 5 thumb (type 0) not found\n");
	}

	dbg_printf("\n");
	return cardPullOutOffset;
}

// SDK 5
static u16* findCardPullOutOffsetThumb5Type1(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	if (moduleParams->sdk_version < 0x5000000) {
		return NULL;
	}
	
	dbg_printf("findCardPullOutOffsetThumbType1:\n");
	
	u16* cardPullOutOffset = findOffsetThumb(
		(u16*)ndsHeader->arm9destination, iUncompressedSize,//, ndsHeader->arm9binarySize,
		cardPullOutSignatureThumb5Alt, 4
	);
	if (cardPullOutOffset) {
		dbg_printf("Card pull out handler SDK 5 thumb alt (type 1) found\n");
	} else {
		dbg_printf("Card pull out handler SDK 5 thumb alt (type 1) not found\n");
	}

	dbg_printf("\n");
	return cardPullOutOffset;
}

void patchCardPullOut(cardengineArm9* ce9, const tNDSHeader* ndsHeader, const module_params_t* moduleParams, bool usesThumb, int sdk5ReadType, u32** cardPullOutOffsetPtr) {
	// Card pull out
	u32* cardPullOutOffset = patchOffsetCache.cardPullOutOffset;
	if (!patchOffsetCache.cardPullOutOffset) {
		cardPullOutOffset = NULL;
		if (usesThumb) {
			//dbg_printf("Trying SDK 5 thumb...\n");
			if (sdk5ReadType == 0) {
				cardPullOutOffset = (u32*)findCardPullOutOffsetThumb5Type0(ndsHeader, moduleParams);
			} else {
				cardPullOutOffset = (u32*)findCardPullOutOffsetThumb5Type1(ndsHeader, moduleParams);
			}
			if (!cardPullOutOffset) {
				//dbg_printf("Trying thumb...\n");
				cardPullOutOffset = (u32*)findCardPullOutOffsetThumb(ndsHeader);
			}
		} else {
			cardPullOutOffset = findCardPullOutOffset(ndsHeader, moduleParams);
		}
		if (cardPullOutOffset) {
			patchOffsetCache.cardPullOutOffset = cardPullOutOffset;
		}
	}
	*cardPullOutOffsetPtr = cardPullOutOffset;
	if (!cardPullOutOffset) {
		return;
	}

	// Patch
	*cardPullOutOffset = usesThumb ? 0x47704770 : 0xE12FFF1E; // bx lr
    dbg_printf("cardPullOut location : ");
    dbg_hexa((u32)cardPullOutOffset);
    dbg_printf("\n\n");
}
