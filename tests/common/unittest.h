// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

#ifndef LIBPMEMSTREAM_UNITTEST_H
#define LIBPMEMSTREAM_UNITTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libpmemstream.h"
#include "test_backtrace.h"

#include <errno.h>
#include <fcntl.h>
#include <libpmem2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* XXX: refactor to use __start (https://stackoverflow.com/questions/15919356/c-program-start) */
#define START() test_register_sighandlers()

static inline void UT_OUT(const char *format, ...)
{
	va_list args_list;
	va_start(args_list, format);
	vfprintf(stdout, format, args_list);
	va_end(args_list);

	fprintf(stdout, "\n");
}

static inline void UT_FATAL(const char *format, ...)
{
	va_list args_list;
	va_start(args_list, format);
	vfprintf(stderr, format, args_list);
	va_end(args_list);

	fprintf(stderr, "\n");

	abort();
}

// XXX: use pmemstream_errormsg()
/* assert a condition is true at runtime */
#define UT_ASSERT(cnd)                                                                                                 \
	((void)((cnd) ||                                                                                               \
		(UT_FATAL("%s:%d %s - assertion failure: %s, errormsg: %s", __FILE__, __LINE__, __func__, #cnd,        \
			  pmem2_errormsg()),                                                                           \
		 0)))

// XXX: use pmemstream_errormsg()
/* assertion with extra info printed if assertion fails at runtime */
#define UT_ASSERTinfo(cnd, info)                                                                                       \
	((void)((cnd) ||                                                                                               \
		(UT_FATAL("%s:%d %s - assertion failure: %s (%s = %s), errormsg: %s", __FILE__, __LINE__, __func__,    \
			  #cnd, #info, info, pmem2_errormsg),                                                          \
		 0)))

// XXX: use pmemstream_errormsg()
/* assert two integer values are equal at runtime */
#define UT_ASSERTeq(lhs, rhs)                                                                                          \
	((void)(((lhs) == (rhs)) ||                                                                                    \
		(UT_FATAL("%s:%d %s - assertion failure: %s (0x%llx) == %s "                                           \
			  "(0x%llx), errormsg: %s",                                                                    \
			  __FILE__, __LINE__, __func__, #lhs, (unsigned long long)(lhs), #rhs,                         \
			  (unsigned long long)(rhs), pmem2_errormsg()),                                                \
		 0)))

// XXX: use pmemstream_errormsg()
/* assert two integer values are not equal at runtime */
#define UT_ASSERTne(lhs, rhs)                                                                                          \
	((void)(((lhs) != (rhs)) ||                                                                                    \
		(UT_FATAL("%s:%d %s - assertion failure: %s (0x%llx) != %s "                                           \
			  "(0x%llx), errormsg: %s",                                                                    \
			  __FILE__, __LINE__, __func__, #lhs, (unsigned long long)(lhs), #rhs,                         \
			  (unsigned long long)(rhs), pmem2_errormsg()),                                                \
		 0)))

/* Creates a pmem2_map from a file path, with given size.
 * XXX: add granularity (and FILE_MODE?) as a param
 */
static inline struct pmem2_map *map_open(const char *file, size_t size)
{
	const mode_t FILE_MODE = 0644;
	struct pmem2_source *source;
	struct pmem2_config *config;
	struct pmem2_map *map = NULL;

	int fd = open(file, O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
	if (fd < 0) {
		UT_FATAL("File creation error (errno: %d)!", errno);
		return NULL;
	}

	if (size > 0) {
		if (ftruncate(fd, (off_t)size) != 0)
			goto err_fd;
	}

	if (pmem2_source_from_fd(&source, fd) != 0)
		goto err_fd;

	if (pmem2_config_new(&config) != 0)
		goto err_config;

	pmem2_config_set_required_store_granularity(config, PMEM2_GRANULARITY_PAGE);

	if (pmem2_map_new(&map, config, source) != 0)
		goto err_map;

err_map:
	pmem2_config_delete(&config);
err_config:
	pmem2_source_delete(&source);
err_fd:
	close(fd);

	UT_ASSERTne(map, NULL);
	return map;
}

#ifdef __cplusplus
}
#endif

#endif /* LIBPMEMSTREAM_UNITTEST_H */
