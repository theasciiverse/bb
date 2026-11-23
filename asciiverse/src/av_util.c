/*
 * av_util.c: where BB finds its music, and its session log.
 *
 * BB keeps no saves or settings. Its log, one per run, says which display
 * and sound it found, which songs it loaded, and how it ended, so a
 * report of "no music" or "nothing on screen" can be answered.
 *
 * Copyright (C) 2026 Asciiverse. GPL-2.0-or-later, like BB.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define SEP '\\'
#else
#include <unistd.h>
#define SEP '/'
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include "av_util.h"

static char exe_dir[4096];
static FILE *logf;

const char *av_exe_dir(void)
{
    static int done;
    char path[4096];
    char *slash;
    int n = -1;
    if (done)
        return exe_dir;
    done = 1;
#if defined(_WIN32)
    n = (int) GetModuleFileNameA(NULL, path, sizeof(path) - 1);
#elif defined(__APPLE__)
    {
        uint32_t size = sizeof(path);
        char real[4096];
        if (_NSGetExecutablePath(path, &size) == 0) {
            if (realpath(path, real))
                strcpy(path, real);
            n = (int) strlen(path);
        }
    }
#else
    n = (int) readlink("/proc/self/exe", path, sizeof(path) - 1);
#endif
    if (n <= 0)
        return exe_dir;
    path[n] = 0;
    slash = strrchr(path, SEP);
#ifdef _WIN32
    if (!slash)
        slash = strrchr(path, '/');
#endif
    if (slash) {
        slash[1] = 0;
        strcpy(exe_dir, path);
    }
    return exe_dir;
}

static int make_dir(const char *path)
{
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

/* makes every folder along path */
static int make_dirs(char *path)
{
    char *p;
    for (p = path + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char c = *p;
            *p = 0;
            make_dir(path);
            *p = c;
        }
    }
    return make_dir(path);
}

void av_log_open(void)
{
    char dir[4096];
    char file[4200];
    const char *base;
    time_t now = time(NULL);
#ifdef _WIN32
    base = getenv("LOCALAPPDATA");
    if (!base || !*base)
        return;
    snprintf(dir, sizeof(dir), "%s\\bb", base);
#else
    base = getenv("XDG_STATE_HOME");
    if (base && *base) {
        snprintf(dir, sizeof(dir), "%s/bb", base);
    } else {
        const char *home = getenv("HOME");
        if (!home || !*home)
            return;
        snprintf(dir, sizeof(dir), "%s/.local/state/bb", home);
    }
#endif
    if (!make_dirs(dir))
        return;
    snprintf(file, sizeof(file), "%s%cbb.log", dir, SEP);
    logf = fopen(file, "w");
    if (logf)
        fprintf(logf, "bb 1.3rc1 (Asciiverse), %s", ctime(&now));
}

void av_log(const char *fmt, ...)
{
    va_list ap;
    if (!logf)
        return;
    va_start(ap, fmt);
    vfprintf(logf, fmt, ap);
    va_end(ap);
    fputc('\n', logf);
    fflush(logf);
}

void av_log_close(void)
{
    if (logf) {
        fclose(logf);
        logf = NULL;
    }
}
