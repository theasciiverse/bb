/* av_sound.c: MikMod's output through miniaudio. */
#ifndef AV_SOUND_H
#define AV_SOUND_H
extern struct MDRIVER drv_av;
/* the sound system miniaudio found ("PulseAudio", "ALSA", ...) or "none" */
const char *av_sound_backend(void);
#endif
