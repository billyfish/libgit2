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

struct TimerIFace *ITimer;
struct Device * TimerBase;

struct Library * SocketBase;
struct SocketIFace *ISocket;


static int amiga_open_lib (struct Library **library_pp, const char *lib_name_s, const uint32_t lib_version, struct Interface **interface_pp, const char * const interface_name_s, const uint32_t interface_version);

static void amiga_close_lib (struct Library *library_p, struct Interface *interface_p);



double
difftime(time_t t1,time_t t0)
{
	double result;

	result = ((double)t1) - ((double)t0);

	return(result);
}


int amiga_init (void)
{
	int ret = -1;

	if (amiga_open_lib (&SocketBase, "socket.library", 52L, (struct Interface **) &ISocket, "main", 1)) {
		ret = 0;
	}

	return ret;
}


void amiga_exit (void)
{
	amiga_close_lib (SocketBase, (struct Interface *) ISocket);
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

