#include <unistd.h>
#include <inttypes.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

/* Data Types (Core) */
#define usize size_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define nullptr NULL

#define bool int
#define true 1
#define false 0

#define ASSERT_OR(cond, msg, abort) do { if (!(cond)) { printf("%s:%d %s\n", __FILE__, __LINE__, msg); abort; } } while(0)
#define ASSERT(cond, msg) ASSERT_OR(cond, msg, exit(1))
#define TODO() ASSERT(false, "TODO")

#define loop while(1)

#define debugf(msg, ...) printf("DEBUG: " msg "\n", __VA_ARGS__)
#define debug(msg) printf("DEBUG: " msg "\n")

typedef struct {
    char *ptr;
    usize length;
    usize capacity;
} String;

typedef struct {
    const char *ptr;
    usize length;
} StringView;

StringView stringview_create(const char *);
StringView stringview_create_with_length(const char *, usize);
void stringview_print(StringView *sv);
void stringview_debug_print(StringView *sv);
char stringview_char_at(StringView *sv, usize pos);
void stringview_split(StringView *sv, char, StringView *lhs, StringView *rhs);
void stringview_split_str(StringView *, char *, StringView *lhs, StringView *rhs);

#define SV_SPLIT(sv, separator, lhs_name, rhs_name) StringView lhs_name = {0};\
    StringView rhs_name = {0};\
    stringview_split(sv, separator, &lhs_name, &rhs_name)

#define SV_SPLIT_STR(sv, separator, lhs_name, rhs_name) StringView lhs_name = {0};\
    StringView rhs_name = {0};\
    stringview_split_str(sv, separator, &lhs_name, &rhs_name)

String stringview_to_string(const StringView *sv);
void stringview_triml(StringView *sv, usize);
void stringview_trimr(StringView *sv, usize);
void string_print(String *sv);

bool string_compare_str(String *, const char*);

int string_init(String *);
void string_destroy(String *);
int string_append_char(String *, char);
int string_append(String *, const char*);
int string_append_sv(String *, const StringView*);

StringView string_make_view(const String *);

#ifdef WITH_LUIB_IMPL
#include <string.h>
#include <stdio.h>

#define min(a, b) (a <= b ? a : b)
#define max(a, b) (a >= b ? a : b)

StringView stringview_create(const char *str) {
    usize len = strlen(str);

    StringView sv = { .ptr = str, .length = len };

    return sv;
}

StringView stringview_create_with_length(const char *str, usize len) {
    StringView sv = { .ptr = str, .length = len };

    return sv;
}

void stringview_print(StringView *sv) {
    for(usize i = 0; i < sv->length; i++) {
        putc(sv->ptr[i], stdout);
    }
}

void stringview_debug_print(StringView *sv) {
    for(usize i = 0; i < sv->length; i++) {
        char c = sv->ptr[i];
        switch (c) {
        case '\n':
            printf("\\n");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\t':
            printf("\\t");
            break;
        default:
            putc(c, stdout);
            break;
        }
    }
}

char stringview_char_at(StringView *sv, usize pos) {
    if (pos > sv->length) { return '\0'; }

    return sv->ptr[pos];
}

void stringview_split(StringView *sv, char c, StringView *lhs, StringView *rhs) {
    usize found_position = 0;
    
    while(found_position < sv->length && stringview_char_at(sv, found_position) != c) {
        found_position++;
    }

    lhs->ptr = sv->ptr;
    lhs->length = found_position;

    if(found_position + 1 > sv->length) {
        rhs->ptr = sv->ptr + sv->length;
        rhs->length = 0;
    } else {
        rhs->ptr = sv->ptr + found_position + 1;
        rhs->length = sv->length - found_position - 1;
    }
}

void stringview_split_str(StringView *sv, char *separator, StringView *lhs, StringView *rhs) {
    usize sep_len = strlen(separator);
    usize found_position = 0;
    bool did_find = false;

    for (; found_position < (sv->length - sep_len); found_position++) {
        if(strncmp(separator, sv->ptr+found_position, sep_len) == 0) {
            did_find = true;
            break;
        }
    }

    if(did_find) {
        lhs->ptr = sv->ptr;
        lhs->length = found_position;

        rhs->ptr = sv->ptr + found_position + sep_len;
        rhs->length = sv->length - found_position - sep_len;
    } else {
        lhs->ptr = sv->ptr;
        lhs->length = sv->length;

        rhs->ptr = sv->ptr + sv->length;
        rhs->length = 0;
    }
}

void stringview_triml(StringView *sv, usize count) {
    usize chars_removed = count;
    if (sv->length < count) {
        chars_removed = count;
    }

    sv->ptr = sv->ptr + chars_removed;
    sv->length = sv->length - chars_removed;
}

void stringview_trimr(StringView *sv, usize count) {
    usize chars_removed = count;
    if (sv->length < count) {
        chars_removed = count;
    }

    sv->length = sv->length - chars_removed;
}

String stringview_to_string(const StringView *sv) {
    String st;
    string_init(&st);

    string_append_sv(&st, sv);

    return st;
}

void string_print(String *sv) {
    printf("%s", sv->ptr);
}

bool string_compare_str(String *sb, const char* str) {
    return strncmp(sb->ptr, str, sb->length) == 0;
}

int string_init(String *sb) {
    usize default_capacity = 8;
    sb->capacity = default_capacity;
    sb->length = 0;
    sb->ptr = calloc(default_capacity, sizeof(char));

    if (sb->ptr == NULL) {
        fprintf(stderr, "Could not allocate the string");
        return -1;
    }

    return 0;
}

void string_destroy(String *sb) {
    free(sb->ptr);
    sb->capacity = sb->length = 0;
    sb->ptr = nullptr;
}

int string_grow_to_length(String *sb, usize new_length) {
    if (new_length < sb->capacity) return 0;

    usize new_capacity = sb->capacity;
    do {
        new_capacity = new_capacity * 2;
    } while(new_capacity <= new_length);

    char *new_ptr = calloc(new_capacity, sizeof(char));
    memcpy(new_ptr, sb->ptr, sb->length);

    if (new_ptr == nullptr) {
        return -1;
    }

    sb->capacity = new_capacity;
    sb->ptr = new_ptr;

    return 0;
}

int string_append_char(String *sb, char c) {
    usize new_length = sb->length + 1;

    usize gr = string_grow_to_length(sb, new_length);
    if (gr < 0) {
        return gr;
    }

    *(sb->ptr + sb->length) = c;

    sb->length = new_length;

    return 0;
}

int string_append(String *sb, const char* str) {
    usize added_len = strlen(str);
    usize new_length = sb->length + added_len;
    usize gr = string_grow_to_length(sb, new_length);
    if (gr < 0) {
        return gr;
    }

    char *append_start = sb->ptr + sb->length;

    memcpy(append_start, str, added_len);

    sb->length = new_length;

    return 0;
}

int string_append_sv(String *st, const StringView *sv) {
    debugf("Sv length = %zu", sv->length);
    if (sv->length == 0) { return 0; }

    usize added_len = sv->length;
    usize new_length = st->length + added_len;

    usize gr = string_grow_to_length(st, new_length);
    if (gr < 0) {
        return gr;
    }

    char *append_start = st->ptr + st->length;

    memcpy(append_start, sv->ptr, added_len);

    st->length = new_length;

    return 0;
}

StringView string_make_view(const String *sb) {
    StringView sv = {.length = sb->length, .ptr = sb->ptr };

    return sv;
}
#endif