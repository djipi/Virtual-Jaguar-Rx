//
// DAC (really, Synchronous Serial Interface) Handler
//
// Originally by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Rewritten by James Hammons
// (C) 2010 Underground Software
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  PL = PvtLewis <from Atari Age>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JLH  04/30/2012  Changed SDL audio handler to run JERRY
// JPM  March/2022  Added the save state patch from PvtLewis
//

// Need to set up defaults that the BIOS sets for the SSI here in DACInit()... !!! FIX !!!
// or something like that... Seems like it already does, but it doesn't seem to
// work correctly...! Perhaps just need to set up SSI stuff so BUTCH doesn't get
// confused...

// After testing on a real Jaguar, it seems clear that the I2S interrupt drives
// the audio subsystem. So while you can drive the audio at a *slower* rate than
// set by SCLK, you can't drive it any *faster*. Also note, that if the I2S
// interrupt is not enabled/running on the DSP, then there is no audio. Also,
// audio can be muted by clearing bit 8 of JOYSTICK (JOY1).
//
// Approach: We can run the DSP in the host system's audio IRQ, by running the
// DSP for the alloted time (depending on the host buffer size & sample rate)
// by simply reading the L/R_I2S (L/RTXD) registers at regular intervals. We
// would also have to time the I2S/TIMER0/TIMER1 interrupts in the DSP as well.
// This way, we can run the host audio IRQ at, say, 48 KHz and not have to care
// so much about SCLK and running a separate buffer and all the attendant
// garbage that comes with that awful approach.
//
// There would still be potential gotchas, as the SCLK can theoretically drive
// the I2S at 26590906 / 2 (for SCLK == 0) = 13.3 MHz which corresponds to an
// audio rate 416 KHz (dividing the I2S rate by 32, for 16-bit stereo). It
// seems doubtful that anything useful could come of such a high rate, and we
// can probably safely ignore any such ridiculously high audio rates. It won't
// sound the same as on a real Jaguar, but who cares? :-)

#include "dac.h"

#include "SDL.h"
#include "cdrom.h"
#include "dsp.h"
#include "event.h"
#include "jerry.h"
#include "jaguar.h"
#include "log.h"
#include "m68000/m68kinterface.h"
//#include "memory.h"
#include "settings.h"
#include "state.h"

//#define DEBUG_DAC

#define BUFFER_SIZE			0x10000				// Make the DAC buffers 64K x 16 bits
#define DAC_AUDIO_RATE		48000				// Set the audio rate to 48 KHz

// Jaguar memory locations

#define LTXD			0xF1A148
#define RTXD			0xF1A14C
#define LRXD			0xF1A148
#define RRXD			0xF1A14C
#define SCLK			0xF1A150
#define SMODE			0xF1A154

// Global variables

// These are defined in memory.h/cpp
//uint16_t lrxd, rrxd;							// I2S ports (into Jaguar)

// Local variables

static SDL_AudioSpec desired;
static bool SDLSoundInitialized;
//static uint8_t SCLKFrequencyDivider = 19;			// Default is roughly 22 KHz (20774 Hz in NTSC mode)
// /*static*/ uint16_t serialMode = 0;

// Private function prototypes

void SDLSoundCallback(void * userdata, Uint8 * buffer, int length);
void DSPSampleCallback(void);

static Uint8 * sampleBuffer = NULL;
static int bufferIndex = 0;
static int numberOfSamples = 0;
static bool bufferDone = false;


size_t dac_dump(FILE *fp)
{
	size_t total_dumped = 0;

#if 0
	DUMPINT(bufferIndex);
	DUMPBOOL(bufferDone);
	DUMPINT(numberOfSamples);
	int length = numberOfSamples * 2;
	if (fwrite(sampleBuffer, 1, length, fp) != length)
	{
		WriteLog("DUMP sampleBuffer error\n");
		return -1;
	}
	total_dumped += length;
#endif

	return total_dumped;
}

size_t dac_load(FILE *fp)
{
size_t total_loaded = 0;

#if 0 // we shouldn't be touching the current sample buffer
	LOADINT(bufferIndex);
	LOADBOOL(bufferDone);
	LOADINT(numberOfSamples);
	//DACReset();
	char tmpbuf[4096*4];
	int length = numberOfSamples * 2;
	if (length > sizeof(tmpbuf))
	{
		length = sizeof(tmpbuf);
	}
	if (fread(tmpbuf, 1, length, fp) != length)
	{
		WriteLog("LOAD sampleBuffer error\n");
		return -1;
	}
	total_loaded += length;
	if (bufferIndex == numberOfSamples)
	{
		bufferDone = true;
	}
	if (!bufferDone && sampleBuffer != NULL)
	{
		memcpy(sampleBuffer, tmpbuf, length);
	}

	WriteLog("dac_load index: %d   done: %d   numberOfSamples: %d   length: %d\n", bufferIndex, bufferDone, numberOfSamples, length);
#endif

	return total_loaded;
}


