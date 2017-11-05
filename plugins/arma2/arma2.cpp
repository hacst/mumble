// Copyright 2005-2017 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "../mumble_plugin_win32_32bit.h"

procptr32_t posptr, frontptr, topptr;

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, MumbleString*, MumbleWideString*) {
	for (int i=0;i<3;i++)
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;

	// char state;
	bool ok;

	/*
	float front_corrector1;
	float front_corrector2;
	float front_corrector3;

	float top_corrector1;
	float top_corrector2;
	float top_corrector3;
	*/

	/*
		value is 0 when one is not in a game, 4 when one is

	ok = peekProc((BYTE *) 0x, &state, 1); // Magical state value
	if (! ok)
		return false;

	if (state == 0)
		return true; // This results in all vectors beeing zero which tells Mumble to ignore them.
	*/

	ok = peekProc(posptr, avatar_pos, 12) &&
	     peekProc(frontptr, avatar_front, 12) &&
	     peekProc(topptr, avatar_top, 12);

	if (avatar_pos[1] > 999000000.0)
		return false;

	/*
	peekProc(frontptr, &front_corrector1, 4) &&
	peekProc(frontptr + 0xC, &front_corrector2, 4) &&
	peekProc(frontptr + 0x18, &front_corrector3, 4) &&
	peekProc(topptr, &top_corrector1, 4) &&
	peekProc(topptr + 0xC, &top_corrector2, 4) &&
	peekProc(topptr + 0x18, &top_corrector3, 4);
	*/

	if (! ok)
		return false;

	/*
	avatar_front[0] = front_corrector1;
	avatar_front[1] = front_corrector2;
	avatar_front[2] = front_corrector3;

	avatar_top[0] = top_corrector1;
	avatar_top[1] = top_corrector2;
	avatar_top[2] = top_corrector3;
	*/

	for (int i=0;i<3;i++) {
		camera_pos[i] = avatar_pos[i];
		camera_front[i] = avatar_front[i];
		camera_top[i] = avatar_top[i];
	}

	return true;
}

static int trylock(const MumblePIDLookup lookupFunc, const MumblePIDLookupContext lookupContext) {
	posptr = 0;

	if (! initialize(lookupFunc, lookupContext, L"arma2.exe"))
		return false;

	/*
	BYTE bState;
	peekProc((BYTE *) 0x00BF64D0, &bState, 1);
	if (bState == 0)
		return false;
	*/

	/*
	   Comment out code we don't need
	   BYTE *pModule=getModuleAddr(L"<module name, if you need it>.dll");
	   if (!pModule)
	*/

	procptr32_t ptr1 = peekProc<procptr32_t>(0x00C500FC);

	procptr32_t ptr2 = peekProc<procptr32_t>(ptr1 + 0x88);

	procptr32_t base = ptr2 + 0x10;

	posptr = base + 0x18;
	frontptr = base;
	topptr = base + 0xC;

	float apos[3], afront[3], atop[3], cpos[3], cfront[3], ctop[3];
	MumbleString context;
	MumbleWideString identity;

	if (fetch(apos, afront, atop, cpos, cfront, ctop, &context, &identity)) {
		return true;
	} else {
		generic_unlock();
		return false;
	}
}

static MumblePlugin arma2plug = {
	MUMBLE_PLUGIN_MAGIC,
	1,
	false,
	MumbleInitConstWideString(L"ArmA 2"),
	MumbleInitConstWideString(L"ArmA 1.08"),
	MumbleInitConstWideString(L"Supports Armed Assault 2. No identity or context support yet."),
	fetch,
	trylock,
	generic_unlock,
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &arma2plug;
}
