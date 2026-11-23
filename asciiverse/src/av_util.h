/* av_util.c: where BB finds its music, and its session log. */
#ifndef AV_UTIL_H
#define AV_UTIL_H
/* the folder BB's program file is in, with a trailing separator, or "" */
const char *av_exe_dir(void);
/* opens bb.log in the state folder ($XDG_STATE_HOME/bb, else
   ~/.local/state/bb; %LOCALAPPDATA%\bb on Windows), new for each run */
void av_log_open(void);
void av_log(const char *fmt, ...);
void av_log_close(void);
#endif
