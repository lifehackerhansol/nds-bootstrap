#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"
#include "value_bits.h"

extern u32 _io_dldi_features;
extern u32 newArm7binarySize;

// Sleep patch
static const u32 sleepPatch[2]         = {0x0A000001, 0xE3A00601};
static const u16 sleepPatchThumb[2]    = {0xD002, 0x4831};
static const u16 sleepPatchThumbAlt[2] = {0xD002, 0x0440};

// Sleep input write
static const u32 sleepInputWriteEndSignature1[2]     = {0x04000136, 0x027FFFA8};
static const u32 sleepInputWriteEndSignature5[2]     = {0x04000136, 0x02FFFFA8};
static const u32 sleepInputWriteSignature[1]         = {0x13A04902};
static const u32 sleepInputWriteSignatureAlt[1]      = {0x11A05004};
static const u16 sleepInputWriteBeqSignatureThumb[1] = {0xD000};

static u32* findSleepPatchOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findSleepPatchOffset:\n");

	u32* sleepPatchOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		sleepPatch, 2
	);
	if (sleepPatchOffset) {
		dbg_printf("Sleep patch found\n");
	} else {
		dbg_printf("Sleep patch not found\n");
	}

	dbg_printf("\n");
	return sleepPatchOffset;
}

static u16* findSleepPatchOffsetThumb(const tNDSHeader* ndsHeader) {
	dbg_printf("findSleepPatchOffsetThumb:\n");
	
	u16* sleepPatchOffset = findOffsetThumb(
		(u16*)ndsHeader->arm7destination, newArm7binarySize,
		sleepPatchThumb, 2
	);
	if (sleepPatchOffset) {
		dbg_printf("Thumb sleep patch thumb found\n");
	} else {
		dbg_printf("Thumb sleep patch thumb not found\n");
	}

	if (!sleepPatchOffset) {
		sleepPatchOffset = findOffsetThumb(
			(u16*)ndsHeader->arm7destination, newArm7binarySize,
			sleepPatchThumbAlt, 2
		);
		if (sleepPatchOffset) {
			dbg_printf("Thumb sleep patch thumb alt found\n");
		} else {
			dbg_printf("Thumb sleep patch thumb alt not found\n");
		}
	}

	dbg_printf("\n");
	return sleepPatchOffset;
}

static u32* findSleepInputWriteOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	dbg_printf("findSleepInputWriteOffset:\n");

	u32* offset = NULL;
	u32* endOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		isSdk5(moduleParams) ? sleepInputWriteEndSignature5 : sleepInputWriteEndSignature1, 2
	);
	if (endOffset) {
		offset = findOffsetBackwards(
			endOffset, 0x38,
			sleepInputWriteSignature, 1
		);
		if (!offset) {
			offset = findOffsetBackwards(
				endOffset, 0x3C,
				sleepInputWriteSignatureAlt, 1
			);
		}
		if (!offset) {
			u32 thumbOffset = (u32)findOffsetBackwardsThumb(
				(u16*)endOffset, 0x30,
				sleepInputWriteBeqSignatureThumb, 1
			);
			if (thumbOffset) {
				thumbOffset += 2;
				offset = (u32*)thumbOffset;
			}
		}
	}
	if (offset) {
		dbg_printf("Sleep input write found\n");
	} else {
		dbg_printf("Sleep input write not found\n");
	}

	dbg_printf("\n");
	return offset;
}

void patchSleepMode(const tNDSHeader* ndsHeader) {
	// Sleep
	u32* sleepPatchOffset = patchOffsetCache.sleepPatchOffset;
	if (!patchOffsetCache.sleepPatchOffset) {
		sleepPatchOffset = findSleepPatchOffset(ndsHeader);
		if (!sleepPatchOffset) {
			dbg_printf("Trying thumb...\n");
			sleepPatchOffset = (u32*)findSleepPatchOffsetThumb(ndsHeader);
		}
		patchOffsetCache.sleepPatchOffset = sleepPatchOffset;
	}
	if ((_io_dldi_features & 0x00000010) || forceSleepPatch) {
		if (sleepPatchOffset) {
			// Patch
			*((u16*)sleepPatchOffset + 2) = 0;
			*((u16*)sleepPatchOffset + 3) = 0;

			dbg_printf("Sleep location : ");
			dbg_hexa((u32)sleepPatchOffset);
			dbg_printf("\n\n");
		}
	}
}

void patchSleepInputWrite(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	u32* offset = patchOffsetCache.sleepInputWriteOffset;
	if (!patchOffsetCache.sleepInputWriteOffset) {
		offset = findSleepInputWriteOffset(ndsHeader, moduleParams);
		if (offset) {
			patchOffsetCache.sleepInputWriteOffset = offset;
		}
	}
	if (!offset) {
		return;
	}

	if (!sleepMode) {
		if (*offset == 0x13A04902 || *offset == 0x11A05004) {
			*offset = 0xE1A00000; // nop
		} else {
			u16* offsetThumb = (u16*)offset;
			*offsetThumb = 0x46C0; // nop
		}
	}

	dbg_printf("Sleep input write location : ");
	dbg_hexa((u32)offset);
	dbg_printf("\n\n");
}
