/*
 * Copyright 2010-2016, Tarantool AUTHORS, please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const char *
fmt_va(const char *fmt, va_list args)
{
	size_t buf_size = 1024*1024;
	static char *buf;
	static size_t buf_used;
	if (buf == NULL)
	{
		buf = (char *) malloc(buf_size);
		assert(buf != NULL);
		buf_used = 0;
	}

	va_list args_copy;
	va_copy(args_copy, args);

	int res = vsnprintf(NULL, 0, fmt, args);
	assert(res >= 0);
	size_t size = (size_t)res + 1;

	/* Recycle as in static alloc. */
	assert(size <= buf_size);
	if (buf_used + size > buf_size)
		buf_used = 0;
	char *str = buf + buf_used;
	buf_used += size;

	res = vsnprintf(str, size, fmt, args_copy);
	assert((res >= 0) && (size_t)res == size - 1);
	va_end(args_copy);
	return str;
}

const char *
fmt(const char *fmt, ...)
{
	va_list args;
	const char *ret;
	va_start(args, fmt);
	ret = fmt_va(fmt, args);
	va_end(args);
	return ret;
}
