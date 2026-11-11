/*  Small compiler — Windows DLL public interface.
 *
 *  Embeds the Small 2.3.1 compiler in a host application: feed a script
 *  buffer in, get a compiled .amx blob out. Diagnostics are routed through
 *  an optional callback so the host can capture them.
 *
 *  The compiler is built around heavy global state, so only one compile
 *  may run at a time per loaded DLL.
 */
#ifndef SCDLL_H_INCLUDED
#define SCDLL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  if defined(SC_DLL_EXPORTS)
#    define SC_API __declspec(dllexport)
#  else
#    define SC_API __declspec(dllimport)
#  endif
#else
#  define SC_API
#endif

typedef void (*sc_diag_fn)(void *user, const char *line);

/* Called when the compiler tries to resolve a `#include <name>` (or
 * `#include "name"`) directive. The host returns a pointer to the
 * include's source bytes in *out_buf and its length in *out_len; the
 * pointed-to buffer must remain valid until sc_compile_buffer returns.
 * Pass -1 for *out_len if the buffer is NUL-terminated.
 *
 * Return 0 if the include was resolved, non-zero to let the compiler
 * fall through to the disk search (controlled by `include_path`). The
 * compiler may call this multiple times per include directive as it
 * tries different search-path variants of `name`; each call is
 * independent. */
typedef int (*sc_include_fn)(void       *user,
                             const char *name,
                             const char **out_buf,
                             int        *out_len);

typedef struct sc_compile_options {
    const char        *source;        /* main script bytes (need not be NUL-terminated) */
    int                source_len;    /* length in bytes; pass -1 if NUL-terminated      */
    const char        *source_name;   /* logical filename used in error messages;
                                         NULL defaults to "memory.sma"                   */
    const char        *include_path;  /* path searched for #include files; may be NULL    */
    int                extra_argc;    /* additional compiler flags (e.g. "-d2", "-O2")    */
    const char *const *extra_argv;
    sc_diag_fn         diag;          /* optional: receives one diagnostic line at a time */
    void              *diag_user;
    sc_include_fn      include;       /* optional: in-memory #include resolver. Called
                                         BEFORE the disk search; non-zero return falls
                                         through to disk. May be NULL. */
    void              *include_user;
} sc_compile_options;

typedef struct sc_compile_result {
    int   errnum;     /* 0 on success; otherwise the error count from the compiler */
    void *amx;        /* freshly malloc'd .amx blob; NULL on failure.
                         Caller must release with sc_free().                       */
    int   amx_len;    /* size of the .amx blob in bytes                            */
} sc_compile_result;

/* Compile a single in-memory translation unit.
 * Returns 0 on success, non-zero on error (matches sc_compile's retcode:
 *   0 = ok, 1 = warnings only, 2 = errors, 3 = fatal/abort).
 * *out is always populated; on failure amx is NULL and amx_len is 0. */
SC_API int  sc_compile_buffer(const sc_compile_options *opts,
                              sc_compile_result        *out);

/* Release a buffer returned in sc_compile_result::amx. */
SC_API void sc_free(void *p);

#ifdef __cplusplus
}
#endif

#endif  /* SCDLL_H_INCLUDED */
