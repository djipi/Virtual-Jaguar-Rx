//
// state.h: Machine state save/load support
//
// by James L. Hammons
// Additions by Jean-Paul Mari
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JPM  Sept./2018  Added savestate features
//

#ifndef __STATE_H__
#define __STATE_H__

// Savestate functions
extern bool SaveState(unsigned char *FileName);
extern bool LoadState(unsigned char *FileName);

#endif	// __STATE_H__
