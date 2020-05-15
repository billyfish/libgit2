/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_amigaos4_compat_h__
#define INCLUDE_amigaos4_compat_h__

#include <sys/types.h>


struct p_timespec {
	time_t tv_secs;
	long tv_nsec;
};

#define timespec p_timespec


int amiga_init (void);
void amiga_exit (void);


#endif
