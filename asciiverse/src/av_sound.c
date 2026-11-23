/*
 * av_sound.c: a MikMod output driver that plays through miniaudio.
 *
 * BB plays its three Scream Tracker modules with MikMod. MikMod's own
 * drivers talk to one sound system each (ALSA, PulseAudio, OSS, Core
 * Audio, DirectSound) and are linked against it. This one hands MikMod's
 * software mixer to miniaudio instead, which finds the system's sound
 * library when BB starts (ALSA, PulseAudio or JACK on Linux, Core Audio on
 * macOS, WASAPI on Windows), so one binary plays everywhere, and plays on
 * silently where there is none.
 *
 * miniaudio pulls the samples from its own thread, the way MikMod's SDL
 * driver works, so the music never waits for the picture: BB's frames can
 * take as long as they like.
 *
 * AV_AUDIO_DUMP=<file> plays to no device and writes the stream to the
 * file instead (raw 16-bit stereo at 44100 Hz), to check the music
 * without listening.
 *
 * Copyright (C) 2026 Asciiverse. GPL-2.0-or-later, like BB.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mikmod_internals.h"

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_ALSA
#define MA_ENABLE_PULSEAUDIO
#define MA_ENABLE_JACK
#define MA_ENABLE_COREAUDIO
#define MA_ENABLE_WASAPI
#define MA_ENABLE_DSOUND
#define MA_ENABLE_WINMM
#define MA_ENABLE_NULL
#include "miniaudio.h"

#include "av_sound.h"

static ma_device device;
static ma_context null_context;
static int device_ready, null_ready;
static FILE *dump;
static volatile int playing;

/*
 * Takes MikMod's vars lock only if it is free. The audio thread must never
 * wait for it: MikMod holds it while stopping the output (Player_Free of
 * the playing song when BB changes songs, MikMod_Exit when it quits), and
 * stopping the output waits for this thread to leave the callback. A
 * waiting callback would leave both waiting for good, which froze BB on
 * the change to its second song. When MikMod is busy, the callback plays
 * one period (20 ms) of silence instead.
 */
static int av_trylock_vars(void)
{
#if defined(HAVE_PTHREAD)
    return pthread_mutex_trylock(&_mm_mutex_vars) == 0;
#elif defined(_WIN32) || defined(__CYGWIN__)
    return !_mm_mutex_vars || WaitForSingleObject(_mm_mutex_vars, 0) == WAIT_OBJECT_0;
#else
    return 1;
#endif
}

static void av_callback(ma_device *dev, void *out, const void *in, ma_uint32 frames)
{
    ULONG bytes = frames * ma_get_bytes_per_frame(dev->playback.format, dev->playback.channels);
    (void) in;
    if (!playing || !av_trylock_vars()) {
        memset(out, 0, bytes);
        if (dump)
            fwrite(out, 1, bytes, dump);
        return;
    }
    {
        ULONG got = VC_WriteBytes((SBYTE *) out, bytes);
        if (got < bytes)
            VC_SilenceBytes((SBYTE *) out + got, bytes - got);
    }
    MUTEX_UNLOCK(vars);
    if (dump)
        fwrite(out, 1, bytes, dump);
}

static BOOL av_IsPresent(void)
{
    return 1;
}

static int av_Init(void)
{
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
    md_mode &= ~DMODE_FLOAT;
    config.playback.format = (md_mode & DMODE_16BITS) ? ma_format_s16 : ma_format_u8;
    config.playback.channels = (md_mode & DMODE_STEREO) ? 2 : 1;
    config.sampleRate = md_mixfreq;
    config.periodSizeInMilliseconds = 20;
    config.dataCallback = av_callback;
    if (getenv("AV_AUDIO_DUMP")) {
        ma_backend null_backend = ma_backend_null;
        dump = fopen(getenv("AV_AUDIO_DUMP"), "wb");
        if (!dump || ma_context_init(&null_backend, 1, NULL, &null_context) != MA_SUCCESS) {
            _mm_errno = MMERR_OPENING_AUDIO;
            return 1;
        }
        null_ready = 1;
    }
    if (ma_device_init(null_ready ? &null_context : NULL, &config, &device) != MA_SUCCESS) {
        _mm_errno = MMERR_OPENING_AUDIO;
        return 1;
    }
    device_ready = 1;
    if (VC_Init())
        return 1;
    return 0;
}

static void av_Exit(void)
{
    playing = 0;
    if (device_ready) {
        ma_device_uninit(&device);
        device_ready = 0;
    }
    if (null_ready) {
        ma_context_uninit(&null_context);
        null_ready = 0;
    }
    if (dump) {
        fclose(dump);
        dump = NULL;
    }
    VC_Exit();
}

static int av_Reset(void)
{
    av_Exit();
    return av_Init();
}

static int av_PlayStart(void)
{
    if (VC_PlayStart())
        return 1;
    playing = 1;
    if (ma_device_start(&device) != MA_SUCCESS) {
        playing = 0;
        _mm_errno = MMERR_OPENING_AUDIO;
        return 1;
    }
    return 0;
}

static void av_PlayStop(void)
{
    playing = 0;
    if (device_ready)
        ma_device_stop(&device);
    VC_PlayStop();
}

static void av_Update(void)
{
    /* miniaudio's thread pulls the samples; nothing to push */
}

const char *av_sound_backend(void)
{
    return device_ready ? ma_get_backend_name(device.pContext->backend) : "none";
}

MIKMODAPI struct MDRIVER drv_av = {
    NULL,
    "miniaudio",
    "miniaudio output (Asciiverse)",
    0, 255,
    "miniaudio",
    NULL,
    NULL,
    av_IsPresent,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,
    av_Init,
    av_Exit,
    av_Reset,
    VC_SetNumVoices,
    av_PlayStart,
    av_PlayStop,
    av_Update,
    NULL,
    VC_VoiceSetVolume,
    VC_VoiceGetVolume,
    VC_VoiceSetFrequency,
    VC_VoiceGetFrequency,
    VC_VoiceSetPanning,
    VC_VoiceGetPanning,
    VC_VoicePlay,
    VC_VoiceStop,
    VC_VoiceStopped,
    VC_VoiceGetPosition,
    VC_VoiceRealVolume
};
