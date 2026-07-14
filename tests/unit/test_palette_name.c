// First unit suite for the project. Regression coverage for the palette-name storage
// contract sprouted out of setPaletteName() (see tests/docs/handover-unit-tests.md).
// Built under ASan/UBSan by `make test`, so the historical malloc(strlen)+strcpy
// off-by-one (issue #76 family) would fail here directly.
#include "greatest.h"
#include "palette_name.h"
#include <string.h>
#include <stdlib.h>

#define SLOTS 16

TEST dup_copies_string_nul_terminated(void) {
    char *names[SLOTS] = {0};
    const char *in = "sunset.pal";

    char *out = palette_name_dup(names, 3, in);

    ASSERT(out != NULL);
    ASSERT_EQ(out, names[3]);            // stored into the requested slot
    ASSERT_STR_EQ(in, out);              // content matches
    ASSERT_EQ('\0', out[strlen(in)]);    // NUL present => strlen+1 bytes allocated
    ASSERT(out != in);                   // it is a copy, not the input pointer

    free(names[3]);
    PASS();
}

TEST dup_replaces_and_frees_previous(void) {
    char *names[SLOTS] = {0};

    palette_name_dup(names, 0, "first");     // ASan tracks this allocation
    char *second = palette_name_dup(names, 0, "second");  // must free "first"

    ASSERT_STR_EQ("second", names[0]);
    ASSERT_EQ(second, names[0]);

    free(names[0]);
    PASS();
}

TEST dup_null_clears_and_frees_slot(void) {
    char *names[SLOTS] = {0};

    palette_name_dup(names, 5, "temp");
    char *r = palette_name_dup(names, 5, NULL);  // frees + clears

    ASSERT_EQ(NULL, r);
    ASSERT_EQ(NULL, names[5]);
    PASS();
}

TEST dup_into_empty_slot_is_safe(void) {
    char *names[SLOTS] = {0};   // slot starts NULL: must not free garbage

    char *out = palette_name_dup(names, 7, "x");

    ASSERT(out != NULL);
    ASSERT_STR_EQ("x", names[7]);
    free(names[7]);
    PASS();
}

SUITE(palette_name_suite) {
    RUN_TEST(dup_copies_string_nul_terminated);
    RUN_TEST(dup_replaces_and_frees_previous);
    RUN_TEST(dup_null_clears_and_frees_slot);
    RUN_TEST(dup_into_empty_slot_is_safe);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(palette_name_suite);
    GREATEST_MAIN_END();
}
