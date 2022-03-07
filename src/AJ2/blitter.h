
#pragma once

//#define USE_MIDSUMMER_BLITTER
#define USE_MIDSUMMER_BLITTER_MKII

#ifdef USE_MIDSUMMER_BLITTER
extern void BlitterMidsummer(uint32_t cmd);
#endif
extern void BlitterMidsummer2(void);
