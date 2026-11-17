/* Aster's platform interface. No parsing, typing, IR or optimization lives here.
 * Array control blocks are stable: growing a buffer never invalidates aliases.
 * The allocation list gives the batch compiler a simple process-lifetime arena. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
typedef struct Mem {
    void *p;
    struct Mem *next;
} Mem;
typedef struct {
    unsigned char *data;
    int64_t len;
} Str;
typedef struct {
    unsigned char *data;
    int64_t len, cap, stride;
} Arr;
static Mem *arena;
static Arr *arguments;
static void cleanup(void) {
    while (arena) {
        Mem *n = arena->next;
        free(arena->p);
        free(arena);
        arena = n;
    }
}
static void *alloc(size_t n) {
    void *p = calloc(1, n ? n : 1);
    if (!p) {
        fputs("aster: out of memory\n", stderr);
        exit(70);
    }
    Mem *m = malloc(sizeof *m);
    m->p = p;
    m->next = arena;
    arena = m;
    return p;
}
static void trap(const char *s) {
    fprintf(stderr, "aster: %s\n", s);
    exit(70);
}
static Str *string(const void *p, int64_t n) {
    Str *s = alloc(sizeof *s);
    s->data = alloc((size_t)n + 1);
    memcpy(s->data, p, (size_t)n);
    s->len = n;
    return s;
}
int64_t rt_len(void *p) {
    return ((Str *)p)->len;
}
Arr *rt_array(int64_t stride) {
    Arr *a = alloc(sizeof *a);
    a->stride = stride;
    return a;
}
void *rt_at(Arr *a, int64_t i) {
    if (i < 0 || i >= a->len)
        trap("array index out of bounds");
    return a->data + i * a->stride;
}
void rt_push(Arr *a, void *value) {
    if (a->len == a->cap) {
        a->cap = a->cap ? a->cap * 2 : 8;
        unsigned char *p = alloc((size_t)(a->cap * a->stride));
        if (a->len)
            memcpy(p, a->data, (size_t)(a->len * a->stride));
        a->data = p;
    }
    memcpy(a->data + a->len * a->stride, value, (size_t)a->stride);
    a->len++;
}
void rt_pop(Arr *a) {
    if (!a->len)
        trap("pop from empty array");
    a->len--;
}
int64_t rt_byte(Str *s, int64_t i) {
    if (i < 0 || i >= s->len)
        trap("string index out of bounds");
    return s->data[i];
}
Str *rt_slice(Str *s, int64_t start, int64_t n) {
    if (start < 0 || n < 0 || start > s->len || n > s->len - start)
        trap("invalid string slice");
    return string(s->data + start, n);
}
Str *rt_concat(Str *a, Str *b) {
    Str *s = alloc(sizeof *s);
    s->len = a->len + b->len;
    s->data = alloc((size_t)s->len + 1);
    memcpy(s->data, a->data, (size_t)a->len);
    memcpy(s->data + a->len, b->data, (size_t)b->len);
    return s;
}
int64_t rt_equal(Str *a, Str *b) {
    return a->len == b->len && !memcmp(a->data, b->data, (size_t)a->len);
}
Str *rt_chr(int64_t n) {
    unsigned char c = (unsigned char)n;
    return string(&c, 1);
}
Str *rt_itoa(int64_t n) {
    char b[32];
    int z = snprintf(b, sizeof b, "%ld", (long)n);
    return string(b, z);
}
Str *rt_env(Str *key) {
    char *s = getenv((char *)key->data);
    return string(s ? s : "", s ? (int64_t)strlen(s) : 0);
}
int64_t rt_parse_int(Str *s) {
    int base =
        s->len > 2 && s->data[0] == '0' && (s->data[1] == 'x' || s->data[1] == 'X') ? 16 : 10;
    uint64_t u = strtoull((char *)s->data, NULL, base);
    int64_t n;
    memcpy(&n, &u, 8);
    return n;
}
void rt_print(Str *s) {
    fwrite(s->data, 1, (size_t)s->len, stdout);
}
void rt_eprint(Str *s) {
    fwrite(s->data, 1, (size_t)s->len, stderr);
}
void rt_println(Str *s) {
    rt_print(s);
    putchar('\n');
}
void rt_print_int(int64_t n) {
    printf("%ld", (long)n);
}
void rt_println_int(int64_t n) {
    printf("%ld\n", (long)n);
}
void rt_exit(int64_t n) {
    exit((int)n);
}
Str *rt_read_file(Str *path) {
    FILE *f = fopen((char *)path->data, "rb");
    if (!f) {
        fprintf(stderr, "aster: cannot read %s: %s\n", path->data, strerror(errno));
        exit(66);
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    unsigned char *b = alloc((size_t)n + 1);
    if (fread(b, 1, (size_t)n, f) != (size_t)n)
        trap("file read failed");
    fclose(f);
    Str *s = alloc(sizeof *s);
    s->data = b;
    s->len = n;
    return s;
}
int64_t rt_write_file(Str *path, Str *s) {
    FILE *f = fopen((char *)path->data, "wb");
    if (!f) {
        fprintf(stderr, "aster: cannot write %s\n", path->data);
        exit(73);
    }
    size_t n = fwrite(s->data, 1, (size_t)s->len, f);
    fclose(f);
    return n == (size_t)s->len ? 0 : 1;
}
Arr *rt_args(void) {
    return arguments;
}
int64_t rt_run(Arr *a) {
    char **v = alloc((size_t)(a->len + 1) * sizeof *v);
    for (int64_t i = 0; i < a->len; i++)
        v[i] = (char *)(*(Str **)rt_at(a, i))->data;
    fflush(NULL);
    pid_t p = fork();
    if (!p) {
        execvp(v[0], v);
        perror(v[0]);
        _exit(127);
    }
    if (p < 0)
        trap("fork failed");
    int s = 0;
    while (waitpid(p, &s, 0) < 0) {
        if (errno != EINTR)
            trap("waitpid failed");
    }
    return WIFEXITED(s) ? WEXITSTATUS(s) : 128 + WTERMSIG(s);
}
int64_t rt_clock_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * INT64_C(1000000000) + t.tv_nsec;
}
int64_t rt_div(int64_t a, int64_t b) {
    if (!b)
        trap("division by zero");
    if (a == INT64_MIN && b == -1)
        return a;
    return a / b;
}
int64_t rt_rem(int64_t a, int64_t b) {
    if (!b)
        trap("division by zero");
    if (a == INT64_MIN && b == -1)
        return 0;
    return a % b;
}
/* Byte-buffer primitive; higher-level builders and maps are in Aster. */
Str *rt_join(Arr *a, Str *sep) {
    int64_t n = 0;
    for (int64_t i = 0; i < a->len; i++)
        n += (*(Str **)rt_at(a, i))->len + (i ? sep->len : 0);
    Str *s = alloc(sizeof *s);
    s->data = alloc((size_t)n + 1);
    s->len = n;
    int64_t off = 0;
    for (int64_t i = 0; i < a->len; i++) {
        if (i) {
            memcpy(s->data + off, sep->data, (size_t)sep->len);
            off += sep->len;
        }
        Str *v = *(Str **)rt_at(a, i);
        memcpy(s->data + off, v->data, (size_t)v->len);
        off += v->len;
    }
    return s;
}
extern int64_t aster_entry(void);
#ifndef ASTER_RUNTIME_NO_MAIN
int main(int argc, char **argv) {
    atexit(cleanup);
    arguments = rt_array(sizeof(Str *));
    for (int i = 0; i < argc; i++) {
        Str *s = string(argv[i], (int64_t)strlen(argv[i]));
        rt_push(arguments, &s);
    }
    return (int)aster_entry();
}
#endif
