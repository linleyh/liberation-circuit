#include <allegro5/allegro.h>
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"

#include "g_misc.h"

#include "x_sound.h"
#include "x_init.h"
#include "x_synth.h"
#include "x_music.h"

extern struct sound_configstruct sound_config; // in x_sound.c


void load_in_sample(int s, const char* file_name);
void load_in_msample(int s, const char* file_name);
void build_tone_array(void);

extern struct game_struct game;

extern ALLEGRO_SAMPLE *sample [SAMPLES];
extern ALLEGRO_SAMPLE *msample [MSAMPLES];

ALLEGRO_EVENT_SOURCE sound_event_source;
ALLEGRO_EVENT_QUEUE *sound_queue;
ALLEGRO_EVENT sound_event;

ALLEGRO_TIMER* sound_timer;

ALLEGRO_THREAD *sound_thread;
int started_sound_thread = 0;

extern float tone [TONES];

//float temp_buffer [SYNTH_SAMPLE_MAX_SIZE];

void init_sound(int camstate_rand_seed)
{

 settings.sound_on = 1;

//    al_init_acodec_addon(); - shouldn't need this


 if (!al_install_audio()
  || !al_init_acodec_addon())
 {
  fprintf(stdout, "\nAllegro audio installation failed. Starting without sound.");
  settings.sound_on = 0;
  return;
 }


 if (!al_reserve_samples(24))
 {
  fprintf(stdout, "\nCould not set up Allegro audio voice/mixer. Starting without sound.");
  settings.sound_on = 0;
  return;
 }

 if (settings.option [OPTION_VOL_MUSIC] == 0
		&& settings.option [OPTION_VOL_EFFECT] == 0)
	{
  settings.sound_on = 0;
  return;
	}


 load_in_sample(SAMPLE_BLIP1, "data/sound/blip.wav");
 load_in_sample(SAMPLE_BLIP2, "data/sound/blip2.wav");
 load_in_sample(SAMPLE_BLIP3, "data/sound/blip3.wav");
 load_in_sample(SAMPLE_BLIP4, "data/sound/blip4.wav");
 load_in_sample(SAMPLE_KILL, "data/sound/kill.wav");
 load_in_sample(SAMPLE_CHIRP, "data/sound/chirp.wav");
 load_in_sample(SAMPLE_OVER, "data/sound/over.wav");
 load_in_sample(SAMPLE_ALLOC, "data/sound/alloc.wav");
 load_in_sample(SAMPLE_NEW, "data/sound/new.wav");

 load_in_sample(SAMPLE_BANG, "data/sound/bang.wav");
 load_in_sample(SAMPLE_BANG2, "data/sound/bang2.wav");
 load_in_sample(SAMPLE_INT_UP, "data/sound/int_up.wav");
 load_in_sample(SAMPLE_INT_BREAK, "data/sound/int_break.wav");
 load_in_sample(SAMPLE_RESTORE, "data/sound/restore.wav");
 load_in_sample(SAMPLE_STREAM1, "data/sound/stream1.wav");
 load_in_sample(SAMPLE_STREAM2, "data/sound/stream2.wav");
 load_in_sample(SAMPLE_ZAP, "data/sound/zap.wav");
 load_in_sample(SAMPLE_SPIKE, "data/sound/spike.wav");
 load_in_sample(SAMPLE_SLICE, "data/sound/slice.wav");
 load_in_sample(SAMPLE_ULTRA, "data/sound/ultra.wav");
 load_in_sample(SAMPLE_BUBBLE, "data/sound/bubble.wav");

 load_in_sample(SAMPLE_DRUM1, "data/sound/music/drum1.wav");
 load_in_sample(SAMPLE_DRUM2, "data/sound/music/drum2.wav");
 load_in_sample(SAMPLE_DRUM3, "data/sound/music/drum3.wav");
 load_in_sample(SAMPLE_CLICK, "data/sound/music/click.wav");
 load_in_sample(SAMPLE_THUMP, "data/sound/music/thump.wav");

// load_in_msample(MSAMPLE_NOTE, "data/sound/amb/note.wav");
// load_in_msample(MSAMPLE_NOTE2, "data/sound/amb/note_harm.wav");
// load_in_msample(MSAMPLE_NOTE3, "data/sound/amb/note_sine.wav");
// load_in_msample(MSAMPLE_NOTE4, "data/sound/amb/note_sq.wav");

// load_in_amb_sample(AMB_WARBLE, "sound/amb/warble.wav");
// load_in_amb_sample(AMB_NOTE, "sound/amb/note.wav");



 //al_stop_samples();

 build_tone_array();

 sound_config.music_volume = settings.option [OPTION_VOL_MUSIC] * 0.01;
 sound_config.effect_volume = settings.option [OPTION_VOL_EFFECT] * 0.01;

 sound_timer = al_create_timer(0.22); // 0.20
 if (!sound_timer)
 {
    fprintf(stdout, "\nError: failed to create sound timer.");
    safe_exit(-1);
 }
 al_start_timer(sound_timer);
 al_init_user_event_source(&sound_event_source);

 sound_queue = al_create_event_queue();
 al_register_event_source(sound_queue, &sound_event_source);
 al_register_event_source(sound_queue, al_get_timer_event_source(sound_timer));

 sound_event.user.type = ALLEGRO_GET_EVENT_TYPE(1, 0, 4, 0);

 sthread_init_sample_pointers();

 if (settings.option [OPTION_VOL_MUSIC] != 0)
  init_camstate(-1, 1, 0, camstate_rand_seed); // this is usually only called from within the sound thread
   // but can be called here because the sound thread hasn't been started yet.
   // -1 means start new music
    else
     init_camstate(-2, 0, 0, 0); // -2 means turn the music off


 sound_thread = al_create_thread(thread_check_sound_queue, NULL);
 al_start_thread(sound_thread);

 started_sound_thread = 1;

}

void stop_sound_thread(void)
{

 if (started_sound_thread)
 {

//  al_join_thread(sound_thread, NULL);
  al_destroy_thread(sound_thread);

 }

}

void load_in_sample(int s, const char* file_name)
{

 if (settings.sound_on == 0)
  return;

 sample [s] = al_load_sample(file_name);

 if (!sample [s])
 {
  fprintf(stdout, "\nCould not load sample file (%s). Sound disabled.", file_name);
  settings.sound_on = 0;
 }

}


void load_in_msample(int s, const char* file_name)
{

 if (settings.sound_on == 0)
  return;

 msample [s] = al_load_sample(file_name);

 if (!msample [s])
 {
  fprintf(stdout, "\nCould not load sample file (%s). Sound disabled.", file_name);
  settings.sound_on = 0;
 }

}




void build_tone_array(void)
{

 int i;

 for (i = 0; i < TONES; i ++)
 {
  tone [i] = 0.5 * pow(1.05946309, i + 1);
 }


}



