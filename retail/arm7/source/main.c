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
#include <string.h>
#include <calico.h>
#include <nds.h>

// Registers for Power Management (I2C_PM)
#define I2CREGPM_MMCPWR         0x12
#define I2CREGPM_RESETFLAG      0x70

//static vu32* wordCommandAddr;

static Thread ndsBootstrapPxiThread7;
alignas(8) static u8 ndsBootstrapPxiThread7Stack[1024];

static int ndsBootstrapPxiThreadMain(void* arg) {
	// Set up PXI mailbox, used to receive PXI command words
	Mailbox ndsBootstrapPxiMailbox;
	u32 ndsBootstrapPxiMailboxData[8];
	mailboxPrepare(&ndsBootstrapPxiMailbox, ndsBootstrapPxiMailboxData, sizeof(ndsBootstrapPxiMailboxData)/sizeof(u32));
	pxiSetMailbox(PxiChannel_User0, &ndsBootstrapPxiMailbox);

	// Main PXI message loop
	for (;;) {
		// Receive a message
		u32 msg = mailboxRecv(&ndsBootstrapPxiMailbox) & 0xFF;
		u32 rc = 1;

		switch (msg) {
			// 0: set SCFG clk
			case 0: {
				REG_SCFG_CLK = 0x0181;
				rc = 0;

				break;
			}

			// 1: get SCFG_CLK
			case 1: {
				rc = REG_SCFG_CLK;
				break;
			}

			// 2: get SCFG_EXT
			case 2: {
				rc = REG_SCFG_EXT;
				break;
			}
			default:
				break;
		}

		// Send a reply back to the ARM9
		pxiReply(PxiChannel_User0, rc);
	}

	return 0;
}

int main(void) {
	*(u16*)0x02FFFC30 = *(vu16*)0x4004700; // SNDEXCNT (Used for checking for regular DS or DSi/3DS in DS mode)

	// Grab from DS header in GBA slot
	*(u16*)0x02FFFC36 = *(u16*)0x0800015E;	// Header CRC16
	*(u32*)0x02FFFC38 = *(u32*)0x0800000C;	// Game Code

	*(u32*)0x02FFFDF0 = REG_SCFG_EXT;

	// Read settings from NVRAM
	envReadNvramSettings();

	// Set up extended keypad server (X/Y/hinge)
	keypadStartExtServer();

	// Configure and enable VBlank interrupt
	lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
	irqEnable(IRQ_VBLANK);

	// Set up RTC
	rtcInit();
	rtcSyncTime();

	// Initialize power management
	pmInit();

	// Set up block device peripherals
	blkInit();

	if (isDSiMode()) {
		i2cLock();
		i2cWriteRegister8(I2cDev_MCU, I2CREGPM_MMCPWR, 0);		// Press power button for auto-reset
		i2cUnlock();
	}

	if (isDSiMode() && REG_SCFG_EXT == 0) {
		u32 wordBak = *(vu32*)0x037C0000;
		*(vu32*)0x037C0000 = 0x414C5253;
		if (*(vu32*)0x037C0000 == 0x414C5253 && *(vu32*)0x037C8000 != 0x414C5253) {
			*(u32*)0x02FFE1A0 = 0x080037C0;
		}
		*(vu32*)0x037C0000 = wordBak;
	}

	// Keep the ARM7 mostly idle
	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
