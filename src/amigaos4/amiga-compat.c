/*
** Copyright 2014-2020 The Earlham Institute
** 
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** 
**     http://www.apache.org/licenses/LICENSE-2.0
** 
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*
 * amiga-compat.c
 *
 *  Created on: 15 May 2020
 *      Author: billy
 */

#include "inttypes.h"

#include "amigaos4/amiga-compat.h"

#include <proto/timer.h>
#include <proto/bsdsocket.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

struct Device *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;

struct Library *SocketBase = NULL;
struct SocketIFace *ISocket = NULL;

struct Library *UtilityBase = NULL;
struct UtilityIFace *IUtility = NULL;

struct Library *DOSBase = NULL;
struct DOSIFace *IDOS = NULL;


static struct TimeRequest *s_tr_p = NULL;

static int amiga_open_lib (struct Library **library_pp, const char *lib_name_s, const uint32_t lib_version, struct Interface **interface_pp, const char * const interface_name_s, const uint32_t interface_version);

static void amiga_close_lib (struct Library *library_p, struct Interface *interface_p);

static int amiga_open_timer (void);

static void amiga_close_timer (void);


double difftime (time_t t0, time_t t1)
{
	double result = ((double) t0) - ((double) t1);

	return result;
}


int amiga_init (void)
{
	int ret = -1;

	if (amiga_open_lib (&DOSBase, "dos.library", 52L, (struct Interface **) &IDOS, "main", 1) == 0) {
		if (amiga_open_lib (&UtilityBase, "utility.library", 52L, (struct Interface **) &IUtility, "main", 1) == 0) {
			if (amiga_open_lib (&SocketBase, "socket.library", 52L, (struct Interface **) &ISocket, "main", 1) == 0) {
				if (amiga_open_timer () == 0) {
					ret = 0;
				}
			}
		}
	}

	return ret;
}


void amiga_exit (void)
{
	amiga_close_timer ();
	amiga_close_lib (SocketBase, (struct Interface *) ISocket);
	amiga_close_lib (UtilityBase, (struct Interface *) IUtility);
	amiga_close_lib (DOSBase, (struct Interface *) IDOS);
}




static int amiga_open_lib (struct Library **library_pp, const char *lib_name_s, const uint32_t lib_version, struct Interface **interface_pp, const char * const interface_name_s, const uint32_t interface_version)
{
	int ret = -1;
	if ((*library_pp = IExec->OpenLibrary (lib_name_s, lib_version)) != NULL) {
		if ((*interface_pp = IExec->GetInterface (*library_pp, interface_name_s, interface_version, NULL)) != NULL) {
			ret = 0;
		} else {
			IExec->CloseLibrary (*library_pp);
		}
	}

	return ret;
}


static void amiga_close_lib (struct Library *library_p, struct Interface *interface_p)
{
	if (interface_p) {
		IExec->DropInterface (interface_p);
	}

	if (library_p) {
		IExec->CloseLibrary (library_p);
	}
}



static int amiga_open_timer (void)
{
	int ret = -1;

	s_tr_p = IExec->AllocSysObjectTags (ASOT_IOREQUEST,
		ASOIOR_Size, sizeof (struct TimeRequest),
		ASOIOR_ReplyPort, NULL,
		TAG_END);

	if (s_tr_p != NULL) {
		if (! (IExec->OpenDevice ("timer.device", UNIT_VBLANK, (struct IORequest *) s_tr_p, 0) )) {
			TimerBase = s_tr_p -> Request.io_Device;
			ITimer = (struct TimerIFace*) IExec->GetInterface ((struct Library *) TimerBase, "main", 1, NULL);
			ret = 0;
		}
	}

	return ret;
}


static void amiga_close_timer (void)
{
	IExec->DropInterface ((struct Interface *) ITimer);
	IExec->CloseDevice ((struct IORequest *) s_tr_p);
	IExec->FreeSysObject (ASOT_IOREQUEST, s_tr_p);
}
