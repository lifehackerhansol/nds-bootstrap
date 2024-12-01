#include <nds/ndstypes.h>
#include <stddef.h> // NULL
#include "debug_file.h"
#include "find.h"
#include "patch.h"

extern u32 arm7mbk;
extern u32 newArm7binarySize;

// Post-boot code
static const u32 postBootStartSignature[1]      = {0xE92D47F0};
static const u16 postBootStartSignatureThumb[1] = {0xB5F8};
static const u32 postBootEndSignature[1]        = {0x04000300};

static u32* findPostBootOffset(const tNDSHeader* ndsHeader) {
	dbg_printf("findPostBootOffset:\n");

	u32* startOffset = NULL;
	u32* endOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		postBootEndSignature, 1
	);
	if (endOffset) {
		dbg_printf("Post boot end found: ");
		dbg_hexa((u32)endOffset);
		dbg_printf("\n");

		startOffset = findOffsetBackwards(
			endOffset, 0x200,
			postBootStartSignature, 1
		);
		if (startOffset) {
			dbg_printf("Post boot start found\n");
		} else {
			dbg_printf("Post boot start not found\n");
		}
		if (!startOffset) {
			startOffset = (u32*)findOffsetBackwardsThumb(
				(u16*)endOffset, 0x100,
				postBootStartSignatureThumb, 1
			);
			if (startOffset) {
				dbg_printf("Post boot start thumb found\n");
			} else {
				dbg_printf("Post boot start thumb not found\n");
			}
		}
	} else {
		dbg_printf("Post boot not found\n");
	}

	dbg_printf("\n");
	return startOffset;
}

void patchPostBoot(const tNDSHeader* ndsHeader) {
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
