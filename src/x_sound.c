#include <allegro5/allegro.h>
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include <stdio.h>
//#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "g_header.h"
#include "m_maths.h"

#include "x_sound.h"
#include "x_init.h"
#include "x_synth.h"
#include "x_music.h"

extern struct game_struct game;
extern struct view_struct view;

ALLEGRO_SAMPLE *sample [SAMPLES];
extern ALLEGRO_SAMPLE *msample [MSAMPLES];
extern ALLEGRO_EVENT_SOURCE sound_event_source;
extern ALLEGRO_EVENT_QUEUE *sound_queue;
extern ALLEGRO_EVENT sound_event;

float tone [TONES];

extern struct synthstruct synth;

struct sound_list_struct
{
	int exists;
	int sample_index;
	int pitch;
	int vol;
	int priority;
	int pan; // -100 to 100 (converted to float when played)
	al_fixed x, y;


};


// SOUND_LIST_LENGTH is the max number of gameplay sounds that can be played each tick
#define SOUND_LIST_LENGTH 16

struct sound_list_struct sound_list [SOUND_LIST_LENGTH];


void clear_sound_list(void)
{
 int i;

 for (i = 0; i < SOUND_LIST_LENGTH; i ++)
	{
		sound_list [i].exists = 0;
	}

}

// This is used for both menu input and in-game command input
void play_interface_sound(int s, int pitch)
{


 if (settings.sound_on == 0
  || game.fast_forward != FAST_FORWARD_OFF)
   return;

// al_play_sample(sample [s], 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

//bool al_play_sample(ALLEGRO_SAMPLE *spl, float gain, float pan, float speed,
//   ALLEGRO_PLAYMODE loop, ALLEGRO_SAMPLE_ID *ret_id)

 sound_event.user.data1 = s;
 sound_event.user.data2 = pitch;
 sound_event.user.data3 = 0; // pan (-1 to 1)
 sound_event.user.data4 = 40; // vol (0 to 100)

 al_emit_user_event(&sound_event_source, &sound_event, NULL);

}

void play_game_sound(int s, int pitch, int vol, int priority, al_fixed x, al_fixed y)
{

 if (settings.sound_on == 0
  || game.fast_forward != FAST_FORWARD_OFF
  || abs(x - view.camera_x) > (view.centre_x_zoomed + al_itofix(100))
  || abs(y - view.camera_y) > (view.centre_y_zoomed + al_itofix(100))
		|| (game.vision_mask
			&& w.vision_area[game.user_player_index]
			                [fixed_to_block(x)]
									          [fixed_to_block(y)].vision_time < w.world_time - VISION_AREA_VISIBLE_TIME))
   return;

/*
  || x < view.camera_x - view.centre_x_zoomed - al_itofix(100)
  || x > view.camera_x + view.centre_x_zoomed + al_itofix(100)
  || y < view.camera_y - view.centre_y_zoomed - al_itofix(100)
  || y > view.camera_y + view.centre_y_zoomed + al_itofix(100))
   return;*/

 int i;


 for (i = 0; i < SOUND_LIST_LENGTH; i ++)
	{
		if (sound_list[i].exists
			&&	sound_list[i].sample_index == s)
			return;
	}

 for (i = 0; i < SOUND_LIST_LENGTH; i ++)
	{
		if (sound_list[i].exists == 0)
				break;
	}

	if (i == SOUND_LIST_LENGTH)
	{
  for (i = 0; i < SOUND_LIST_LENGTH; i ++)
	 {
			 if (sound_list[i].priority < priority)
 				break;
 	}

 	if (i == SOUND_LIST_LENGTH)
	 	return; // sound list already full of higher priority sounds
	}


	sound_list [i].exists = 1;
	sound_list [i].sample_index = s;
	sound_list [i].pitch = pitch;
	if (view.centre_x_zoomed == 0)
	 sound_list [i].pan = 0; // pan (-100 to 100)
	  else
  	 sound_list [i].pan = (al_fixtoi(x - view.camera_x) * 100) / al_fixtoi(view.centre_x_zoomed); // pan (-100 to 100)
	sound_list [i].vol = vol;
	sound_list [i].priority = priority;

/*
 sound_event.user.data1 = s;
 sound_event.user.data2 = pitch;
 sound_event.user.data3 = (al_fixtoi(x - view.camera_x) * 100) / al_fixtoi(view.centre_x_zoomed); // pan (-100 to 100)
 sound_event.user.data4 = vol; // volume (0-100)

 al_emit_user_event(&sound_event_source, &sound_event, NULL);
*/

}

