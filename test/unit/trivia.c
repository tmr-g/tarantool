/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2010-2025, Tarantool AUTHORS, please see AUTHORS file.
 */

#define UNIT_TAP_COMPATIBLE 1
#include "unit.h"
#include "small/static.h"
#include "tzcode/tzcode.h"
#include "trivia/util.h"

static void
test_div_round_up(void)
{
	header();
	plan(6);

	ok(DIV_ROUND_UP(0, 1) == 0);
	ok(DIV_ROUND_UP(0, UINT64_MAX) == 0);
	ok(DIV_ROUND_UP(1, UINT64_MAX) == 1);
	ok(DIV_ROUND_UP(42, UINT64_MAX) == 1);
	ok(DIV_ROUND_UP(UINT64_MAX, 1) == UINT64_MAX);
	ok(DIV_ROUND_UP(UINT64_MAX, UINT64_MAX) == 1);

	check_plan();
	footer();
}

/* Simple type with synthetic errors for TOSTR() tests. */
struct point {
	int x, y;
};

static int
point_snprint(char *buf, int len, const struct point *x, bool with_type)
{
	if (x == NULL)
		return snprintf(buf, len, "<NULL>");
	char *type = with_type ? "point" : "";
	return snprintf(buf, len, "%s(%d, %d)", type, x->x, x->y);
}

static int
point_snprint_err_mem(char *buf, int len, const struct point *x)
{
	return SMALL_STATIC_SIZE;
}

/* You may see a real example in tuple test unit. */
static int
point_snprint_err_data(char *buf, int len, const struct point *x)
{
	return -1;
}

static void
test_tostr(void)
{
	header();
	plan(10);

	struct point a = {123456789, 987654321};

	/* maxlen > 0: cropping. */

	ok(strcmp(TOSTR(point_snprint, 3, NULL, false), "<NU") == 0);

	const int sufficient_maxlen = 64;
	const int poor_maxlen = 20;

	ok(strcmp(TOSTR(point_snprint, sufficient_maxlen, &a, true),
		  "point(123456789, 987654321)") == 0);
	ok(strcmp(TOSTR(point_snprint, poor_maxlen, &a, true),
		  "point(123456789, 987") == 0);

	/* Check for data error on printing. */
	ok(strcmp(TOSTR(point_snprint_err_data, sufficient_maxlen, &a),
		  TOSTR_ERRRES_TEXT(point_snprint_err_data)) == 0);
	ok(strcmp(TOSTR(point_snprint_err_data, poor_maxlen, &a),
		  TOSTR_ERRRES_TEXT(point_snprint_err_data)) == 0);

	/* maxlen == 0: calc len case. */

	ok(strcmp(TOSTR(point_snprint, 0, NULL, false), "<NULL>") == 0);

	ok(strcmp(TOSTR(point_snprint, 0, &a, false),
		  "(123456789, 987654321)") == 0);
	ok(strcmp(TOSTR(point_snprint, 0, &a, true),
		  "point(123456789, 987654321)") == 0);

	/* Check for data error on calc len. */
	ok(strcmp(TOSTR(point_snprint_err_data, 0, &a),
		  TOSTR_ERRRES_TEXT(point_snprint_err_data)) == 0);
	/* Check for no mem error: requested len is out of static buf size. */
	ok(strcmp(TOSTR(point_snprint_err_mem, 0, &a),
		  TOSTR_ERRRES_TEXT(point_snprint_err_mem)) == 0);

	check_plan();
	footer();
}

int
main(void)
{
	plan(2);

	test_div_round_up();
	test_tostr();

	return check_plan();
}
