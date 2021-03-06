/*
 * error.c - error handlers
 *
 * Copyright 2010 Rui Ueyama <rui314@gmail.com>.  All rights reserved.
 * This code is available under the simplified BSD license.  See LICENSE for details.
 */

#include "8cc.h"
#include <execinfo.h>

Exception *current_handler;

Exception *make_exception(void) {
    Exception *e = malloc(sizeof(Exception));
    e->msg = NULL;
    return e;
}

static void print(char *pre, char *format, va_list ap) {
    fprintf(stderr, "%s", pre);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}

void debug(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

static NORETURN void verror(char *format, va_list ap) {
    if (current_handler) {
        Exception *e = current_handler;
        current_handler = NULL;
        e->msg = to_string("ERROR: ");
        string_vprintf(e->msg, format, ap);
        longjmp(e->jmpbuf, 1);
    }
    print("ERROR: ", format, ap);
    exit(-1);
}

static void vwarn(char *format, va_list ap) {
    print("WARN: ", format, ap);
}

NORETURN void error(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    verror(format, ap);
    va_end(ap);
}

void warn(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vwarn(format, ap);
    va_end(ap);
}

NORETURN void print_parse_error(int line, int column, char *msg, va_list ap) {
    String *b = make_string_printf("Line %d:%d: ", line, column);
    string_append(b, msg);
    verror(STRING_BODY(b), ap);
}

static void print_stack_trace_int(bool safe) {
    void *buf[20];
    int size = backtrace(buf, sizeof(buf));
    fprintf(stderr, "Stack trace:\n");
    fflush(stderr);

    if (safe)
        backtrace_symbols_fd(buf, size, STDERR_FILENO);
    else {
        char **strs = backtrace_symbols(buf, size);
        for (int i = 0; i < size; i++)
            fprintf(stderr, "  %s\n", strs[i]);
        free(strs);
    }
}

void print_stack_trace(void) {
    print_stack_trace_int(false);
}

/*
 * print_stack_trace() that don't call malloc().
 */
void print_stack_trace_safe(void) {
    print_stack_trace_int(true);
}