void play_sound_list(void)
{
	int i;

	for (i = 0; i < SOUND_LIST_LENGTH; i ++)
	{

		if (sound_list [i].exists)
		{

   sound_event.user.data1 = sound_list [i].sample_index;
   sound_event.user.data2 = sound_list [i].pitch;
   sound_event.user.data3 = sound_list [i].pan;
   sound_event.user.data4 = sound_list [i].vol; // volume (0-100)

   al_emit_user_event(&sound_event_source, &sound_event, NULL);

			sound_list [i].exists = 0;
		}
	}


}

/*
void	play_view_sound(void)
{

	game.play_sound = 0;

	if (game.fast_forward != 0)
		return;

	if (game.play_sound_note < 0
		|| game.play_sound_note >= TONES)
		return;

	game.play_sound_counter = 5;

 sound_event.user.data1 = SAMPLE_BLIP1;
 sound_event.user.data2 = game.play_sound_note;
 sound_event.user.data3 = 0; // pan (-1 to 1)
 sound_event.user.data4 = game.play_sound_vol; // volume (-1 to 1)

 al_emit_user_event(&sound_event_source, &sound_event, NULL);

}*/


struct sound_configstruct sound_config;


void *thread_check_sound_queue(ALLEGRO_THREAD *thread, void *arg)
{

// return NULL;

 ALLEGRO_EVENT received_sound_event;
 int event_received;

 int camstate_paused = 0;

// sthread_init_mustate(0);

// return NULL;

 while(TRUE)
 {

  event_received = al_wait_for_event_timed(sound_queue, &received_sound_event, 0.1);

// check for a signal that the user has exited:
  if (al_get_thread_should_stop(thread))
   return NULL;

  if (event_received == 0)
   continue;


// If data1 is -1, this is a message to reset camstate
// data 2 should be the area index
// data 3 should be the region index
// data 4 is the random seed to use
   if (received_sound_event.user.data1 == -1
				|| received_sound_event.user.data1 == -2)
			{
				init_camstate(received_sound_event.user.data1, received_sound_event.user.data2, received_sound_event.user.data3, received_sound_event.user.data4);
				continue;
			}

			if (received_sound_event.user.data1 == -3)
			{
				camstate_paused = 1;
				continue;
			}

			if (received_sound_event.user.data1 == -4)
			{
				camstate_paused = 0;
				continue;
			}

  if (received_sound_event.type == ALLEGRO_EVENT_TIMER)
  {
  	if (!camstate_paused)
    sthread_run_camstate();
//   if (sthread_play_current_piece() == 0)
//    sthread_init_mustate(0);
   continue;
  }



   float pan = (float) received_sound_event.user.data3 / 100;

   if (pan < -1)
    pan = -1;
   if (pan > 1)
    pan = 1;

   al_play_sample(sample [received_sound_event.user.data1], sound_config.effect_volume * (float) received_sound_event.user.data4 * 0.01, pan, tone [received_sound_event.user.data2], ALLEGRO_PLAYMODE_ONCE, NULL);

 };

}




// call this (from anywhere in the main game thread) when it's time to restart the music
void reset_music(int area_index, int region_index, unsigned int rand_seed)
{

   if (settings.option [OPTION_VOL_MUSIC] == 0)
				return;

   sound_event.user.data1 = -1; // tells code in x_music to reset camstate
   sound_event.user.data2 = area_index;
   sound_event.user.data3 = region_index;
   sound_event.user.data4 = rand_seed;

   al_emit_user_event(&sound_event_source, &sound_event, NULL);

}

// call this (from anywhere in the main game thread) to turn off the music
void turn_music_off(void)
{

   sound_event.user.data1 = -2; // tells code in x_music to set camstate.active to 0
//   sound_event.user.data2 = rand_seed;

   al_emit_user_event(&sound_event_source, &sound_event, NULL);

}


// call this (from anywhere in the main game thread) to turn off the music
void pause_music(void)
{

   sound_event.user.data1 = -3; // tells code in x_music to pause music

   al_emit_user_event(&sound_event_source, &sound_event, NULL);

}

// call this (from anywhere in the main game thread) to turn off the music
void unpause_music(void)
{

   sound_event.user.data1 = -4; // tells code in x_music to unpause music

   al_emit_user_event(&sound_event_source, &sound_event, NULL);

}


