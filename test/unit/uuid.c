#include "core/random.h"
#include "mp_uuid.h"
#include "msgpuck/msgpuck.h"
#include "mp_extension_types.h"
#include "tt_uuid.h"
#include <string.h>

/* For temporary bench. */
// #include <time.h>
// #include "small/static.h"
// #include "trivia/util.h"

#define UNIT_TAP_COMPATIBLE 1
#include "unit.h"

static void
uuid_test(struct tt_uuid a, struct tt_uuid b, int expected_result)
{
        char a_str[UUID_STR_LEN + 1];
        char b_str[UUID_STR_LEN + 1];

        tt_uuid_to_string(&a, a_str);
        tt_uuid_to_string(&b, b_str);

        int cmp_result = tt_uuid_compare(&a, &b);

        char *sign = 0;

        if (cmp_result == 1)
                sign = ">";
        else if (cmp_result == -1)
                sign = "<";
        else
                sign = "=";

        is(cmp_result,
           expected_result,
           "%s %s %s", a_str, sign, b_str);
}

static void
mp_uuid_test()
{
        plan(4);
        char buf[18];
        char *data = buf;
        const char *data1 = buf;
        struct tt_uuid uu, ret;
        random_init();
        tt_uuid_create(&uu);
        char *end = mp_encode_uuid(data, &uu);
        is(end - data, mp_sizeof_uuid(), "mp_sizeof_uuid() == encoded length");
        struct tt_uuid *rc = mp_decode_uuid(&data1, &ret);
        is(rc, &ret, "mp_decode_uuid() return code");
        is(data1, end, "mp_sizeof_uuid() == decoded length");
        is(tt_uuid_compare(&uu, &ret), 0, "mp_decode_uuid(mp_encode_uuid(uu)) == uu");
        check_plan();
}

static int
mp_fprint_ext_test(FILE *file, const char **data, int depth)
{
        (void)depth;
        int8_t type;
        uint32_t len = mp_decode_extl(data, &type);
        if (type != MP_UUID)
                return fprintf(file, "undefined");
        return mp_fprint_uuid(file, data, len);
}

static int
mp_snprint_ext_test(char *buf, int size, const char **data, int depth)
{
        (void)depth;
        int8_t type;
        uint32_t len = mp_decode_extl(data, &type);
        if (type != MP_UUID)
                return snprintf(buf, size, "undefined");
        return mp_snprint_uuid(buf, size, data, len);
}

static void
mp_print_test(void)
{
        plan(5);
        header();

        mp_snprint_ext = mp_snprint_ext_test;
        mp_fprint_ext = mp_fprint_ext_test;

        char buffer[1024];
        char str[1024];
        struct tt_uuid uuid;
        tt_uuid_create(&uuid);

        mp_encode_uuid(buffer, &uuid);
        int rc = mp_snprint(NULL, 0, buffer);
        is(rc, UUID_STR_LEN, "correct mp_snprint size with empty buffer");
        rc = mp_snprint(str, sizeof(str), buffer);
        is(rc, UUID_STR_LEN, "correct mp_snprint size");
        is(strcmp(str, tt_uuid_str(&uuid)), 0, "correct mp_snprint result");

        FILE *f = tmpfile();
        rc = mp_fprint(f, buffer);
        is(rc, UUID_STR_LEN, "correct mp_fprint size");
        rewind(f);
        rc = fread(str, 1, sizeof(str), f);
        str[rc] = 0;
        is(strcmp(str, tt_uuid_str(&uuid)), 0, "correct mp_fprint result");
        fclose(f);

        mp_snprint_ext = mp_snprint_ext_default;
        mp_fprint_ext = mp_fprint_ext_default;

/* Temporary bench. */
/*
uint64_t diff(struct timespec *a, struct timespec *b)
{
	uint64_t nsa = (uint64_t)a->tv_sec * 1000000000 + (uint64_t)a->tv_nsec;
	uint64_t nsb = (uint64_t)b->tv_sec * 1000000000 + (uint64_t)b->tv_nsec;
	return nsa - nsb;
}

	struct timespec t0, t1;
	clock_gettime(CLOCK_REALTIME, &t0);
	for (int i = 0; i < 1000000; i++)
		TOSTR(tt_uuid_snprint, UUID_STR_LEN, &uuid);
	clock_gettime(CLOCK_REALTIME, &t1);
	note("T1=%ld", diff(&t1, &t0));

	clock_gettime(CLOCK_REALTIME, &t0);
	for (int i = 0; i < 1000000; i++)
		TOSTR(tt_uuid_snprint, 0, &uuid);
	clock_gettime(CLOCK_REALTIME, &t1);
	note("T2=%ld", diff(&t1, &t0));
*/
/* Temporary bench. END */

        footer();
        check_plan();
}

int
main(void)
{
        plan(4);

        uuid_test(
                (struct tt_uuid){.time_low = 1712399963,
                                .time_mid = 34898,
                                .time_hi_and_version = 18482,
                                .clock_seq_hi_and_reserved = 175,
                                .clock_seq_low = 139,
                                .node = {'A', 'd', '\325', ',', 'b', '\353'}},
                (struct tt_uuid){.time_low = 409910263,
                                .time_mid = 53143,
                                .time_hi_and_version = 20014,
                                .clock_seq_hi_and_reserved = 139,
                                .clock_seq_low = 27,
                                .node = {'v', '\025', 'O', 'o', '9', 'I'}},
                1);


        uuid_test(
                (struct tt_uuid){.time_low = 123421000,
                                .time_mid = 36784,
                                .time_hi_and_version = 11903,
                                .clock_seq_hi_and_reserved = 175,
                                .clock_seq_low = 80,
                                .node = {'A', 'd', '\325', ',', 'b', '\353'}},
                (struct tt_uuid){.time_low = 532451999,
                                .time_mid = 23976,
                                .time_hi_and_version = 10437,
                                .clock_seq_hi_and_reserved = 139,
                                .clock_seq_low = 54,
                                .node = {'v', '\025', 'O', 'o', '9', 'I'}},
                -1);

        mp_uuid_test();
        mp_print_test();

        return check_plan();
}