//
// Initialize the SDL sound system
//
void DACInit(void)
{
	SDLSoundInitialized = false;

//	if (!vjs.audioEnabled)
	if (!vjs.DSPEnabled)
	{
		WriteLog("DAC: DSP/host audio playback disabled.\n");
		return;
	}

	desired.freq = DAC_AUDIO_RATE;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;						// 2K buffer = audio delay of 42.67 ms (@ 48 KHz)
	desired.callback = SDLSoundCallback;

	if (SDL_OpenAudio(&desired, NULL) < 0)		// NULL means SDL guarantees what we want
		WriteLog("DAC: Failed to initialize SDL sound...\n");
	else
	{
		SDLSoundInitialized = true;
		DACReset();
		SDL_PauseAudio(false);					// Start playback!
		WriteLog("DAC: Successfully initialized. Sample rate: %u\n", desired.freq);
	}

	ltxd = lrxd = desired.silence;
	sclk = 19;									// Default is roughly 22 KHz

	uint32_t riscClockRate = (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL);
	uint32_t cyclesPerSample = riscClockRate / DAC_AUDIO_RATE;
	WriteLog("DAC: RISC clock = %u, cyclesPerSample = %u\n", riscClockRate, cyclesPerSample);
}


//
// Reset the sound buffer FIFOs
//
void DACReset(void)
{
//	LeftFIFOHeadPtr = LeftFIFOTailPtr = 0, RightFIFOHeadPtr = RightFIFOTailPtr = 1;
	ltxd = lrxd = desired.silence;
}


//
// Pause/unpause the SDL audio thread
//
void DACPauseAudioThread(bool state/*= true*/)
{
		SDL_PauseAudio(state);
}


//
// Close down the SDL sound subsystem
//
void DACDone(void)
{
	if (SDLSoundInitialized)
	{
		SDL_PauseAudio(true);
		SDL_CloseAudio();
	}

	WriteLog("DAC: Done.\n");
}


// Approach: Run the DSP for however many cycles needed to correspond to whatever sample rate
// we've set the audio to run at. So, e.g., if we run it at 48 KHz, then we would run the DSP
// for however much time it takes to fill the buffer. So with a 2K buffer, this would correspond
// to running the DSP for 0.042666... seconds. At 26590906 Hz, this would correspond to
// running the DSP for 1134545 cycles. You would then sample the L/RTXD registers every
// 1134545 / 2048 = 554 cycles to fill the buffer. You would also have to manage interrupt
// timing as well (generating them at the proper times), but that shouldn't be too difficult...
// If the DSP isn't running, then fill the buffer with L/RTXD and exit.

//
// SDL callback routine to fill audio buffer
//
// Note: The samples are packed in the buffer in 16 bit left/16 bit right pairs.
//       Also, length is the length of the buffer in BYTES
//

volatile bool dac_load_state = false;
volatile bool dac_dump_state = false;

void SDLSoundCallback(void * userdata, Uint8 * buffer, int length)
{
	WriteLog("SDLSoundCallback called: length: %d  load: %d  dump: %d\n", length, dac_load_state, dac_dump_state);

	// 1st, check to see if the DSP is running. If not, fill the buffer with L/RXTD and exit.

	if (!DSPIsRunning())
	{
		for(int i=0; i<(length/2); i+=2)
		{
			((uint16_t *)buffer)[i + 0] = ltxd;
			((uint16_t *)buffer)[i + 1] = rtxd;
		}

		return;
	}

	// The length of time we're dealing with here is 1/48000 s, so we multiply this
	// by the number of cycles per second to get the number of cycles for one sample.
//	uint32_t riscClockRate = (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL);
//	uint32_t cyclesPerSample = riscClockRate / DAC_AUDIO_RATE;
	// This is the length of time
//	timePerSample = (1000000.0 / (double)riscClockRate) * ();

	// Now, run the DSP for that length of time for each sample we need to make

	bufferIndex = 0;
	sampleBuffer = buffer;
// If length is the length of the sample buffer in BYTES, then shouldn't the # of
// samples be / 4? No, because we bump the sample count by 2, so this is OK.
	numberOfSamples = length / 2;
	bufferDone = false;

	SetCallbackTime(DSPSampleCallback, 1000000.0 / (double)DAC_AUDIO_RATE, EVENT_JERRY);

	// These timings are tied to NTSC, need to fix that in event.cpp/h! [FIXED]
	do
	{
		double timeToNextEvent = GetTimeToNextEvent(EVENT_JERRY);

		if (vjs.DSPEnabled)
		{
			if (vjs.usePipelinedDSP)
				DSPExecP2(USEC_TO_RISC_CYCLES(timeToNextEvent));
			else
				DSPExec(USEC_TO_RISC_CYCLES(timeToNextEvent));
		}

		HandleNextEvent(EVENT_JERRY);
	}
	while (!bufferDone);

	if (dac_load_state)
	{
		dac_load_state = false;
	}
	else
	{
		if (dac_dump_state)
		{
			dac_dump_state = false;
		}
	}
}


