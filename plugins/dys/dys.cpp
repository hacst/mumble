// Copyright 2005-2017 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "../mumble_plugin_win32_32bit.h"

using namespace std;

procptr32_t posptr, rotptr, stateptr, hostptr;

static bool calcout(float *pos, float *rot, float *opos, float *front, float *top) {
	float h = rot[0];
	float v = rot[1];

	if ((v < -360.0f) || (v > 360.0f) || (h < -360.0f) || (h > 360.0f))
		return false;

	h *= static_cast<float>(M_PI / 180.0f);
	v *= static_cast<float>(M_PI / 180.0f);

	// Seems Dystopia is in inches. INCHES?!?
	opos[0] = pos[0] / 39.37f;
	opos[1] = pos[2] / 39.37f;
	opos[2] = pos[1] / 39.37f;

	front[0] = cos(v) * cos(h);
	front[1] = -sin(h);
	front[2] = sin(v) * cos(h);

	h -= static_cast<float>(M_PI / 2.0f);

	top[0] = cos(v) * cos(h);
	top[1] = -sin(h);
	top[2] = sin(v) * cos(h);

	return true;
}

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, MumbleString *context, MumbleWideString*) {
	for (int i=0;i<3;i++)
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;

	float ipos[3], rot[3];
	bool ok;
	char state;
	char chHostStr[40];
	string sHost;
	wostringstream new_identity;
	ostringstream new_context;

	ok = peekProc(posptr, ipos, 12) &&
	     peekProc(rotptr, rot, 12) &&
	     peekProc(stateptr, &state, 1) &&
	     peekProc(hostptr, chHostStr, 40);
	if (!ok)
		return false;

	chHostStr[39] = 0;

	sHost.assign(chHostStr);
	if (sHost.find(':')==string::npos)
		sHost.append(":27015");

	new_context << "<context>"
	            << "<game>dys</game>"
	            << "<hostport>" << sHost << "</hostport>"
	            << "</context>";
	MumbleStringAssign(context, new_context.str());

	/* TODO
	new_identity << "<identity>"
	             << "<name>" << "SAS" << "</name>"
	             << "</identity>";
	identity = new_identity.str(); */

	// Check to see if you are spawned
	if (state == 0 || state == 2)
		return true; // Deactivate plugin

	ok = calcout(ipos, rot, avatar_pos, avatar_front, avatar_top);
	if (ok) {
		for (int i=0;i<3;++i) {
			camera_pos[i] = avatar_pos[i];
			camera_front[i] = avatar_front[i];
			camera_top[i] = avatar_top[i];
		}
		return true;
	}

	return false;
}

static int trylock(const MumblePIDLookup lookupFunc, const MumblePIDLookupContext lookupContext) {
	posptr = rotptr = 0;

	if (! initialize(lookupFunc, lookupContext, L"hl2.exe", L"client.dll"))
		return false;

	procptr32_t mod_engine=getModuleAddr(L"engine.dll");
	if (!mod_engine)
		return false;

	// Check if we really have Dystopia running
	/*
		position tuple:		client.dll+0x423990  (x,y,z, float)
		orientation tuple:	client.dll+0x423924  (v,h float)
		ID string:			client.dll+0x3c948e = "DysObjective@@" (14 characters, text)
		spawn state:        client.dll+0x3c6270  (0 when at main menu, 2 when not spawned, 6 when spawned, byte)
		host string:		engine.dll+0x3909c4 (ip:port zero-terminated string)
	*/

	// Remember addresses for later
	posptr = pModule + 0x4A3330;
	rotptr = pModule + 0x454E04;
	stateptr = pModule + 0x4518A0;
	hostptr = mod_engine + 0x3C2A84;

	//Gamecheck
	char sMagic[14];
	if (!peekProc(pModule + 0x463726, sMagic, 14) || strncmp("DysObjective@@", sMagic, 14)!=0)
		return false;

	// Check if we can get meaningful data from it
	float apos[3], afront[3], atop[3];
	float cpos[3], cfront[3], ctop[3];
	MumbleWideString sidentity;
	MumbleString scontext;

	if (fetch(apos, afront, atop, cpos, cfront, ctop, &scontext, &sidentity)) {
		return true;
	} else {
		generic_unlock();
		return false;
	}
}

static MumblePlugin dysplug = {
	MUMBLE_PLUGIN_MAGIC,
	1,
	false,
	L"Dystopia",
	L"Build 4104",
	L"Supports Dystopia. No identity support yet.",
	fetch,
	trylock,
	generic_unlock
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &dysplug;
}
