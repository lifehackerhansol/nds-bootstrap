/*---------------------------------------------------------------------------------

default ARM7 core

Copyright (C) 2005 - 2010
	Michael Noland (joat)
	Jason Rogers (dovoto)
	Dave Murphy (WinterMute)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
	must not claim that you wrote the original software. If you use
	this software in a product, an acknowledgment in the product
	documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
	must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
	distribution.

---------------------------------------------------------------------------------*/

//#include <stdio.h>
#include <stdlib.h> // NULL
#include <nds/ndstypes.h>
#include <nds/arm7/input.h>
#include <nds/fifocommon.h>
#include <nds/system.h>
#include <nds/arm7/clock.h>
#include <nds/arm7/i2c.h>
#include <nds/debug.h>

#include "hex.h"
#include "fifocheck.h"

//static vu32* wordCommandAddr;

void VcountHandler(void) {
	inputGetAndSend();
}

void myFIFOValue32Handler(u32 value, void* userdata) {
	nocashMessage("myFIFOValue32Handler");
 	nocashMessage("default");
	nocashMessage("fifoSendValue32");
	fifoSendValue32(FIFO_USER_02, *(u16*)value);
}

int main(void) {
	*(u16*)0x02FFFC30 = *(vu16*)0x4004700; // SNDEXCNT (Used for checking for regular DS or DSi/3DS in DS mode)

	// Grab from DS header in GBA slot
	*(u16*)0x02FFFC36 = *(u16*)0x0800015E;	// Header CRC16
	*(u32*)0x02FFFC38 = *(u32*)0x0800000C;	// Game Code

	*(u32*)0x02FFFDF0 = REG_SCFG_EXT;

	*(vu32*)0x4004820 = 0x8B7F0305;
	*(u8*)0x02FFFDF4 = (*(vu32*)0x4004820 != 0) ? 1 : 0; // SD/MMC access is enabled in SCFG
	*(vu32*)0x4004820 = 0;

	// read User Settings from firmware
	readUserSettings();
	irqInit();

	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();

	SetYtrigger(80);

	installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);

	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	if (isDSiMode()) {
		i2cWriteRegister(I2C_PM, I2CREGPM_MMCPWR, 0);		// Press power button for auto-reset
		//i2cWriteRegister(I2C_PM, I2CREGPM_MMCPWR, 1);		// Have IRQ check for power button press
		//i2cWriteRegister(I2C_PM, I2CREGPM_RESETFLAG, 1);		// SDK 5 --> Bootflag = Warmboot/SkipHealthSafety
	}

	if (isDSiMode() && REG_SCFG_EXT == 0) {
		u32 wordBak = *(vu32*)0x037C0000;
		*(vu32*)0x037C0000 = 0x414C5253;
		if (*(vu32*)0x037C0000 == 0x414C5253 && *(vu32*)0x037C8000 != 0x414C5253) {
			*(u32*)0x02FFE1A0 = 0x080037C0;
		}
		*(vu32*)0x037C0000 = wordBak;
	}

	swiIntrWait(0, IRQ_FIFO_NOT_EMPTY);

	SCFGFifoCheck();

	fifoSendValue32(FIFO_USER_05, 1);

	fifoSetValue32Handler(FIFO_USER_01, myFIFOValue32Handler, NULL);

	// Keep the ARM7 mostly idle
	while (1) {
		swiIntrWait(0, IRQ_FIFO_NOT_EMPTY);
	}
	
	return 0;
}
