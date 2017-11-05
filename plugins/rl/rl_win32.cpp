// Copyright 2005-2017 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "../mumble_plugin_win32_32bit.h"

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, MumbleString*, MumbleWideString*) {
	for (int i=0;i<3;i++)
		avatar_pos[i]=avatar_front[i]=avatar_top[i]=camera_pos[i]=camera_front[i]=camera_top[i]=0.0f;

	// Boolean value to check if game addresses retrieval is successful
	bool ok;

	// Avatar pointers
	procptr32_t avatar_base = peekProc<procptr32_t>(pModule + 0x174269C);
	if (!avatar_base) return false;
	procptr32_t avatar_offset_0 = peekProc<procptr32_t>(avatar_base + 0x448);
	if (!avatar_offset_0) return false;
	procptr32_t avatar_offset_1 = peekProc<procptr32_t>(avatar_offset_0 + 0x440);
	if (!avatar_offset_1) return false;
	procptr32_t avatar_offset_2 = peekProc<procptr32_t>(avatar_offset_1 + 0x0);
	if (!avatar_offset_2) return false;
	procptr32_t avatar_offset = peekProc<procptr32_t>(avatar_offset_2 + 0x1C);
	if (!avatar_offset) return false;

	// Peekproc and assign game addresses to our containers, so we can retrieve positional data
	ok = peekProc(avatar_offset + 0x0, avatar_pos, 12) && // Avatar Position values (X, Y and Z).
			peekProc(pModule + 0x17428D8, camera_pos, 12) && // Camera Position values (X, Y and Z).
			peekProc(avatar_offset + 0xC, avatar_front, 12) && // Avatar Front values (X, Y and Z).
			peekProc(pModule + 0x17428C0, camera_front, 12) && // Camera Front Vector values (X, Y and Z).
			peekProc(pModule + 0x17428CC, camera_top, 12); // Camera Top Vector values (X, Y and Z).

	// This prevents the plugin from linking to the game in case something goes wrong during values retrieval from memory addresses.
	if (! ok)
		return false;

	avatar_top[2] = -1; // This tells Mumble to automatically calculate top vector using front vector.

	// Scale from centimeters to meters
	for (int i=0;i<3;i++) {
		avatar_pos[i]/=100.0f;
		camera_pos[i]/=100.0f;
	}

	return true;
}

static int trylock(const MumblePIDLookup lookupFunc, const MumblePIDLookupContext lookupContext) {

	if (! initialize(lookupFunc, lookupContext, L"RocketLeague.exe")) // Link the game executable
		return false;

	// Check if we can get meaningful data from it
	float apos[3], afront[3], atop[3], cpos[3], cfront[3], ctop[3];
	MumbleWideString sidentity;
	MumbleString scontext;

	if (fetch(apos, afront, atop, cpos, cfront, ctop, &scontext, &sidentity)) {
		return true;
	} else {
		generic_unlock();
		return false;
	}
}

static MumblePlugin rlplug = {
	MUMBLE_PLUGIN_MAGIC,
	1,
	false,
	MumbleInitConstWideString(L"Rocket League"),
	MumbleInitConstWideString(L"1.29"),
	MumbleInitConstWideString(L"Supports Rocket League without context or identity support yet."),
	fetch,
	trylock,
	generic_unlock
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &rlplug;
}
