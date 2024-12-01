#include <nds/ndstypes.h>
#include <nds/system.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"
#include "tonccpy.h"

extern u16 a9ScfgRom;
extern u32 newArm7binarySize;
extern u32 vAddrOfRelocSrc;
extern u32 relocDestAtSharedMem;

u32 newSwiGetPitchTableAddr = 0;

// SWI
static const u32 swi12Signature[1] = {0x4770DF12}; // LZ77UnCompReadByCallbackWrite16bit

static const u16 swiGetPitchTableSignatureThumb[2]  = {0xDF1B,0x4770};
static const u32 swiGetPitchTableSignature1[4]      = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004721};
static const u32 swiGetPitchTableSignature1Alt1[4]  = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004ACD};
static const u32 swiGetPitchTableSignature1Alt2[4]  = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004BB9};
static const u32 swiGetPitchTableSignature1Alt3[4]  = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004BC9};
static const u32 swiGetPitchTableSignature1Alt4[4]  = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004BE5};
static const u32 swiGetPitchTableSignature1Alt5[4]  = {0xE59FC004, 0xE08FC00C, 0xE12FFF1C, 0x00004CA5};
static const u32 swiGetPitchTableSignature1Alt6[3]  = {0xE59FC000, 0xE12FFF1C, 0x038039D5};
static const u32 swiGetPitchTableSignature1Alt7[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803BE9};
static const u32 swiGetPitchTableSignature1Alt8[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803E05};
static const u32 swiGetPitchTableSignature1Alt9[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803E09};
static const u32 swiGetPitchTableSignature1Alt10[3] = {0xE59FC000, 0xE12FFF1C, 0x03803F21};
static const u32 swiGetPitchTableSignature1Alt11[3] = {0xE59FC000, 0xE12FFF1C, 0x03804189};
static const u32 swiGetPitchTableSignature1Alt12[3] = {0xE59FC000, 0xE12FFF1C, 0x038049D5};
static const u32 swiGetPitchTableSignature1Alt13[3] = {0xE59FC000, 0xE12FFF1C, 0x03804BE9};
static const u32 swiGetPitchTableSignature1Alt14[3] = {0xE59FC000, 0xE12FFF1C, 0x03804E35};
static const u32 swiGetPitchTableSignature1Alt15[3] = {0xE59FC000, 0xE12FFF1C, 0x03800D89};
static const u32 swiGetPitchTableSignature3[3]      = {0xE59FC000, 0xE12FFF1C, 0x03800FD5};
static const u32 swiGetPitchTableSignature3Alt1[3]  = {0xE59FC000, 0xE12FFF1C, 0x03801149};
static const u32 swiGetPitchTableSignature3Alt2[3]  = {0xE59FC000, 0xE12FFF1C, 0x03801215};
static const u32 swiGetPitchTableSignature3Alt3[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804119};
static const u32 swiGetPitchTableSignature3Alt4[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804301};
static const u32 swiGetPitchTableSignature3Alt5[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804305};
static const u32 swiGetPitchTableSignature3Alt6[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804395};
static const u32 swiGetPitchTableSignature3Alt7[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804439};
static const u32 swiGetPitchTableSignature3Alt8[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804559};
static const u32 swiGetPitchTableSignature3Alt9[3]  = {0xE59FC000, 0xE12FFF1C, 0x03804615};
static const u32 swiGetPitchTableSignature3Alt10[3] = {0xE59FC000, 0xE12FFF1C, 0x038053E1};
static const u32 swiGetPitchTableSignature3Alt11[3] = {0xE59FC000, 0xE12FFF1C, 0x03805485};
static const u32 swiGetPitchTableSignature3Alt12[3] = {0xE59FC000, 0xE12FFF1C, 0x038055A5};
static const u32 swiGetPitchTableSignature4[3]      = {0xE59FC000, 0xE12FFF1C, 0x038006A1};
static const u32 swiGetPitchTableSignature4Alt1[3]  = {0xE59FC000, 0xE12FFF1C, 0x03800811};
static const u32 swiGetPitchTableSignature4Alt2[3]  = {0xE59FC000, 0xE12FFF1C, 0x03800919};
static const u32 swiGetPitchTableSignature4Alt3[3]  = {0xE59FC000, 0xE12FFF1C, 0x03800925};
static const u32 swiGetPitchTableSignature4Alt4[3]  = {0xE59FC000, 0xE12FFF1C, 0x038035C5};
static const u32 swiGetPitchTableSignature4Alt5[3]  = {0xE59FC000, 0xE12FFF1C, 0x038035ED};
static const u32 swiGetPitchTableSignature4Alt6[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803715};
static const u32 swiGetPitchTableSignature4Alt7[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803829};
static const u32 swiGetPitchTableSignature4Alt8[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803DC1};
static const u32 swiGetPitchTableSignature4Alt9[3]  = {0xE59FC000, 0xE12FFF1C, 0x03803ED5};
static const u32 swiGetPitchTableSignature4Alt10[3] = {0xE59FC000, 0xE12FFF1C, 0x03803F15};
static const u32 swiGetPitchTableSignature5[4]      = {0x781A4B06, 0xD3030791, 0xD20106D1, 0x1A404904};

static u16 swi12Patch[2] =
{
	0xDF02, // SWI  0x02
	0x4770, // BX LR
};

static u16 swiGetPitchTablePatch[8] =
{
	0x4902, // LDR  R1, =0x46A
	0x1A40, // SUBS R0, R0, R1
	0xDF1B, // SWI  0x1B
	0x0400, // LSLS R0, R0, #0x10
	0x0C00, // LSRS R0, R0, #0x10
	0x4770, // BX LR
	0x046A,
	0
};

