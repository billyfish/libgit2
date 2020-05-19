/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_amigaos4_compat_h__
#define INCLUDE_amigaos4_compat_h__

#include <sys/types.h>


#define __USE_INLINE__
#include <proto/amissl.h>
#undef __USE_INLINE__

/*
struct timespec
{
	unsigned int tv_sec;
	unsigned int tv_nsec;
};
*/

/*
struct p_timespec {stack
	time_t tv_secs;
	long tv_nsec;
};
*/

struct p_timeval {
	time_t tv_sec;
	long tv_usec;
};

#define timespec p_timespec


int amiga_init (void);
void amiga_exit (void);

extern double difftime (time_t t1, time_t t0);

#endif
