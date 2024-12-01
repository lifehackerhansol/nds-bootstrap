#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u8 arm7newUnitCode;
extern u32 newArm7binarySize;

// RAM clear
// static const u32 ramClearSignature[2]    = {0xE12FFF1E, 0x027FF000};
static const u32 ramClearSignatureTwl[2] = {0x02FFC000, 0x02FFF000};

static u32* findRamClearOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findRamClearOffset:\n");

	u32* ramClearOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		/* (arm7newUnitCode > 0) ? */ ramClearSignatureTwl /* : ramClearSignature */, 2
	);
	if (ramClearOffset) {
		dbg_printf("RAM clear found\n");
	} else {
		dbg_printf("RAM clear not found\n");
	}

	dbg_printf("\n");
	return ramClearOffset;
}

void patchRamClear(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
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
