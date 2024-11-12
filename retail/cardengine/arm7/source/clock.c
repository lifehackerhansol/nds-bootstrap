// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2005 Michael Noland (Joat)
// Copyright (C) 2005 Jason Rogers (Dovoto)
// Copyright (C) 2005 Dave Murphy (WinterMute)
// Copyright (C) 2023 Antonio Niño Díaz

#include <time.h>

#include <nds/arm7/clock.h>
#include <nds/bios.h>
#include <nds/interrupts.h>
#include <nds/ipc.h>
#include <nds/timers.h>
#include <nds/system.h>

// Delay (in swiDelay units) for each bit transfer
#define RTC_DELAY 48

void BCDToInteger(uint8_t *data, uint32_t length)
{
    for (u32 i = 0; i < length; i++)
        data[i] = (data[i] & 0xF) + ((data[i] & 0xF0) >> 4) * 10;
}

void rtcTransaction(uint8_t *command, uint32_t commandLength, uint8_t *result,
                    uint32_t resultLength)
{
    uint32_t bit;
    uint8_t data;

    // Raise CS
    REG_RTCCNT8 = RTCCNT_CS_0 | RTCCNT_SCK_1 | RTCCNT_SIO_1;
    swiDelay(RTC_DELAY);
    REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_1 | RTCCNT_SIO_1;
    swiDelay(RTC_DELAY);

    // Write command byte (high bit first)
    data = *command++;

    for (bit = 0; bit < 8; bit++)
    {
        REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_0 | RTCCNT_SIO_OUT | (data >> 7);
        swiDelay(RTC_DELAY);

        REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_1 | RTCCNT_SIO_OUT | (data >> 7);
        swiDelay(RTC_DELAY);

        data = data << 1;
    }

    // Write parameter bytes (low bit first)
    for (; commandLength > 1; commandLength--)
    {
        data = *command++;

        for (bit = 0; bit < 8; bit++)
        {
            REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_0 | RTCCNT_SIO_OUT | (data & 1);
            swiDelay(RTC_DELAY);

            REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_1 | RTCCNT_SIO_OUT | (data & 1);
            swiDelay(RTC_DELAY);

            data = data >> 1;
        }
    }

    // Read result bytes (low bit first)
    for (; resultLength > 0; resultLength--)
    {
        data = 0;

        for (bit = 0; bit < 8; bit++)
        {
            REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_0;
            swiDelay(RTC_DELAY);

            REG_RTCCNT8 = RTCCNT_CS_1 | RTCCNT_SCK_1;
            swiDelay(RTC_DELAY);

            if (REG_RTCCNT8 & RTCCNT_SIO)
                data |= (1 << bit);
        }
        *result++ = data;
    }

    // Finish up by dropping CS low
    REG_RTCCNT8 = RTCCNT_CS_0 | RTCCNT_SCK_1;
    swiDelay(RTC_DELAY);
}

void rtcTimeAndDateGet(rtcTimeAndDate *rtc)
{
    uint8_t command, status, response[7];

    command = READ_TIME_AND_DATE;
    rtcTransaction(&command, 1, response, 7);

    command = READ_STATUS_REG1;
    rtcTransaction(&command, 1, &status, 1);

    if (status & STATUS_24HRS)
        response[4] &= 0x3f;

    BCDToInteger(response, 7);

    rtc->year = response[0];
    rtc->month = response[1];
    rtc->day = response[2];
    rtc->weekday = response[3];
    rtc->hours = response[4];
    rtc->minutes = response[5];
    rtc->seconds = response[6];
}
