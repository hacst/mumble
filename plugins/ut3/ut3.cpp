// Copyright 2005-2017 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "../mumble_plugin_win32_32bit.h"

using namespace std;

procptr32_t pos0ptr, pos1ptr, pos2ptr, faceptr, topptr;
//BYTE *stateptr;

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, MumbleString*, MumbleWideString*) {
	char state;
	//char ccontext[128];
	bool ok;

	float face_corrector[3];
	float top_corrector[3];

	for (int i=0;i<3;i++)
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;

	ok = peekProc(0x01DEAFD9, &state, 1);
	if (! ok)
		return false;

	if (state == 1)
		return true;

	/*
	   Z-Value is increasing when heading north
				  decreasing when heading south
	   X-Value is increasing when heading east
				  decreasing when heading west
	   Y-Value is increasing when going up
				  decreasing when going down
	*/

	//Convert to left-handed coordinate system

	ok = peekProc(pos2ptr, avatar_pos, 4) &&	//X
	     peekProc(pos1ptr, avatar_pos+1, 4) &&	//Y
	     peekProc(pos0ptr, avatar_pos+2, 4) &&  //Z
	     peekProc(faceptr, &face_corrector, 12) &&
	     peekProc(topptr, &top_corrector, 12);

	//peekProc((BYTE *) 0x0122E0B8, ccontext, 128);

	if (! ok)
		return false;

	if (face_corrector[1] <= -0.98) {
		top_corrector[1] = -top_corrector[1];
	}
	if (face_corrector[1] >= 0.98) {
		top_corrector[1] = -top_corrector[1];
	}

	//Find north by playing on a Warfare game type - center view on the up arrow on the mini map
	avatar_front[0] = face_corrector[2];
	avatar_front[1] = face_corrector[1];
	avatar_front[2] = face_corrector[0];

	avatar_top[0] = top_corrector[2];
	avatar_top[1] = top_corrector[1];
	avatar_top[2] = top_corrector[0];

	//avatar_top[0] = top_corrector[2];
	//avatar_top[1] = top_corrector[1];

	//ccontext[127] = 0;
	//context = std::string(ccontext);

	//if (context.find(':')==string::npos)
	//	context.append(":UT3PORT");

	for (int i=0;i<3;i++) {
		camera_pos[i] = avatar_pos[i];
		camera_front[i] = avatar_front[i];
		camera_top[i] = avatar_top[i];
	}

	return true;
}

static int trylock(const MumblePIDLookup lookupFunc, const MumblePIDLookupContext lookupContext) {
	pos0ptr = pos1ptr = pos2ptr = faceptr = 0;

	if (! initialize(lookupFunc, lookupContext, L"UT3.exe", L"wrap_oal.dll"))
		return false;

	procptr32_t ptraddress = pModule + 0x8A740;
	procptr32_t baseptr = peekProc<procptr32_t>(ptraddress);

	pos0ptr = baseptr;
	pos1ptr = baseptr + 0x4;
	pos2ptr = baseptr + 0x8;
	faceptr = baseptr + 0x18;
	topptr = baseptr + 0x24;

	//stateptr = pModule + 0xC4;

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

static MumblePlugin ut3plug = {
	MUMBLE_PLUGIN_MAGIC,
	1,
	false,
	MumbleInitConstWideString(L"Unreal Tournament 3"),
	MumbleInitConstWideString(L"2.1"),
	MumbleInitConstWideString(L"Supports Unreal Tournament 3. No context or identity support yet."),
	fetch,
	trylock,
	generic_unlock
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &ut3plug;
}
