// Copyright 2005-2017 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "../mumble_plugin_win32_32bit.h"

bool is_steam = false;

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, MumbleString*, MumbleWideString*) {
	for (int i=0;i<3;i++)
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;

	//char ccontext[128];
	//char state;
	bool ok;

	/*
	ok = peekProc((BYTE *) 0x0, &state, 1); // Magical state value
	if (! ok)
		return false;
	*/

	// Find out whether this is the steam version
	char sMagic[6];
	if (!peekProc(0x015715b4, sMagic, 6)) {
		generic_unlock();
		return false;
	}

	is_steam = (strncmp("Score:", sMagic, 6) == 0);

	if (is_steam) {
		ok = peekProc(0x01571E90, avatar_pos, 12) &&
		     peekProc(0x01571E80, avatar_front, 12) &&
		     peekProc(0x01571E70, avatar_top, 12);
	} else {
		ok = peekProc(0x01579600, avatar_pos, 12) &&
		     peekProc(0x015795F0, avatar_front, 12) &&
		     peekProc(0x015795E0, avatar_top, 12);
	}

	if (! ok)
		return false;

	// Disable when not in game
	if (avatar_pos[1] == 9999)
		return true;

	/*
	    Get context string; in this plugin this will be an
	    ip:port (char 256 bytes) string

	ccontext[127] = 0;
	context = std::string(ccontext);
	*/
	/*
	if (state == 0)
		return true; // This results in all vectors beeing zero which tells Mumble to ignore them.
	*/

	for (int i=0;i<3;i++) {
		camera_pos[i] = avatar_pos[i];
		camera_front[i] = avatar_front[i];
		camera_top[i] = avatar_top[i];
	}

	return ok;
}

static int trylock(const MumblePIDLookup lookupFunc, const MumblePIDLookupContext lookupContext) {

	if (! initialize(lookupFunc, lookupContext, L"BFBC2Game.exe"))
		return false;

	float apos[3], afront[3], atop[3], cpos[3], cfront[3], ctop[3];
	MumbleString context;
	MumbleWideString identity;

	if (!fetch(apos, afront, atop, cpos, cfront, ctop, &context, &identity)) {
		generic_unlock();
		return false;
	}

	return true;
}

static MumblePlugin bfbc2plug = {
	MUMBLE_PLUGIN_MAGIC,
	1,
	true,
	L"Battlefield Bad Company 2",
	L"Build 795745",
	L"Supports Battlefield Bad Company 2. No identity or context support.",
	fetch,
	trylock,
	generic_unlock,
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &bfbc2plug;
}
