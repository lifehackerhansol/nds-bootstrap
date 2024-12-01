#include <stddef.h> // NULL
#include "patch.h"
#include "find.h"
#include "debug_file.h"

extern u8 arm7newUnitCode;
extern u32 newArm7binarySize;

//
// Subroutine function signatures ARM7
//

// Relocate
static const u32 relocateStartSignature[1]      = {0x027FFFFA};
static const u32 relocateStartSignature5[1]     = {0x3381C0DE}; //  33 81 C0 DE  DE C0 81 33 00 00 00 00 is the marker for the beggining of the relocated area :-)
static const u32 relocateStartSignature5Alt[1]  = {0x2106C0DE};
static const u32 relocateStartSignature5Alt2[1] = {0x02FFFFFA};

static const u32 nextFunctiontSignature[1] = {0xE92D4000};
static const u32 relocateValidateSignature[1] = {0x400010C};

// irq enable
static const u32 irqEnableStartSignature1[4]      = {0xE59FC028, 0xE1DC30B0, 0xE3A01000, 0xE1CC10B0}; // SDK <= 3
static const u32 irqEnableStartSignature4[4]      = {0xE92D4010, 0xE1A04000, 0xEBFFFFF6, 0xE59FC020}; // SDK >= 4
static const u32 irqEnableStartSignature4Alt[4]   = {0xE92D4010, 0xE1A04000, 0xEBFFFFE9, 0xE59FC020}; // SDK 5
static const u16 irqEnableStartSignatureThumb[5]  = {0xB530, 0xB081, 0x4D07, 0x882C, 0x2100};
static const u16 irqEnableStartSignatureThumb3[5] = {0xB510, 0x1C04, 0xF7FF, 0xFFF4, 0x4B05}; // SDK 3
static const u16 irqEnableStartSignatureThumb5[5] = {0xB510, 0x1C04, 0xF7FF, 0xFFE4, 0x4B05}; // SDK 5

