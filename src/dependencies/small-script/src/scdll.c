/*  Small compiler — Windows DLL wrapper.
 *
 *  Implements the sc_* I/O hook surface declared in sc.h with memory-backed
 *  storage so the embedded compiler can run without touching the disk for
 *  the main translation unit, the assembler intermediate, or the .amx
 *  output. #include files resolve through an optional host callback
 *  (opts->include); if the host doesn't supply one, or the callback
 *  doesn't recognise the name, the lookup falls through to stdio with
 *  the -i<path> search list.
 *
 *  This file replaces the stdio-backed defaults that live in sc1.c behind
 *  #if !defined NO_MAIN, so the DLL build must define NO_MAIN.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#ifndef SC_DLL_EXPORTS
#define SC_DLL_EXPORTS
#endif
#include "sc.h"
#include "scdll.h"

/* ------------------------------------------------------------------------- *
 *  Per-compile context (single-instance — the underlying compiler is not
 *  reentrant either, so guarding against concurrent calls is the host's
 *  responsibility).
 * ------------------------------------------------------------------------- */

#define MEM_SENTINEL_DEFAULT  "memory.sma"

static const char *g_src_buf       = NULL;
static int         g_src_len       = 0;
static const char *g_src_sentinel  = MEM_SENTINEL_DEFAULT;

static char       *g_bin_buf       = NULL;
static int         g_bin_len       = 0;
static int         g_bin_alive     = 0;   /* set when sc_closebin keeps the buffer */

static sc_diag_fn  g_diag          = NULL;
static void       *g_diag_user     = NULL;

static sc_include_fn g_include      = NULL;
static void         *g_include_user = NULL;

/* ------------------------------------------------------------------------- *
 *  Handle types. Every handle returned to the compiler is a pointer to a
 *  struct whose first field tags its kind, so the dispatch functions below
 *  can route generically.
 * ------------------------------------------------------------------------- */

typedef enum {
    H_FILE = 1,    /* on-disk file (used for #include lookup)              */
    H_SRC  = 2,    /* in-memory source buffer (read-only, line-by-line)    */
    H_RW   = 3     /* in-memory growable buffer (asm intermediate, binary) */
} handle_kind;

typedef struct {
    handle_kind kind;
} handle_hdr;

typedef struct {
    handle_hdr hdr;
    FILE      *fp;
} file_handle;

typedef struct {
    handle_hdr  hdr;
    const char *buf;
    size_t      size;
    size_t      pos;
} src_handle;

typedef struct {
    handle_hdr hdr;
    char      *buf;
    size_t     size;     /* bytes written so far */
    size_t     cap;      /* allocated capacity   */
    size_t     pos;      /* read/write cursor    */
} rw_handle;

/* ------------------------------------------------------------------------- *
 *  Memory helpers
 * ------------------------------------------------------------------------- */

static int rw_grow(rw_handle *h, size_t need)
{
    size_t cap = h->cap ? h->cap : 4096;
    while (cap < need) cap *= 2;
    if (cap == h->cap) return 1;
    char *nb = (char *)realloc(h->buf, cap);
    if (!nb) return 0;
    h->buf = nb;
    h->cap = cap;
    return 1;
}

static rw_handle *rw_new(void)
{
    rw_handle *h = (rw_handle *)calloc(1, sizeof(*h));
    if (!h) return NULL;
    h->hdr.kind = H_RW;
    return h;
}

static void diag_emit(const char *msg)
{
    if (g_diag) g_diag(g_diag_user, msg);
}

/* ------------------------------------------------------------------------- *
 *  sc_*  I/O hook implementations (replace the stdio versions in sc1.c)
 * ------------------------------------------------------------------------- */

int sc_printf(const char *message, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, message);
    int n = vsnprintf(buf, sizeof buf, message, ap);
    va_end(ap);
    if (n > 0) diag_emit(buf);
    return n;
}

int sc_error(int number, char *message, char *filename,
             int firstline, int lastline, va_list argptr)
{
    static const char *prefix[3] = { "error", "fatal error", "warning" };
    char head[512];
    char body[1024];
    char joined[2048];

    head[0] = '\0';
    if (number != 0) {
        const char *pre = prefix[number / 100];
        if (firstline >= 0)
            snprintf(head, sizeof head, "%s(%d -- %d) : %s %03d: ",
                     filename ? filename : "?", firstline, lastline, pre, number);
        else
            snprintf(head, sizeof head, "%s(%d) : %s %03d: ",
                     filename ? filename : "?", lastline, pre, number);
    }
    vsnprintf(body, sizeof body, message, argptr);
    snprintf(joined, sizeof joined, "%s%s", head, body);
    diag_emit(joined);
    return 0;
}