void DSPSampleCallback(void)
{
	if (bufferIndex >= numberOfSamples)
	{
		bufferDone = true;
		//return;
	}
	else
	{
		((uint16_t *)sampleBuffer)[bufferIndex + 0] = ltxd;
		((uint16_t *)sampleBuffer)[bufferIndex + 1] = rtxd;
		bufferIndex += 2;

		if (bufferIndex == numberOfSamples)
		{
			bufferDone = true;
			//return;
		}
		else
		{
			SetCallbackTime(DSPSampleCallback, 1000000.0 / (double)DAC_AUDIO_RATE, EVENT_JERRY);
		}
	}
}


#if 0
//
// Calculate the frequency of SCLK * 32 using the divider
//
int GetCalculatedFrequency(void)
{
	int systemClockFrequency = (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL);

	// We divide by 32 here in order to find the frequency of 32 SCLKs in a row (transferring
	// 16 bits of left data + 16 bits of right data = 32 bits, 1 SCLK = 1 bit transferred).
	return systemClockFrequency / (32 * (2 * (SCLKFrequencyDivider + 1)));
}
#endif


//
// LTXD/RTXD/SCLK/SMODE ($F1A148/4C/50/54)
//
void DACWriteByte(uint32_t offset, uint8_t data, uint32_t who/*= UNKNOWN*/)
{
	WriteLog("DAC: %s writing BYTE %02X at %08X\n", whoName[who], data, offset);
	if (offset == SCLK + 3)
		DACWriteWord(offset - 3, (uint16_t)data);
}


void DACWriteWord(uint32_t offset, uint16_t data, uint32_t who/*= UNKNOWN*/)
{
	if (offset == LTXD + 2)
	{
		ltxd = data;
	}
	else if (offset == RTXD + 2)
	{
		rtxd = data;
	}
	else if (offset == SCLK + 2)					// Sample rate
	{
		WriteLog("DAC: Writing %u to SCLK (by %s)...\n", data, whoName[who]);

		sclk = data & 0xFF;
		JERRYI2SInterruptTimer = -1;
		RemoveCallback(JERRYI2SCallback);
		JERRYI2SCallback();
	}
	else if (offset == SMODE + 2)
	{
//		serialMode = data;
		smode = data;
		WriteLog("DAC: %s writing to SMODE. Bits: %s%s%s%s%s%s [68K PC=%08X]\n", whoName[who],
			(data & 0x01 ? "INTERNAL " : ""), (data & 0x02 ? "MODE " : ""),
			(data & 0x04 ? "WSEN " : ""), (data & 0x08 ? "RISING " : ""),
			(data & 0x10 ? "FALLING " : ""), (data & 0x20 ? "EVERYWORD" : ""),
			m68k_get_reg(NULL, M68K_REG_PC));
	}
}


//
// LRXD/RRXD/SSTAT ($F1A148/4C/50)
//
uint8_t DACReadByte(uint32_t offset, uint32_t who/*= UNKNOWN*/)
{
//	WriteLog("DAC: %s reading byte from %08X\n", whoName[who], offset);
	return 0xFF;
}


//static uint16_t fakeWord = 0;
uint16_t DACReadWord(uint32_t offset, uint32_t who/*= UNKNOWN*/)
{
//	WriteLog("DAC: %s reading word from %08X\n", whoName[who], offset);
//	return 0xFFFF;
//	WriteLog("DAC: %s reading WORD %04X from %08X\n", whoName[who], fakeWord, offset);
//	return fakeWord++;
//NOTE: This only works if a bunch of things are set in BUTCH which we currently don't
//      check for. !!! FIX !!!
// Partially fixed: We check for I2SCNTRL in the JERRY I2S routine...
//	return GetWordFromButchSSI(offset, who);
	if (offset == LRXD || offset == RRXD)
		return 0x0000;
	else if (offset == LRXD + 2)
		return lrxd;
	else if (offset == RRXD + 2)
		return rrxd;

	return 0xFFFF;	// May need SSTAT as well... (but may be a Jaguar II only feature)
}
