#include "global.h"
#include "rh_log.h"
#include "string_util.h"
#include "characters.h"
#include "malloc.h"
#include <stdarg.h>

// Tiny EWRAM bookkeeping only — line storage lives on the heap when in use.
static EWRAM_DATA u8 *sLines = NULL;
static EWRAM_DATA u16 sHead = 0;
static EWRAM_DATA u16 sCount = 0;

static const u8 sEmptyLine[] = _("");

static bool8 RhLog_EnsureBuf(void)
{
    if (sLines != NULL)
        return TRUE;

    sLines = AllocZeroed(RH_LOG_LINES * RH_LOG_LINE_LEN);
    if (sLines == NULL)
        return FALSE;

    sHead = 0;
    sCount = 0;
    return TRUE;
}

static u8 *RhLog_LineSlot(u16 slot)
{
    return &sLines[slot * RH_LOG_LINE_LEN];
}

static void RhLog_PushLine(const u8 *src)
{
    u8 *dst;
    u16 i;

    if (!RhLog_EnsureBuf())
        return;

    dst = RhLog_LineSlot(sHead);
    for (i = 0; i < RH_LOG_LINE_LEN - 1 && src[i] != EOS; i++)
        dst[i] = src[i];
    dst[i] = EOS;

    sHead = (sHead + 1) % RH_LOG_LINES;
    if (sCount < RH_LOG_LINES)
        sCount++;
}

static u8 *RhLog_AppendBounded(u8 *out, u8 *outEnd, const u8 *src)
{
    while (*src != EOS && out < outEnd)
        *out++ = *src++;
    return out;
}

void RhLog(const u8 *msg)
{
    if (msg == NULL)
        return;
    RhLog_PushLine(msg);
}

void RhLogf(const u8 *fmt, ...)
{
    u8 buf[RH_LOG_LINE_LEN];
    u8 numBuf[12];
    u8 *out;
    u8 *outEnd;
    va_list args;
    const u8 *p;
    const u8 *str;
    s32 value;

    if (fmt == NULL)
        return;

    out = buf;
    outEnd = buf + (RH_LOG_LINE_LEN - 1);
    va_start(args, fmt);

    for (p = fmt; *p != EOS && out < outEnd;)
    {
        if (*p != CHAR_PERCENT)
        {
            *out++ = *p++;
            continue;
        }

        p++; // skip %
        if (*p == EOS)
        {
            *out++ = CHAR_PERCENT;
            break;
        }

        if (*p == CHAR_PERCENT)
        {
            *out++ = CHAR_PERCENT;
            p++;
            continue;
        }

        if (*p == CHAR_d)
        {
            value = va_arg(args, int);
            ConvertIntToDecimalStringN(numBuf, value, STR_CONV_MODE_LEFT_ALIGN, 10);
            out = RhLog_AppendBounded(out, outEnd, numBuf);
            p++;
        }
        else if (*p == CHAR_x || *p == CHAR_X)
        {
            value = va_arg(args, int);
            ConvertIntToHexStringN(numBuf, value, STR_CONV_MODE_LEFT_ALIGN, 8);
            out = RhLog_AppendBounded(out, outEnd, numBuf);
            p++;
        }
        else if (*p == CHAR_s || *p == CHAR_S)
        {
            str = va_arg(args, const u8 *);
            if (str == NULL)
                str = sEmptyLine;
            out = RhLog_AppendBounded(out, outEnd, str);
            p++;
        }
        else
        {
            *out++ = CHAR_PERCENT;
            if (out < outEnd)
                *out++ = *p;
            p++;
        }
    }

    va_end(args);
    *out = EOS;
    RhLog_PushLine(buf);
}

void RhLog_Clear(void)
{
    sHead = 0;
    sCount = 0;
    if (sLines != NULL)
    {
        Free(sLines);
        sLines = NULL;
    }
}

u16 RhLog_GetCount(void)
{
    return sCount;
}

const u8 *RhLog_GetLine(u16 index)
{
    u16 start;

    if (sLines == NULL || index >= sCount)
        return sEmptyLine;

    start = (sHead + RH_LOG_LINES - sCount) % RH_LOG_LINES;
    return RhLog_LineSlot((start + index) % RH_LOG_LINES);
}