/* ---- source ---- */

void *sc_opensrc(char *filename)
{
    /* Main translation unit: fed from memory if the filename matches the
     * sentinel set up by sc_compile_buffer. */
    if (g_src_buf && filename && g_src_sentinel
        && strcmp(filename, g_src_sentinel) == 0)
    {
        src_handle *h = (src_handle *)calloc(1, sizeof(*h));
        if (!h) return NULL;
        h->hdr.kind = H_SRC;
        h->buf      = g_src_buf;
        h->size     = (size_t)g_src_len;
        h->pos      = 0;
        return h;
    }

    /* #include lookup: give the host's include callback first crack.
     * The callback returns a host-owned buffer; we wrap it in an
     * H_SRC handle so the existing read/eof/reset path works without
     * a new handle kind. The buffer must outlive sc_compile_buffer. */
    if (g_include && filename) {
        const char *buf  = NULL;
        int         len  = 0;
        if (g_include(g_include_user, filename, &buf, &len) == 0 && buf) {
            if (len < 0) len = (int)strlen(buf);
            src_handle *h = (src_handle *)calloc(1, sizeof(*h));
            if (!h) return NULL;
            h->hdr.kind = H_SRC;
            h->buf      = buf;
            h->size     = (size_t)len;
            h->pos      = 0;
            return h;
        }
    }

    FILE *fp = fopen(filename, "rt");
    if (!fp) return NULL;
    file_handle *h = (file_handle *)calloc(1, sizeof(*h));
    if (!h) { fclose(fp); return NULL; }
    h->hdr.kind = H_FILE;
    h->fp       = fp;
    return h;
}

void sc_closesrc(void *handle)
{
    if (!handle) return;
    handle_hdr *hdr = (handle_hdr *)handle;
    if (hdr->kind == H_FILE) {
        file_handle *fh = (file_handle *)handle;
        if (fh->fp) fclose(fh->fp);
    }
    free(handle);
}

void sc_resetsrc(void *handle, void *position)
{
    assert(handle != NULL);
    handle_hdr *hdr = (handle_hdr *)handle;
    if (hdr->kind == H_FILE) {
        file_handle *fh = (file_handle *)handle;
        fsetpos(fh->fp, (fpos_t *)position);
    } else if (hdr->kind == H_SRC) {
        src_handle *sh = (src_handle *)handle;
        sh->pos = *(size_t *)position;
    }
}

char *sc_readsrc(void *handle, char *target, int maxchars)
{
    handle_hdr *hdr = (handle_hdr *)handle;
    if (hdr->kind == H_FILE) {
        return fgets(target, maxchars, ((file_handle *)handle)->fp);
    }
    /* H_SRC: emit one line (terminator included, like fgets). */
    src_handle *sh = (src_handle *)handle;
    if (sh->pos >= sh->size) return NULL;
    int n = 0;
    while (n < maxchars - 1 && sh->pos < sh->size) {
        char c = sh->buf[sh->pos++];
        target[n++] = c;
        if (c == '\n') break;
    }
    target[n] = '\0';
    return target;
}

void *sc_getpossrc(void *handle)
{
    handle_hdr *hdr = (handle_hdr *)handle;
    if (hdr->kind == H_FILE) {
        static fpos_t lastpos;
        fgetpos(((file_handle *)handle)->fp, &lastpos);
        return &lastpos;
    }
    /* H_SRC: a static slot is fine — only one mark is live at a time
     * (matches the stdio implementation in sc1.c). */
    static size_t last_src_pos;
    last_src_pos = ((src_handle *)handle)->pos;
    return &last_src_pos;
}

int sc_eofsrc(void *handle)
{
    handle_hdr *hdr = (handle_hdr *)handle;
    if (hdr->kind == H_FILE) return feof(((file_handle *)handle)->fp);
    src_handle *sh = (src_handle *)handle;
    return sh->pos >= sh->size;
}

/* ---- assembler intermediate (always in-memory, read+write) ---- */

void *sc_openasm(char *filename) { (void)filename; return rw_new(); }

void sc_closeasm(void *handle, int deletefile)
{
    (void)deletefile;
    if (!handle) return;
    rw_handle *h = (rw_handle *)handle;
    free(h->buf);
    free(h);
}

void sc_resetasm(void *handle)
{
    rw_handle *h = (rw_handle *)handle;
    h->pos = 0;
}

int sc_writeasm(void *handle, char *st)
{
    rw_handle *h = (rw_handle *)handle;
    size_t n = strlen(st);
    if (!rw_grow(h, h->pos + n + 1)) return 0;
    memcpy(h->buf + h->pos, st, n);
    h->pos += n;
    if (h->pos > h->size) h->size = h->pos;
    return 1;
}

