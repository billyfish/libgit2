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


#include <proto/amisslmaster.h>

#include <libraries/amisslmaster.h>

#include "debugging_utils.h"

struct Device *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;

struct Library *SocketBase = NULL;
struct SocketIFace *ISocket = NULL;

struct Library *UtilityBase = NULL;
struct UtilityIFace *IUtility = NULL;

struct Library *DOSBase = NULL;
struct DOSIFace *IDOS = NULL;

struct Library *AmiSSLBase = NULL;
struct AmiSSLIFace *IAmiSSL = NULL;

struct Library *AmiSSLMasterBase = NULL;
struct AmiSSLMasterIFace *IAmiSSLMaster = NULL;

static struct TimeRequest *s_time_req_p = NULL;
static struct MsgPort *s_timer_msg_port_p = NULL;

static BOOL s_amissl_init_flag = FALSE;


static int amiga_open_lib (struct Library **library_pp, const char *lib_name_s, const uint32_t lib_version, struct Interface **interface_pp, const char * const interface_name_s, const uint32_t interface_version);

static void amiga_close_lib (struct Library *library_p, struct Interface *interface_p);

static int amiga_open_timer (void);

static void amiga_close_timer (void);


static int amiga_init_ssl (void);

static void amiga_exit_ssl (void);




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
			if (amiga_open_lib (&SocketBase, "bsdsocket.library", 4L, (struct Interface **) &ISocket, "main", 1) == 0) {
				if (amiga_init_ssl () == 0) {
					if (amiga_open_timer () == 0) {
						ret = 0;
					} else {
						DB (KPRINTF ("%s %ld - Failed to init Timer Device\n", __FILE__, __LINE__));
					}
				} else {
					DB (KPRINTF ("%s %ld - Failed to init SSL\n", __FILE__, __LINE__));
				}
			} else {
				DB (KPRINTF ("%s %ld - Failed to init Socket library\n", __FILE__, __LINE__));
			}
		} else {
			DB (KPRINTF ("%s %ld - Failed to init Utility library\n", __FILE__, __LINE__));
		}
	} else {
		DB (KPRINTF ("%s %ld - Failed to init DOS library\n", __FILE__, __LINE__));
	}

	return ret;
}


void amiga_exit (void)
{
	amiga_close_timer ();
	amiga_exit_ssl ();
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
			DB (KPRINTF ("%s %ld - IExec->GetInterface failed for interface \"%s\" version %lu\n", __FILE__, __LINE__, interface_name_s, interface_version));
		}
	} else {
		DB (KPRINTF ("%s %ld - IExec->OpenLibrary failed for \"%s\" version %lu\n", __FILE__, __LINE__, lib_name_s, lib_version));
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

	s_timer_msg_port_p = (struct MsgPort *) IExec->AllocSysObjectTags (ASOT_PORT,
		ASOPORT_Name, "Timer port",
		TAG_DONE);

	if (s_timer_msg_port_p != NULL) {
		s_time_req_p = (struct TimeRequest *) IExec->AllocSysObjectTags (ASOT_IOREQUEST,
			ASOIOR_Size, sizeof (struct TimeRequest),
			ASOIOR_ReplyPort, s_timer_msg_port_p,
			TAG_END);

		if (s_time_req_p != NULL) {
			if (! (IExec->OpenDevice ("timer.device", UNIT_VBLANK, (struct IORequest *) s_time_req_p, 0) )) {
				TimerBase = s_time_req_p -> Request.io_Device;
				ITimer = (struct TimerIFace*) IExec->GetInterface ((struct Library *) TimerBase, "main", 1, NULL);

				if (ITimer != NULL) {
					ret = 0;
				} else {
					DB (KPRINTF ("%s %ld - IExec->GetInterface failed to get ITimer\n", __FILE__, __LINE__));
				}

			} else {
				DB (KPRINTF ("%s %ld - IExec->OpenDevice failed to open timer.device\n", __FILE__, __LINE__));
			}

		} else {
			IExec->FreeSysObject (ASOT_PORT, s_timer_msg_port_p);
			DB (KPRINTF ("%s %ld - Failed to allocate s_time_req_p\n", __FILE__, __LINE__));
		}

	} else {
		DB (KPRINTF ("%s %ld - Failed to allocate s_timer_msg_port_p\n", __FILE__, __LINE__));
	}

	return ret;
}


static void amiga_close_timer (void)
{
	IExec->DropInterface ((struct Interface *) ITimer);
	IExec->CloseDevice ((struct IORequest *) s_time_req_p);
	IExec->FreeSysObject (ASOT_IOREQUEST, s_time_req_p);
	IExec->FreeSysObject (ASOT_PORT, s_timer_msg_port_p);
}


static int amiga_init_ssl (void)
{
	int ret = -1;

	if (amiga_open_lib (&AmiSSLMasterBase, "amisslmaster.library", AMISSLMASTER_MIN_VERSION, (struct Interface **) &IAmiSSLMaster, "main", 1) == 0) {
		if (IAmiSSLMaster->InitAmiSSLMaster (AMISSL_CURRENT_VERSION, TRUE)) {
			if ((AmiSSLBase = IAmiSSLMaster -> OpenAmiSSL ()) != NULL) {
				if ((IAmiSSL = (struct AmiSSLIFace *) IExec->GetInterface (AmiSSLBase, "main", 1, NULL))  != NULL) {
					if (InitAmiSSL (AmiSSL_ErrNoPtr, &errno, AmiSSL_ISocket, ISocket, TAG_DONE) == 0) {
						ret = 0;
						s_amissl_init_flag = TRUE;
					} else {
						DB (KPRINTF ("%s %ld - InitAmiSSL failed\n", __FILE__, __LINE__));
					}
				} else {
					DB (KPRINTF ("%s %ld - GetInterface for IAmiSSL failed\n", __FILE__, __LINE__));
				}

			} else {
				DB (KPRINTF ("%s %ld - IAmiSSLMaster -> OpenAmiSSL () failed\n", __FILE__, __LINE__));
			}

		} else {
			DB (KPRINTF ("%s %ld - InitAmiSSLMaster () failed\n", __FILE__, __LINE__));
		}

	} else {
		DB (KPRINTF ("%s %ld - Failed to open AmiSSLMaster Library\n", __FILE__, __LINE__));
	}

	return ret;
}


static void amiga_exit_ssl (void)
{
	if (s_amissl_init_flag) {
		CleanupAmiSSL (TAG_DONE);
	}

	if (AmiSSLBase) {
		if (IAmiSSL) {
			IExec->DropInterface ((struct Interface *) IAmiSSL);
		}

		IAmiSSLMaster->CloseAmiSSL ();
	}

	amiga_close_lib (AmiSSLMasterBase, (struct Interface *) IAmiSSLMaster);
}
