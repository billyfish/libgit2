/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_amigaos4_compat_h__
#define INCLUDE_amigaos4_compat_h__

#include <sys/types.h>

/*
struct timespec
{
	unsigned int tv_sec;
	unsigned int tv_nsec;
};
*/

struct p_timespec {
	time_t tv_sec;
	long tv_nsec;
};


struct p_timeval {
	time_t tv_sec;
	long tv_usec;
};

#define timespec p_timespec


#endif
