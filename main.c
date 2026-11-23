/*
 * BB: The portable demo
 *
 * (C) 1997 by AA-group (e-mail: aa@horac.ta.jcu.cz)
 *
 * 3rd August 1997
 * version: 1.2 [final3]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public Licences as by published
 * by the Free Software Foundation; either version 2; or (at your option)
 * any later version
 *
 * This program is distributed in the hope that it will entertaining,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILTY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Publis License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include "timers.h"
#include "bb.h"
#ifdef HAVE_LIBMIKMOD
#include <mikmod.h>
#include "av_sound.h"
#include "av_util.h"
MODULE *module;
int bbsound;
void stop();
tl_timer *update_timer;

static void
update_sound (void *data)
{
  if (Player_Active ())
    MikMod_Update ();
  else if (module)
    Player_SetPosition (0);
}
#endif

int
load_song (char *name)
{
#ifdef HAVE_LIBMIKMOD
  finish_stuff = 0;
  if (bbsound)
    {
      /* the songs are beside the program (where the Asciiverse
         package puts them), else in SOUNDDIR where a system
         install put them, else in the current folder */
      const char *dir = av_exe_dir ();
      char *name2 = malloc (strlen (name) + strlen (dir) + 2);
      sprintf (name2, "%s%s", dir, name);
      stop ();
      if (module != NULL)
	Player_Free (module);
      module = Player_Load (name2, 64, 0);
#ifdef SOUNDDIR
      if (!module)
	{
	  free (name2);
	  name2 = malloc (strlen (name) + strlen (SOUNDDIR) + 2);
	  sprintf (name2, "%s/%s", SOUNDDIR, name);
	  module = Player_Load (name2, 64, 0);
	}
#endif
      if (!module)
        module = Player_Load (name, 64, 0);
      free (name2);
      if (module)
	av_log ("song %s", name);
      else
	av_log ("song %s: %s", name, MikMod_strerror (MikMod_errno));
      if (module == NULL)
	{
	  aa_printf (context, 0, 0, AA_SPECIAL,
		     "Failed to load module:%s",
		     MikMod_strerror (MikMod_errno));
	  aa_flush (context);
	  sleep (1);
	}
    }
  return 0;
#endif
}

void
play ()
{
#ifdef HAVE_LIBMIKMOD
  if (module != NULL)
    {
      Player_Start (module);
      update_sound (NULL);
      if (!update_timer)
	{
	  update_timer = tl_create_timer ();
	  tl_set_handler (update_timer, update_sound, NULL);
	  tl_set_interval (update_timer, 10000);
	  tl_add_timer (syncgroup, update_timer);
	  tl_reset_timer (update_timer);
	}
    }
#endif
}

void
stop ()
{
#ifdef HAVE_LIBMIKMOD
  if (bbsound)
    {
      if (module)
        Player_Free (module);
      module = NULL;
      if (update_timer)
	{
	  tl_free_timer (update_timer);
	  update_timer = NULL;
	}
    }
#endif
}
int
main (int argc, char *argv[])
{
  int p = 0;
  int retval;
  int i, j;
  int nosound = 0;

  /* -nosound (or --nosound) plays the demo without its music */
  for (i = j = 1; i < argc; i++)
    {
      if (!strcmp (argv[i], "-nosound") || !strcmp (argv[i], "--nosound"))
	nosound = 1;
      else
	argv[j++] = argv[i];
    }
  argc = j;
  argv[argc] = NULL;

  av_log_open ();
  for (i = 1; i < argc; i++)
    av_log ("argument %s", argv[i]);
  bbinit (argc, argv);
  av_log ("display: AAlib %s driver, %ix%i", context->driver->shortname,
	  aa_scrwidth (context), aa_scrheight (context));
#ifdef HAVE_LIBMIKMOD
  if (!nosound)
    {
      /* the music starts with the demo, through miniaudio
         (asciiverse/src/av_sound.c), at CD rate with interpolation */
      MikMod_RegisterDriver (&drv_av);
      MikMod_RegisterLoader (&load_s3m);
      md_mixfreq = 44100;
      md_mode |= DMODE_16BITS | DMODE_STEREO | DMODE_INTERP;
      bbsound = 1;
      if (MikMod_Init (""))
	{
	  aa_printf (context, 0, p++, AA_SPECIAL,
		     "Sound initialization failed:%s",
		     MikMod_strerror (MikMod_errno));
	  aa_flush (context);
	  bbsound = 0;
	  sleep (1);
	}
      av_log ("sound: %s", bbsound ? av_sound_backend ()
	      : MikMod_strerror (MikMod_errno));
    }
  else
    av_log ("sound: off (-nosound)");
#endif
  aa_resize(context);
  retval = bb ();
#ifdef HAVE_LIBMIKMOD
  if (bbsound)
    {
      MikMod_Exit ();
    }
#endif
  av_log ("ended");
  av_log_close ();
  return retval;
}