char *sc_readasm(void *handle, char *target, int maxchars)
{
    rw_handle *h = (rw_handle *)handle;
    if (h->pos >= h->size) return NULL;
    int n = 0;
    while (n < maxchars - 1 && h->pos < h->size) {
        char c = h->buf[h->pos++];
        target[n++] = c;
        if (c == '\n') break;
    }
    target[n] = '\0';
    return target;
}

/* ---- binary output (always in-memory, write-only with seek+ftell) ---- */

void *sc_openbin(char *filename) { (void)filename; return rw_new(); }

void sc_closebin(void *handle, int deletefile)
{
    if (!handle) return;
    rw_handle *h = (rw_handle *)handle;
    if (deletefile || h->size == 0) {
        free(h->buf);
    } else {
        /* Hand ownership of the buffer over to the result struct. */
        g_bin_buf   = h->buf;
        g_bin_len   = (int)h->size;
        g_bin_alive = 1;
    }
    free(h);
}

void sc_resetbin(void *handle)
{
    rw_handle *h = (rw_handle *)handle;
    h->pos = 0;
}

int sc_writebin(void *handle, void *buffer, int size)
{
    rw_handle *h = (rw_handle *)handle;
    if (!rw_grow(h, h->pos + (size_t)size)) return 0;
    memcpy(h->buf + h->pos, buffer, (size_t)size);
    h->pos += (size_t)size;
    if (h->pos > h->size) h->size = h->pos;
    return 1;
}

long sc_lengthbin(void *handle)
{
    rw_handle *h = (rw_handle *)handle;
    return (long)h->pos;
}

/* ------------------------------------------------------------------------- *
 *  Public API
 * ------------------------------------------------------------------------- */

#define MAX_ARGV 64

SC_API int sc_compile_buffer(const sc_compile_options *opts,
                             sc_compile_result        *out)
{
    if (!opts || !out) return -1;

    out->errnum  = 0;
    out->amx     = NULL;
    out->amx_len = 0;

    if (!opts->source) return -1;

    int src_len = opts->source_len;
    if (src_len < 0) src_len = (int)strlen(opts->source);

    g_src_buf      = opts->source;
    g_src_len      = src_len;
    g_src_sentinel = opts->source_name ? opts->source_name : MEM_SENTINEL_DEFAULT;
    g_bin_buf      = NULL;
    g_bin_len      = 0;
    g_bin_alive    = 0;
    g_diag         = opts->diag;
    g_diag_user    = opts->diag_user;
    g_include      = opts->include;
    g_include_user = opts->include_user;

    /* Build argv: argv[0] is the (fake) program path, argv[1] is the
     * sentinel filename, then optional -i<path>, then any caller flags. */
    char *argv[MAX_ARGV];
    int   argc = 0;
    char  inc_arg[1024];

    argv[argc++] = (char *)"sc.dll";
    argv[argc++] = (char *)g_src_sentinel;

    if (opts->include_path && *opts->include_path) {
        snprintf(inc_arg, sizeof inc_arg, "-i%s", opts->include_path);
        argv[argc++] = inc_arg;
    }

    int extra = opts->extra_argc;
    if (extra < 0) extra = 0;
    if (argc + extra >= MAX_ARGV) extra = MAX_ARGV - argc - 1;
    for (int i = 0; i < extra; ++i) argv[argc++] = (char *)opts->extra_argv[i];
    argv[argc] = NULL;

    int retcode = sc_compile(argc, argv);

    out->errnum = errnum;   /* global from scvars.c */
    /* Match the sc CLI: sc1.c:449 deletes the binary only when errnum != 0,
     * so warnings (retcode == 1) still produce a valid .amx. g_bin_alive
     * already reflects that decision — trust it and hand the buffer over. */
    if (g_bin_alive) {
        out->amx     = g_bin_buf;
        out->amx_len = g_bin_len;
    } else if (g_bin_buf) {
        free(g_bin_buf);
    }

    g_src_buf      = NULL;
    g_src_len      = 0;
    g_src_sentinel = MEM_SENTINEL_DEFAULT;
    g_bin_buf      = NULL;
    g_bin_len      = 0;
    g_bin_alive    = 0;
    g_diag         = NULL;
    g_diag_user    = NULL;
    g_include      = NULL;
    g_include_user = NULL;

    return retcode;
}

SC_API void sc_free(void *p)
{
    free(p);
}

/* ------------------------------------------------------------------------- *
 *  DllMain
 * ------------------------------------------------------------------------- */

#if defined(_WIN32)
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    (void)hinst; (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hinst);
    return TRUE;
}
#endif
