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
