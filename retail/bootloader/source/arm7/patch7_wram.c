#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u8 arm7newUnitCode;
extern u32 newArm7binarySize;

// WRAM clear
static const u32 wramEndAddr[1]                = {0x0380FF00};
static const u32 wramClearSignature1[2]        = {0xE92D4010, 0xE3A00008};
static const u32 wramClearSignature3[3]        = {0xE92D4010, 0xE24DD008, 0xE3A00008};
static const u32 wramClearSignature3Alt[1]     = {0xE1C507BA};
static const u32 wramClearSignature4[1]        = {0xE1C407BA};
static const u32 wramClearSignature5[2]        = {0xE92D4038, 0xE3A05008};
static const u32 wramClearSignatureTwlEarly[3] = {0xE92D4070, 0xE1A06000, 0xE3560001};
static const u32 wramClearSignatureTwl[3]      = {0xE92D40F8, 0xE1A07000, 0xE3570001};
static const u32 wramClearSignatureTwlAlt1[3]  = {0xE92D43FE, 0xE1A04000, 0xE59F61C8};
static const u32 wramClearSignatureTwlAlt2[4]  = {0xE92D40F8, 0xE24DD008, 0xE1A04000, 0xE3540001};
static const u16 wramClearSignature1Thumb[2]   = {0xB510, 0x2008};
static const u16 wramClearSignature4Thumb[1]   = {0x80E0};
static const u16 wramClearSignatureTwlThumb[3] = {0xB570, 0x1C05, 0x2D01};

static u32* findWramEndAddrOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findWramEndAddrOffset:\n");

	u32* offset = findOffset(
		(u32*)ndsHeader->arm7destination, 0x200,
		wramEndAddr, 1
	);
	if (offset) {
		dbg_printf("WRAM end addr offset found\n");
	} else {
		dbg_printf("WRAM end addr offset not found\n");
	}

	dbg_printf("\n");
	return offset;
}

static u32* findWramClearOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findWramClearOffset:\n");

	u32* offset = NULL;
	if (arm7newUnitCode > 0) {
		offset = findOffset(
			(u32*)ndsHeader->arm7destination, 0x1000,
			wramClearSignatureTwlEarly, 3
		);
		if (offset) {
			dbg_printf("WRAM clear offset (early SDK5) found\n");
		} else {
			dbg_printf("WRAM clear offset (early SDK5) not found\n");
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x1000,
				wramClearSignatureTwl, 3
			);
			if (offset) {
				dbg_printf("WRAM clear offset found\n");
			} else {
				dbg_printf("WRAM clear offset not found\n");
			}
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x1000,
				wramClearSignatureTwlAlt1, 3
			);
			if (offset) {
				dbg_printf("WRAM clear alt 1 offset found\n");
			} else {
				dbg_printf("WRAM clear alt 1 offset not found\n");
			}
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x1000,
				wramClearSignatureTwlAlt2, 4
			);
			if (offset) {
				dbg_printf("WRAM clear alt 2 offset found\n");
			} else {
				dbg_printf("WRAM clear alt 2 offset not found\n");
			}
		}

		if (!offset) {
			offset = (u32*)findOffsetThumb(
				(u16*)ndsHeader->arm7destination, 0x1000,
				wramClearSignatureTwlThumb, 3
			);
			if (offset) {
				dbg_printf("WRAM clear offset thumb found\n");
			} else {
				dbg_printf("WRAM clear offset thumb not found\n");
			}
		}
	} else {
		offset = findOffset(
			(u32*)ndsHeader->arm7destination, 0x1000,
			wramClearSignature1, 2
		);
		if (offset) {
			dbg_printf("WRAM clear offset (SDK2) found\n");
		} else {
			dbg_printf("WRAM clear offset (SDK2) not found\n");
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x1000,
				wramClearSignature3, 3
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK3) found\n");
			} else {
				dbg_printf("WRAM clear offset (SDK3) not found\n");
			}
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x1000,
				wramClearSignature5, 2
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK5) found\n");
			} else {
				dbg_printf("WRAM clear offset (SDK5) not found\n");
			}
		}

		if (!offset) {
			offset = (u32*)findOffsetThumb(
				(u16*)ndsHeader->arm7destination, 0x1000,
				wramClearSignature1Thumb, 2
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK2) thumb found\n");
			} else {
				dbg_printf("WRAM clear offset (SDK2) thumb not found\n");
			}
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x800,
				wramClearSignature3Alt, 1
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK3) alt found\n");
				dbg_printf("\n");
				return offset + 3;
			} else {
				dbg_printf("WRAM clear offset (SDK3) alt not found\n");
			}
		}

		if (!offset) {
			offset = findOffset(
				(u32*)ndsHeader->arm7destination, 0x800,
				wramClearSignature4, 1
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK4) found\n");
				dbg_printf("\n");
				return offset + 2;
			} else {
				dbg_printf("WRAM clear offset (SDK4) not found\n");
			}
		}

		if (!offset) {
			offset = (u32*)findOffsetThumb(
				(u16*)ndsHeader->arm7destination, 0x800,
				wramClearSignature4Thumb, 1
			);
			if (offset) {
				dbg_printf("WRAM clear offset (SDK4) thumb found\n");
				dbg_printf("\n");
				return (u32*)((u32)offset + 6);
			} else {
				dbg_printf("WRAM clear offset (SDK4) thumb not found\n");
			}
		}
	}

	dbg_printf("\n");
	return offset;
}