static u32* a7_findSwi12Offset(const tNDSHeader* ndsHeader) {
	dbg_printf("findSwi12Offset:\n");

	u32* swi12Offset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
		swi12Signature, 1
	);
	if (swi12Offset) {
		dbg_printf("swi 0x12 call found\n");
	} else {
		dbg_printf("swi 0x12 call not found\n");
	}

	dbg_printf("\n");
	return swi12Offset;
}

static u16* findSwiGetPitchTableThumbOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findSwiGetPitchTableThumbOffset:\n");

	u16* offset = findOffsetThumb(
		(u16*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
		swiGetPitchTableSignatureThumb, 2
	);

	if (offset) {
		dbg_printf("swiGetPitchTable thumb found\n");
	} else {
		dbg_printf("swiGetPitchTable thumb not found\n");
	}

	dbg_printf("\n");
	return offset;
}

static u32* findSwiGetPitchTableOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	dbg_printf("findSwiGetPitchTableOffset:\n");

	u32* swiGetPitchTableOffset = NULL;

	if (moduleParams->sdk_version > 0x5000000) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature5, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable call SDK 5 found\n");
		} else {
			dbg_printf("swiGetPitchTable call SDK 5 not found\n");
		}
	}

	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call not found\n");
		}
	}

	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt1, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 1 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 1 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt2, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 2 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 2 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt3, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 3 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 3 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt4, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 4 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 4 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt5, 4
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 5 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 5 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt6, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 6 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 6 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt7, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 7 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 7 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt8, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 8 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 8 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt9, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 9 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 9 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt10, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 10 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 10 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt11, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 11 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 11 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt12, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 12 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 12 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt13, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 13 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 13 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt14, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 14 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 14 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature1Alt15, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 15 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK <= 2 call alt 15 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt1, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 1 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 1 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt2, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 2 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 2 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt3, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 3 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 3 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt4, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 4 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 4 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt5, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 5 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 5 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt6, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 6 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 6 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt7, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 7 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 7 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt8, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 8 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 8 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt9, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 9 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 9 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt10, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 10 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 10 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt11, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 11 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 11 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature3Alt12, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 3 call alt 12 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 3 call alt 12 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt1, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 1 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 1 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt2, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 2 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 2 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt3, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 3 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 3 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt4, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 4 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 4 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt5, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 5 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 5 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt6, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 6 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 6 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt7, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 7 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 7 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt8, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 8 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 8 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt9, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 9 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 9 not found\n");
		}
	}
	if (!swiGetPitchTableOffset) {
		swiGetPitchTableOffset = findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize > 0x10000 ? 0x10000 : newArm7binarySize,
			swiGetPitchTableSignature4Alt10, 3
		);
		if (swiGetPitchTableOffset) {
			dbg_printf("swiGetPitchTable SDK 4 call alt 10 found\n");
		} else {
			dbg_printf("swiGetPitchTable SDK 4 call alt 10 not found\n");
		}
	}

	dbg_printf("\n");
	return swiGetPitchTableOffset;
}

void fixForDSiBios(const cardengineArm7* ce7, const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	u32* swi12Offset = patchOffsetCache.a7Swi12Offset;
	u32* swiGetPitchTableOffset = patchOffsetCache.swiGetPitchTableOffset;
	if (!patchOffsetCache.a7Swi12Offset) {
		swi12Offset = a7_findSwi12Offset(ndsHeader);
		if (swi12Offset) {
			patchOffsetCache.a7Swi12Offset = swi12Offset;
		}
	}
	if (!patchOffsetCache.swiGetPitchTableChecked) {
		if (patchOffsetCache.a7IsThumb && !isSdk5(moduleParams)) {
			swiGetPitchTableOffset = (u32*)findSwiGetPitchTableThumbOffset(ndsHeader);
		} else {
			swiGetPitchTableOffset = findSwiGetPitchTableOffset(ndsHeader, moduleParams);
		}
		if (swiGetPitchTableOffset) {
			patchOffsetCache.swiGetPitchTableOffset = swiGetPitchTableOffset;
		}
		patchOffsetCache.swiGetPitchTableChecked = true;
	}

	if (a9ScfgRom == 1 && REG_SCFG_ROM != 0x703) {
		// swi 0x12 call
		if (swi12Offset) {
			// Patch to call swi 0x02 instead of 0x12
			tonccpy(swi12Offset, swi12Patch, 0x4);
		}

		// swi get pitch table
		if (swiGetPitchTableOffset) {
			// Patch
			if (isSdk5(moduleParams)) {
				toncset16(swiGetPitchTableOffset, 0x46C0, 6);
			} else if (patchOffsetCache.a7IsThumb) {
				tonccpy((u16*)newSwiGetPitchTableAddr, swiGetPitchTablePatch, 0x10);
				u32 srcAddr = (u32)swiGetPitchTableOffset - vAddrOfRelocSrc + 0x37F8000;
				u32 dstAddr = (u32)newSwiGetPitchTableAddr - vAddrOfRelocSrc + 0x37F8000;
				const u16* swiGetPitchTableBranch = generateA7InstrThumb(srcAddr, dstAddr);
				tonccpy(swiGetPitchTableOffset, swiGetPitchTableBranch, 0x4);

				dbg_printf("swiGetPitchTable new location : ");
				dbg_hexa(newSwiGetPitchTableAddr);
				dbg_printf("\n\n");
			} else {
				tonccpy(swiGetPitchTableOffset, ce7->patches->j_twlGetPitchTable, 0xC);
			}
		}
	}

	dbg_printf("swi12 location : ");
	dbg_hexa((u32)swi12Offset);
	dbg_printf("\n\n");
	dbg_printf("swiGetPitchTable location : ");
	dbg_hexa((u32)swiGetPitchTableOffset);
	dbg_printf("\n\n");
}