bool a7GetReloc(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	extern u32 vAddrOfRelocSrc;
	extern u32 relocDestAtSharedMem;

	if (isSdk5(moduleParams)) {
		// Find the relocation signature
		u32 relocationStart = patchOffsetCache.relocateStartOffset;
		if (!patchOffsetCache.relocateStartOffset) {
			relocationStart = (u32)findOffset(
				(u32*)ndsHeader->arm7destination, 0x800,
				relocateStartSignature5, 1
			);
			if (!relocationStart) {
				dbg_printf("Relocation start not found. Trying alt\n");
				relocationStart = (u32)findOffset(
					(u32*)ndsHeader->arm7destination, 0x800,
					relocateStartSignature5Alt, 1
				);
				if (relocationStart) relocationStart += 0x28;
			}

			if (!relocationStart) {
				dbg_printf("Relocation start not found. Trying alt 2\n");
				relocationStart = (u32)findOffset(
					(u32*)ndsHeader->arm7destination, 0x800,
					relocateStartSignature5Alt2, 1
				);
				if (relocationStart) {
					int i = 0;
					while ((*(u32*)relocationStart != 0) && (i < 0x100)) {
						relocationStart += 4;
						i += 4;
					}
					if (*(u32*)relocationStart != 0) {
						relocationStart = 0;
					} else {
						relocationStart -= 8;
					}
				}
			}

			if (relocationStart) {
				patchOffsetCache.relocateStartOffset = relocationStart;
			}
		}
		if (!relocationStart) {
			dbg_printf("Relocation start alt 2 not found\n");
			return false;
		}

		// Validate the relocation signature
		vAddrOfRelocSrc = relocationStart + 0x8;
		// sanity checks
		u32 relocationCheck = patchOffsetCache.relocateValidateOffset;
		if (!patchOffsetCache.relocateValidateOffset) {
			relocationCheck = (u32)findOffset(
				(u32*)ndsHeader->arm7destination, newArm7binarySize,
				relocateValidateSignature, 1
			);
			if (relocationCheck) {
				patchOffsetCache.relocateValidateOffset = relocationCheck;
			}
		}
		u32 relocationCheck2 =
			*(u32*)(relocationCheck - 0x4);

		relocDestAtSharedMem = 0x37F8000;
		if (relocationCheck + 0xC - vAddrOfRelocSrc + relocDestAtSharedMem > relocationCheck2) {
			relocationCheck -= 4;
		}
		if (relocationCheck + 0xC - vAddrOfRelocSrc + relocDestAtSharedMem > relocationCheck2) {
			relocationCheck += 4;
			dbg_printf("Error in relocation checking\n");
			dbg_hexa(relocationCheck + 0xC - vAddrOfRelocSrc + relocDestAtSharedMem);
			dbg_printf(" ");
			dbg_hexa(relocationCheck2);
			dbg_printf("\n");

			vAddrOfRelocSrc =  relocationCheck + 0xC - relocationCheck2 + relocDestAtSharedMem;
			dbg_printf("vAddrOfRelocSrc: ");
		} else {
			dbg_printf("Relocation src: ");
		}
		dbg_hexa(vAddrOfRelocSrc);
		dbg_printf("\n");

		return true;
	}

	// Find the relocation signature
    u32 relocationStart = patchOffsetCache.relocateStartOffset;
	if (!patchOffsetCache.relocateStartOffset) {
		relocationStart = (u32)findOffset(
			(u32*)ndsHeader->arm7destination, newArm7binarySize,
			relocateStartSignature, 1
		);

		if (relocationStart) {
			patchOffsetCache.relocateStartOffset = relocationStart;
		}
	}
	if (!relocationStart) {
		dbg_printf("Relocation start not found\n");
		return false;
	}

    // Validate the relocation signature
	u32 forwardedRelocStartAddr = relocationStart + 4;
	while (!*(u32*)forwardedRelocStartAddr || *(u32*)forwardedRelocStartAddr < 0x02000000 || *(u32*)forwardedRelocStartAddr > 0x03000000) {
		forwardedRelocStartAddr += 4;
	}
	vAddrOfRelocSrc = *(u32*)(forwardedRelocStartAddr + 8);
    
    dbg_printf("forwardedRelocStartAddr\n");
    dbg_hexa(forwardedRelocStartAddr);   
    dbg_printf("\nvAddrOfRelocSrc\n");
    dbg_hexa(vAddrOfRelocSrc);
    dbg_printf("\n");  
	
	// Sanity checks
	u32 relocationCheck1 = *(u32*)(forwardedRelocStartAddr + 0xC);
	u32 relocationCheck2 = *(u32*)(forwardedRelocStartAddr + 0x10);
	if (vAddrOfRelocSrc != relocationCheck1 || vAddrOfRelocSrc != relocationCheck2) {
		dbg_printf("Error in relocation checking method 1\n");
		
		// Find the beginning of the next function
		u32 nextFunction = patchOffsetCache.relocateValidateOffset;
		if (!patchOffsetCache.relocateValidateOffset) {
			nextFunction = (u32)findOffset(
				(u32*)relocationStart, newArm7binarySize,
				nextFunctiontSignature, 1
			);
			if (nextFunction) {
				patchOffsetCache.relocateValidateOffset = nextFunction;
			}
		}
	
		// Validate the relocation signature
		forwardedRelocStartAddr = nextFunction - 0x14;
		
		// Validate the relocation signature
		vAddrOfRelocSrc = *(u32*)(nextFunction - 0xC);
		
		// Sanity checks
		relocationCheck1 = *(u32*)(forwardedRelocStartAddr + 0xC);
		relocationCheck2 = *(u32*)(forwardedRelocStartAddr + 0x10);
		if (vAddrOfRelocSrc != relocationCheck1 || vAddrOfRelocSrc != relocationCheck2) {
			dbg_printf("Error in relocation checking method 2\n");
			return false;
		}
	}

	// Get the remaining details regarding relocation
	u32 valueAtRelocStart = *(u32*)forwardedRelocStartAddr;
	relocDestAtSharedMem = *(u32*)valueAtRelocStart;
	if (relocDestAtSharedMem != 0x37F8000) { // Shared memory in RAM
		// Try again
		vAddrOfRelocSrc += *(u32*)(valueAtRelocStart + 4);
		relocDestAtSharedMem = *(u32*)(valueAtRelocStart + 0xC);
		if (relocDestAtSharedMem != 0x37F8000) {
			dbg_printf("Error in finding shared memory relocation area\n");
			return false;
		}
	}

	dbg_printf("Relocation src: ");
	dbg_hexa(vAddrOfRelocSrc);
	dbg_printf("\n");
	dbg_printf("Relocation dst: ");
	dbg_hexa(relocDestAtSharedMem);
	dbg_printf("\n");

	return true;
}

u32* findCardIrqEnableOffset(const tNDSHeader* ndsHeader, const module_params_t* moduleParams) {
	dbg_printf("findCardIrqEnableOffset:\n");
	
	const u32* irqEnableStartSignature = irqEnableStartSignature1;
	if (ndsHeader->arm7binarySize != 0x289C0 && moduleParams->sdk_version > 0x4000000) {
		irqEnableStartSignature = irqEnableStartSignature4;
	}

	u32* cardIrqEnableOffset = findOffset(
		(u32*)ndsHeader->arm7destination, newArm7binarySize,
		irqEnableStartSignature, 4
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
            irqEnableStartSignature4, 4
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
            irqEnableStartSignature4Alt, 4
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
            irqEnableStartSignatureThumb, 5
		);
		if (cardIrqEnableOffset) {
			// Find again
			cardIrqEnableOffset = (u32*)findOffsetThumb(
				(u16*)cardIrqEnableOffset+4, newArm7binarySize,
				irqEnableStartSignatureThumb, 5
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
            irqEnableStartSignatureThumb3, 5
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
            irqEnableStartSignatureThumb5, 5
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