bool patchWramClear(const tNDSHeader* ndsHeader) {
	if (arm7newUnitCode == 0) {
		u32* offset = patchOffsetCache.wramEndAddrOffset;
		if (!patchOffsetCache.wramEndAddrOffset) {
			offset = findWramEndAddrOffset(ndsHeader);
			if (offset) {
				patchOffsetCache.wramEndAddrOffset = offset;
			}
		}
		if (offset) {
			*offset = CARDENGINE_ARM7_LOCATION;
			dbg_printf("WRAM end addr location : ");
			dbg_hexa((u32)offset);
			dbg_printf("\n\n");
		} else {
			return false;
		}
	}
	if (newArm7binarySize == 0xCAB4) {
		return true;
	}
	u32* offset = patchOffsetCache.wramClearOffset;
	if (!patchOffsetCache.wramClearOffset) {
		offset = findWramClearOffset(ndsHeader);
		if (offset) {
			patchOffsetCache.wramClearOffset = offset;
		}
	}
	if (offset) {
		bool notWithinSubroutine = (*(u16*)offset == 0x2008 || *((u16*)offset + 1) == 0xE3A0);
		bool usesThumb = (notWithinSubroutine ? (*(u16*)offset == 0x2008) : (*((u16*)offset + 1) != 0xE92D));
		if (notWithinSubroutine) {
			if (usesThumb) {
				*((u16*)offset + 20) = 0x2200;	// movs r2, #0
			} else {
				offset[13] = 0xE3A02000;	// mov r2, #0
			}
		} else {
			if (usesThumb) {
				if (arm7newUnitCode == 0) {
					*((u16*)offset + 21) = 0x2200;	// movs r2, #0
				} else {
					*((u16*)offset + 11) = 0x2100;	// movs r1, #0
					*((u16*)offset + 55) = 0x2100;	// movs r1, #0
				}
			} else {
				if (arm7newUnitCode == 0) {
					offset[*offset==0xE92D4038 ? 15 : (offset[1]==0xE24DD008 ? 18 : 17)] = 0xE3A02000;	// mov r2, #0
				} else if (offset[1]==0xE24DD008) {
					offset[10] = 0xE3A01000;	// mov r1, #0
					offset[60] = 0xE3A01000;	// mov r1, #0
				} else if (*offset==0xE92D43FE) {
					offset[12] = 0xE3A01000;	// mov r1, #0
					offset[61] = 0xE3A01000;	// mov r1, #0
				} else {
					offset[*offset==0xE92D40F8 ? 10 : 9]  = 0xE3A01000;	// mov r1, #0
					offset[*offset==0xE92D40F8 ? 44 : 43] = 0xE3A01000;	// mov r1, #0
				}
			}
		}
		dbg_printf("WRAM clear location : ");
		dbg_hexa((u32)offset);
		dbg_printf("\n\n");
	} else {
		return false;
	}
	return true;
}
