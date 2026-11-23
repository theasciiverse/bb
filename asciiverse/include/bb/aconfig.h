/* BB's aconfig.h for the Asciiverse build, in place of configure's: the
   same answers configure gives on Linux, macOS and MinGW. Music is
   HAVE_LIBMIKMOD, played through asciiverse/src/av_sound.c. */
#define STDC_HEADERS 1
#define TIME_WITH_SYS_TIME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_STRDUP 1
#define HAVE_STRSTR 1
#define HAVE_STRTOL 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_UNISTD_H 1
#ifndef _WIN32
#define HAVE_SELECT 1
#define HAVE_SYS_IOCTL_H 1
#endif
#define RETSIGTYPE void
#define PACKAGE "bb"
#define VERSION "1.3rc1"
#define HAVE_LIBMIKMOD 1
