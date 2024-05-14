#define WITH_LUIB_IMPL
#include "stdluib.h"
#undef WITH_LUIB_IMPL

int test_string(void) {
    String str;
    string_init(&str);

    ASSERT_OR(str.ptr != nullptr, "New strings allocate some space", return 1);
    ASSERT_OR(str.length == 0, "New strings must be empty", return 1);

    string_append_char(&str, 'h');
    string_append_char(&str, 'e');
    string_append_char(&str, 'l');
    string_append_char(&str, 'l');
    string_append_char(&str, 'o');
    bool eq_start = string_compare_str(&str, "hello");
    ASSERT_OR(eq_start, "String message must coincide", return 1);

    string_append(&str, ", world");
    bool eq_end = string_compare_str(&str, "hello, world");
    ASSERT_OR(eq_end, "String message must coincide", return 1);
    
    string_destroy(&str);
    ASSERT_OR(str.length == 0, "Destroyed string must have length 0", return 1);
    ASSERT_OR(str.ptr == nullptr, "Destroyed string must have invalid pointer", return 1);

    return 0;
}

int test_stringview(void) {
    char *str = "hello, world";

    StringView sv = stringview_create(str);

    ASSERT_OR(strncmp(sv.ptr, str, sv.length) == 0, "New stringview must match source", return 1);
    return 0;
}

int main(void) {
    test_string();
    test_stringview();
    return 0;
}
