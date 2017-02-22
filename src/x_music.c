/*

This file contains the music-playing functions.

Every function with a name starting with "sthread" should run in the separate music thread.

*/


#include <allegro5/allegro.h>
#include <math.h>

#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include <stdio.h>
#include <stdlib.h>
//#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"
#include "h_story.h"

//#include "g_misc.h"
//#include "g_header.h"

#include "x_sound.h"
#include "x_init.h"
#include "x_synth.h"
#include "x_music.h"


extern ALLEGRO_SAMPLE *sample [SAMPLES];
ALLEGRO_SAMPLE *msample [MSAMPLES];
extern ALLEGRO_EVENT_SOURCE sound_event_source;
extern ALLEGRO_EVENT_QUEUE *sound_queue;
extern ALLEGRO_EVENT sound_event;
extern ALLEGRO_TIMER* sound_timer;

extern struct sound_configstruct sound_config; // in x_sound.c

float tone [TONES];

extern struct synthstruct synth;



unsigned int sthread_rand(unsigned int rand_max);
//static void sthread_play_msample(int msample_index, int play_tone, int vol, int pan);
static void sthread_play_scale_sample(int instrument_flip, int instrument, int play_note, float vol, int pan, float freq);
static void sthread_new_instrument(int instrument, int scale_type, int instrument_type, int effect, int tone_offset);
static void sthread_finish_instrument(int flip, int instr, int note_index, float* buffer, int buffer_length);
static int sthread_random_instrument_type(void);
//static void sthread_reset_camstate_beat(float base_tempo, int bar_length, float stress_vol, float beat_extra_time);
static void sthread_place_agent_randomly(int agent_index);
static void sthread_remove_agent(void);
static void sthread_set_bar_type(void);

enum
{
NOTE_0,
NOTE_1,
NOTE_2,
NOTE_3,
NOTE_4,
NOTE_5,
NOTE_6,
NOTE_7,
NOTES
};




//#define CHOOSE_SCALE_INDEX 2

//#define CHOOSE_SCALE_INDEX sthread_rand(SCALE_TYPES)

/*
//Just temperament for a scale starting with C
// - unfortunately just temperament doesn't seem to work so well for this kind of music
#define RATIO_C 1
#define RATIO_CS (float) (16.0/15)
#define RATIO_D (float) (9.0/8)
#define RATIO_DS (float) (6.0/5)
#define RATIO_E (float) (5.0/4)
#define RATIO_F (float) (4.0/3)
#define RATIO_FS (float) (45.0/32)
#define RATIO_G (float) (3.0/2)
#define RATIO_GS (float) (8.0/5)
#define RATIO_A (float) (5.0/3)
#define RATIO_AS (float) (16.0/9)
#define RATIO_B (float) (15.0/8)
*/
/*
Equal temperament:
*/
#define RATIO_C 1
#define RATIO_CS 1.059463
#define RATIO_D 1.122462
#define RATIO_DS 1.189207
#define RATIO_E 1.259921
#define RATIO_F 1.334840
#define RATIO_FS 1.414214
#define RATIO_G 1.498307
#define RATIO_GS 1.587401
#define RATIO_A 1.681793
#define RATIO_AS 1.781797
#define RATIO_B 1.887749


enum
{
SCALE_C_E_G_A_AS,
SCALE_C_G_GS_AS_C_D,
SCALE_C_DS_F_FS_G_AS,
SCALE_CS_DS_FS_GS_AS,
SCALE_C_E_G,
SCALE_C_F_A,
SCALE_C_G,
SCALE_C_F_A_B,
SCALE_C_CS_E_FS,
SCALE_C_DS_FS_AS,

SCALE_TYPES

};

float scale_ratio [SCALE_TYPES] [SCALE_TONES] =
{

	{
		1, // C
		RATIO_E,
		RATIO_G,
		RATIO_A,
		RATIO_AS,
		2,
		RATIO_E * 2,
		RATIO_G * 2,
		RATIO_A * 2,


	}, // 0

	{
		1, // C
		RATIO_G,
		RATIO_GS,
		RATIO_AS,
		2,
		RATIO_D * 2,
		RATIO_DS * 2,
		RATIO_G * 2,
		RATIO_GS * 2,

	}, // 1

	{
		1, // C
		RATIO_DS,
		RATIO_F,
		RATIO_FS,
		RATIO_G,
		RATIO_AS,
		2,
		RATIO_E * 2,
		RATIO_G * 2,

	}, // 2

	{
		RATIO_CS,
		RATIO_DS,
		RATIO_FS,
		RATIO_GS,
		RATIO_AS,
		RATIO_CS * 2,
		RATIO_DS * 2,
		RATIO_FS * 2,
		RATIO_GS * 2,
	}, // 3

	{
		RATIO_C,
		RATIO_E,
		RATIO_G,
		RATIO_C * 2,
		RATIO_E * 2,
		RATIO_G * 2,
		RATIO_C * 4,
		RATIO_E * 4,
		RATIO_G * 4,
	}, // 4

	{
		RATIO_C,
		RATIO_F,
		RATIO_A,
		RATIO_C * 2,
		RATIO_F * 2,
		RATIO_A * 2,
		RATIO_C * 4,
		RATIO_F * 4,
		RATIO_A * 4,
	}, // 5


	{
		RATIO_C,
		RATIO_G,
		RATIO_C * 2,
		RATIO_G * 2,
		RATIO_C * 2,
		RATIO_C * 4,
		RATIO_G * 4,
		RATIO_C * 4,
		RATIO_C *2,
	}, // SCALE_C_G


	{
		RATIO_C,
		RATIO_F,
		RATIO_A,
		RATIO_B,
		RATIO_C * 2,
		RATIO_F * 2,
		RATIO_A * 2,
		RATIO_B * 2,
		RATIO_C * 4,
	}, // SCALE_C_F_A_B


	{
		RATIO_C,
		RATIO_CS,
		RATIO_E,
		RATIO_FS,
		RATIO_GS,
		RATIO_B,
		RATIO_C * 2,
		RATIO_CS * 2,
		RATIO_E * 2,
	}, // SCALE_C_CS_E_FS


	{
		RATIO_C,
		RATIO_DS,
		RATIO_FS,
		RATIO_AS,
		RATIO_C * 2,
		RATIO_DS * 2,
		RATIO_FS * 2,
		RATIO_AS * 2,
		RATIO_C * 4,
	}, // SCALE_C_DS_FS_AS


/*
	{
		1, // C
		RATIO_D,
		RATIO_E,
		RATIO_G,
		RATIO_A,
		2,
		RATIO_D * 2,
		RATIO_E * 2,
		RATIO_G * 2
	},*/
/*
	{
		1, // C
		RATIO_D,
		RATIO_F,
		RATIO_G,
		RATIO_AS,
		2,
		RATIO_D * 2,
		RATIO_F * 2,
		RATIO_AS * 2
	},
*/

};

/*

The following is a cellular-automata music generator.

It's based on Batuhan Bozkurt's Otomata (see http://www.earslap.com/page/otomata.html )



*/

enum
{
INSTRUMENT_TYPE_BASIC_SINE, // later code assumes this is 0
INSTRUMENT_TYPE_SINE_HARM1, // and this is 1
INSTRUMENT_TYPE_SINE_HARM2, // and 2
INSTRUMENT_TYPE_SINE_SQUARE,
INSTRUMENT_TYPE_SQUARE,
INSTRUMENT_TYPE_SQUARE_HARM1, // must come directly after SQUARE
INSTRUMENT_TYPE_SQUARE_HARM2, // must come directly after SQUARE_HARM1

//INSTRUMENT_TYPE_DRUM,

INSTRUMENT_TYPES

};

#define INSTRUMENT_TYPES_COMMON INSTRUMENT_TYPE_SQUARE
// instrument types with values less than INSTRUMENT_TYPES_COMMON will be more common

enum
{
INSTRUMENT_EFFECT_NONE,
INSTRUMENT_EFFECT_HIGH_PITCH,
INSTRUMENT_EFFECT_LOW_PITCH,
INSTRUMENT_EFFECT_SHORT,
INSTRUMENT_EFFECT_LONG,
INSTRUMENT_EFFECT_VIB,

INSTRUMENT_EFFECTS
};

#define CAM_W 9
#define CAM_H 9
#define CAM_AGENTS 20

enum
{
CADIR_UP,
CADIR_RIGHT,
CADIR_DOWN,
CADIR_LEFT,
CADIRS
};

#define INSTRUMENTS 3

struct cam_agent_struct
{
	int exists;
	int x, y;
	int direction;
	int instrument;
//	int move_x, move_y;
 timestamp collision_time;
// int msample_index;
// int scale_index;

 int echo_tone;
 int echo_strength;
 int echo_pan; // should be from -50 to 50
 int echo_instrument;
 int echo_flip; // 0 or 1 - based on instrument_flip for the agent's instrument at time when echo began

};


enum
{
DRUM_NONE,
DRUM_1,
DRUM_2,
DRUM_3,
DRUM_4,

DRUMS

};

//#define AREA_MUSIC_BAR_TYPES - only ever 1 of these (maybe could have one for each region?)
#define AREA_MUSIC_KEY_TYPES 3
#define AREA_MUSIC_INSTRUMENTS 6
#define AREA_MUSIC_EFFECTS 6
#define AREA_MUSIC_DRUMS 4

struct area_music_struct
{
	int bar_type;
	int region_seed [3];
	int key_type [AREA_MUSIC_KEY_TYPES];
	int instrument_type [AREA_MUSIC_INSTRUMENTS];
	int effect_type [AREA_MUSIC_EFFECTS];
	int drum_type [AREA_MUSIC_DRUMS];

};

const struct area_music_struct area_music [STORY_AREAS] =
{
	{
	 1, //int bar_type;
	 {85,
	  84,
	  0}, // int region_seed [3];
	 {
	  SCALE_C_E_G_A_AS,
	  SCALE_C_E_G,
	  SCALE_C_E_G,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM1,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {-1}, // int drum_type [AREA_MUSIC_DRUMS];

	}, // AREA_TUTORIAL

	{
	 2, //int bar_type;
	 {1, 32, 35}, // int region_seed [3];
	 {
	  SCALE_C_F_A_B,
	  SCALE_C_E_G,
	  SCALE_C_G,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM2,
	  INSTRUMENT_TYPE_SINE_HARM2,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_SHORT,
	  INSTRUMENT_EFFECT_LONG
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {0,0,1,1}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_BLUE


	{
	 3, //int bar_type;
	 {7,
	  8,
	  30}, // int region_seed [3];
	 {
	  SCALE_C_E_G_A_AS,
	  SCALE_C_G_GS_AS_C_D,
	  SCALE_C_DS_F_FS_G_AS,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SINE_SQUARE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SINE_SQUARE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_SHORT,
	  INSTRUMENT_EFFECT_LONG
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {2,2,3,3}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_GREEN


	{
	 3, //int bar_type;
	 {47, 46, 50}, // int region_seed [3];
	 {
	  SCALE_C_F_A,
	  SCALE_C_F_A_B,
	  SCALE_C_CS_E_FS,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM2,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SQUARE_HARM2,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_SHORT,
	  INSTRUMENT_EFFECT_LONG
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {3,3,4,4}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_YELLOW

	{
	 4, //int bar_type;
	 {63, 67, 69}, // int region_seed [3];

	 {
	  SCALE_C_DS_F_FS_G_AS,
	  SCALE_CS_DS_FS_GS_AS,
	  SCALE_C_DS_FS_AS,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_SINE_SQUARE,
	  INSTRUMENT_TYPE_SQUARE_HARM2,
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SINE_SQUARE,
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SQUARE_HARM1,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_LONG,
	  INSTRUMENT_EFFECT_VIB,
	  INSTRUMENT_EFFECT_VIB
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {0,1,2,3}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_ORANGE


	{
	 5, //int bar_type;
	 {51, 52, 57}, // int region_seed [3];
	 {
	  SCALE_C_CS_E_FS,
	  SCALE_C_G,
	  SCALE_C_DS_F_FS_G_AS,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SINE_SQUARE,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_LONG,
	  INSTRUMENT_EFFECT_SHORT,
	  INSTRUMENT_EFFECT_VIB
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {1,2,3,4}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_PURPLE

	{
	 6, //int bar_type;
	 {70, 2, 82}, // int region_seed [3];
	 {
	  SCALE_C_E_G_A_AS,
	  SCALE_C_G_GS_AS_C_D,
	  SCALE_CS_DS_FS_GS_AS,
	 }, // int key_type [AREA_MUSIC_KEY_TYPES];
	 {
	  INSTRUMENT_TYPE_BASIC_SINE,
	  INSTRUMENT_TYPE_SINE_HARM1,
	  INSTRUMENT_TYPE_SINE_HARM2,
	  INSTRUMENT_TYPE_SINE_SQUARE,
	  INSTRUMENT_TYPE_SQUARE,
	  INSTRUMENT_TYPE_SQUARE_HARM2,
	 }, // int instrument_type [AREA_MUSIC_INSTRUMENTS];
	 {
	  INSTRUMENT_EFFECT_NONE,
	  INSTRUMENT_EFFECT_LOW_PITCH,
	  INSTRUMENT_EFFECT_HIGH_PITCH,
	  INSTRUMENT_EFFECT_LONG,
	  INSTRUMENT_EFFECT_SHORT,
	  INSTRUMENT_EFFECT_VIB
	 }, // int effect_type [AREA_MUSIC_EFFECTS];
	 {0,1,2,4}, // int drum_type [AREA_MUSIC_DRUMS];
	}, // AREA_RED

};


struct camstate_struct
{
	int active; // is 0 if music currently off.

	int area_index;
//	int region_index; not currently used (region index is only relevant when camstate initialised)

	int scale_index; // shared by all instruments
	int note_offset; // shared by all instruments
	int counter;
	int tempo;
	int next_change; // should never be shorter than about 25 (to make sure that the instrument flipping doesn't mess up old echoes)
	timestamp total_time;
	int cam_grid [CAM_W] [CAM_H]; // holds index of agent, or -1 if empty
	struct cam_agent_struct agent [CAM_AGENTS];

	int max_agents; // set randomly at piece initialisation
	int active_agents; // starts lower than max_agents, then builds up

	int instrument_sample_flip [INSTRUMENTS];
	int instrument_type [INSTRUMENTS];
	int instrument_effect [INSTRUMENTS];
	int base_echo_strength [INSTRUMENTS];

 float base_tempo; // time between steps (in seconds). 0.22 seems to work well
 int beat_timer;
// int bar_length; // should probably be 3, 4 or maybe 5
// float stress_vol;
// float beat_extra_time;

	unsigned int rand_seed;
	unsigned int delayed_rand_seed;
	int time_until_random;

#define BAR_LENGTH_MAX 9

 int bar_length;
 float bar_beat_time [BAR_LENGTH_MAX];
 float bar_beat_vol [BAR_LENGTH_MAX];
 int bar_beat_drum [BAR_LENGTH_MAX];
 int drum_index [DRUMS];
 int bar_pos;

};
struct camstate_struct camstate;

ALLEGRO_SAMPLE* scale_sample [2] [INSTRUMENTS] [SCALE_TONES];


static void sthread_cam_note(int agent_index, int tone_index, float stress);
static void sthread_rotate_cam_agent(int agent_index);
static void sthread_change_camstate(void);
static void sthread_reset_drum_index(void);
static void sthread_change_drums(void);

static void sthread_create_samples_for_scale(int flip, int instrument, int scale_type, int instrument_type, int effect, int tone_offset);

// This function is usually only called from the sound thread. reset_music is called from outside (see x_sound.c).
//  However this function is called from the main thread during initialisation (before the sound thread has started)
// call with region_index == -1 to use rand_seed from the start. Otherwise, music is based on region for a couple of minutes before becoming random.
void init_camstate(int status, int area_index, int region_index, unsigned int rand_seed)
{
//fpr("\n st %i area %i r %i rs %i", status, area_index, region_index, rand_seed);
	if (status == -2)
	{
		camstate.active = 0;
		return;
	}

	camstate.active = 1;

	camstate.area_index = area_index;
//	camstate.region_index = region_index;
	camstate.time_until_random = 1000;

// time_until_random is a few minutes for missions - the music uses only the region's deterministic seed until then
// it's 0 for startup and custom games
// rand_seed should preferably be something fairly random (e.g. mouse position or something like that)
// if (camstate.time_until_random <= 0)
//		camstate.rand_seed = rand_seed;
//	  else

 if (region_index != -1)
	{
  camstate.rand_seed = area_music [area_index].region_seed [region_index]; //rand_seed;
 	camstate.delayed_rand_seed = rand_seed; // only relevant if time_until_random > 0
	}
   else
   {
   	camstate.rand_seed = rand_seed;
   	camstate.area_index = AREA_BLUE + sthread_rand(6);
   	camstate.delayed_rand_seed = rand_seed + 1; // could probably just be rand_seed
   }

	camstate.tempo = 1;
	camstate.counter = 8 + camstate.tempo; // 8 is to put a short delay at the start
	camstate.total_time = 16;
	camstate.next_change = 40 + sthread_rand(20);

	int i, j;

	for (i = 0; i < CAM_W; i ++)
	{
	 for (j = 0; j < CAM_H; j ++)
	 {
	 	camstate.cam_grid [i] [j] = -1;
	 }
	}

	for (i = 0; i < CAM_AGENTS; i ++)
	{
		camstate.agent [i].exists = 0;

	}

	camstate.max_agents = 11 + sthread_rand(6);
	camstate.active_agents = 1 + sthread_rand(5);

 for (i = 0; i < camstate.active_agents; i ++)
	{

		sthread_place_agent_randomly(i);
/*
		camstate.agent [i].exists = 1;
		camstate.agent [i].x = sthread_rand(CAM_W);
		camstate.agent [i].y = sthread_rand(CAM_H);

		camstate.agent [i].direction = sthread_rand(CADIRS);*/

//		camstate.agent [i].scale_index = sthread_rand(2);// + sthread_rand_r(4);

/*
		camstate.agent [i].msample_index = MSAMPLE_NOTE + sthread_rand(3);// + sthread_rand_r(4);
		if (camstate.agent [i].msample_index == MSAMPLE_NOTE2)
			camstate.agent [i].msample_index = MSAMPLE_NOTE3;

		camstate.agent [i].msample_index = MSAMPLE_NOTE3;
*/
	}



//	camstate.scale_index = 1;//SCALE_MINOR;
//	camstate.note_offset = 0;
	camstate.base_echo_strength [0] = 6;//sthread_rand(12);
	camstate.base_echo_strength [1] = 6;//sthread_rand(12);
	camstate.base_echo_strength [2] = 6;//sthread_rand(12);

 camstate.scale_index = area_music [area_index].key_type [sthread_rand(AREA_MUSIC_KEY_TYPES)];//CHOOSE_SCALE_INDEX; //2;//sthread_rand(SCALE_NOTE_TYPE);
 camstate.note_offset = 0;

 sthread_new_instrument(0, camstate.scale_index, sthread_random_instrument_type(), area_music [area_index].key_type [sthread_rand(AREA_MUSIC_EFFECTS)], camstate.note_offset);
// sthread_new_instrument(0, camstate.scale_index, sthread_random_instrument_type(), INSTRUMENT_TYPE_DRUM, camstate.note_offset);
 sthread_new_instrument(1, camstate.scale_index, sthread_random_instrument_type(), area_music [area_index].key_type [sthread_rand(AREA_MUSIC_EFFECTS)], camstate.note_offset);
 sthread_new_instrument(2, camstate.scale_index, sthread_random_instrument_type(), area_music [area_index].key_type [sthread_rand(AREA_MUSIC_EFFECTS)], camstate.note_offset);

 sthread_reset_drum_index();

    sthread_set_bar_type();

				camstate.bar_pos = 0;

    al_set_timer_speed(sound_timer, 0.1);

}



// This only needs to be called once, on startup
void sthread_init_sample_pointers(void)
{

 int i, j;

	for (i = 0; i < INSTRUMENTS; i ++)
	{
		for (j = 0; j < SCALE_TONES; j++)
		{
// set these to NULL to make sure they are not destroyed when new ones are being created:
			scale_sample [0] [i] [j] = NULL;
			scale_sample [1] [i] [j] = NULL;
		}
	}


}


static void sthread_place_agent_randomly(int agent_index)
{

	int counter = 0;

 while(TRUE)
	{
		camstate.agent [agent_index].x = sthread_rand(CAM_W);
		camstate.agent [agent_index].y = sthread_rand(CAM_H);

		if (camstate.cam_grid [camstate.agent [agent_index].x] [camstate.agent [agent_index].y] == -1)
			break;

		counter ++;
		if (counter > 1000)
			return; // failed - very unlikely to actually happen
	}

	camstate.agent [agent_index].exists = 1;
	camstate.agent [agent_index].direction = sthread_rand(CADIRS);
	camstate.agent [agent_index].collision_time = 0;
	camstate.agent [agent_index].echo_strength = 0;
	camstate.agent [agent_index].instrument = sthread_rand(INSTRUMENTS);
	camstate.cam_grid [camstate.agent [agent_index].x] [camstate.agent [agent_index].y] = agent_index;


}

// removes the last agent and decrements camstate.active_agents
static void sthread_remove_agent(void)
{

	if (camstate.active_agents < 2) // leave at least 1
		return;

	int removed_agent = camstate.active_agents - 1;

	camstate.agent [removed_agent].exists = 0;

	camstate.cam_grid [camstate.agent [removed_agent].x] [camstate.agent [removed_agent].y] = -1;

	camstate.active_agents --;

}


static int sthread_random_instrument_type(void)
{

//	if (sthread_rand(3) == 0)
//		return sthread_rand(INSTRUMENT_TYPES);

 return area_music[camstate.area_index].instrument_type [sthread_rand(AREA_MUSIC_INSTRUMENTS)];

}

static void sthread_reset_drum_index(void)
{

 int i;

 for (i = 0; i < DRUMS; i ++)
	{
		camstate.drum_index [i] = 0; // this is potentially a valid index in the sample array, but we can assume that it's not meant to be a drum beat
	}

//  	camstate.drum_index [DRUM_1] = SAMPLE_THUMP;
//  	camstate.drum_index [DRUM_2] = SAMPLE_DRUM3;
//  	camstate.drum_index [DRUM_3] = SAMPLE_NOTE_HARM;
//  	camstate.drum_index [DRUM_4] = SAMPLE_LOW2;


}


static void sthread_change_drums(void)
{

 if (area_music[camstate.area_index].drum_type [0] == -1)
		return;

	switch(sthread_rand(7))
	{
		default:
	 case 0:
		 sthread_reset_drum_index();
		 break;
		case 1:
  	camstate.drum_index [DRUM_1] = FIRST_DRUM_SAMPLE + area_music[camstate.area_index].drum_type [0];
  	break;
		case 2:
  	camstate.drum_index [DRUM_2] = FIRST_DRUM_SAMPLE + area_music[camstate.area_index].drum_type [1];
  	break;
		case 3:
  	camstate.drum_index [DRUM_3] = FIRST_DRUM_SAMPLE + area_music[camstate.area_index].drum_type [2];
  	break;
		case 4:
  	camstate.drum_index [DRUM_4] = FIRST_DRUM_SAMPLE + area_music[camstate.area_index].drum_type [3];
  	break;
	}
}


/*
static void sthread_reset_camstate_beat(float base_tempo, int bar_length, float stress_vol, float beat_extra_time)
{
//fpr("\n reset camstate beat: bar_length %i  beat_extra_time %f", bar_length, beat_extra_time);
	camstate.beat_timer = 0;
	camstate.base_tempo = base_tempo;
 al_set_timer_speed(sound_timer, base_tempo);

	camstate.bar_length = bar_length;
	camstate.bar_length = bar_length;
	camstate.stress_vol = stress_vol;
	camstate.beat_extra_time = beat_extra_time;

}
*/



void sthread_run_camstate(void)
{
//return;
//#define BASE_ECHO_STRENGTH 10

 if (!camstate.active)
		return;

	int i;
/*	for (i = 0; i < CAM_AGENTS; i ++)
	{


		if (!camstate.agent[i].exists)
			continue;

		camstate.agent[i].echo_strength --;
fpr("\n str %i ", camstate.agent[i].echo_strength);
		if (camstate.agent[i].echo_strength > 0
			&& camstate.agent[i].echo_strength < BASE_ECHO_STRENGTH-1)
//			&& (camstate.agent[i].echo_strength&1))
		{
    sthread_play_msample(camstate.agent[i].msample_index, scale [camstate.scale_index] [camstate.agent[i].echo_tone], 20 + camstate.agent[i].echo_strength * 20, -50 + camstate.agent[i].echo_pan + ((camstate.agent[i].echo_strength&2) * 100));
		}
	}*/

 camstate.counter --;

	if (camstate.counter > 0)
		return;



// static int beat_timer = 0;

 float stress = 1;

 camstate.bar_pos ++;

 if (camstate.bar_pos >= camstate.bar_length)
		camstate.bar_pos = 0;

	if (camstate.bar_beat_drum [camstate.bar_pos] != 0)
	{
	 if (camstate.drum_index [camstate.bar_beat_drum [camstate.bar_pos]] != 0)
	  al_play_sample(sample [camstate.drum_index [camstate.bar_beat_drum [camstate.bar_pos]]] , 0.7 * sound_config.music_volume, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}

//	if (camstate.bar_pos == 4)
	 //al_play_sample(sample [SAMPLE_DRUM3] , 1 * sound_config.music_volume, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

 al_set_timer_speed(sound_timer, camstate.bar_beat_time [camstate.bar_pos]);
 stress = camstate.bar_beat_vol [camstate.bar_pos];

// camstate.beat_timer ++;

/*
 switch(camstate.beat_timer)
 {
	 case 1:
		 al_set_timer_speed(sound_timer, camstate.base_tempo + camstate.beat_extra_time);
		 break;
	 case 2:
		 al_set_timer_speed(sound_timer, camstate.base_tempo);
		 stress += camstate.stress_vol;
	  break;
	 default:
			if (camstate.beat_timer == camstate.bar_length)
				camstate.beat_timer = 0;
			break;
 }


 switch(camstate.beat_timer)
 {
	 case 1:
		 al_set_timer_speed(sound_timer, camstate.base_tempo + camstate.beat_extra_time);
		 break;
	 case 2:
		 al_set_timer_speed(sound_timer, camstate.base_tempo);
		 stress += camstate.stress_vol;
	  break;
	 default:
			if (camstate.beat_timer == camstate.bar_length)
				camstate.beat_timer = 0;
			break;
 }
*/

	camstate.counter = 1;//camstate.tempo; // tempo is always 1 anyway
	camstate.total_time ++;
	camstate.next_change --;

	if (camstate.total_time == camstate.time_until_random)
	{
// after a while, the deterministic mission music becomes random.
		camstate.rand_seed = camstate.delayed_rand_seed;
	}


	for (i = 0; i < camstate.active_agents; i ++)
	{


		if (!camstate.agent[i].exists
			 || camstate.agent[i].collision_time == camstate.total_time)
			continue;


		switch(camstate.agent [i].direction)
		{
		 case CADIR_UP:
		 	if (camstate.agent [i].y == 0)
				{
     sthread_cam_note(i, camstate.agent[i].x, stress);
					camstate.agent[i].direction = CADIR_DOWN;
					break;
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = -1;
				camstate.agent[i].y --;
				if (camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] != -1)
				{
					camstate.agent[i].direction = CADIR_RIGHT;
					sthread_rotate_cam_agent(camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y]);
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = i;
				break;
		 case CADIR_DOWN:
		 	if (camstate.agent [i].y == CAM_H - 1)
				{
     sthread_cam_note(i, camstate.agent[i].x, stress);
					camstate.agent[i].direction = CADIR_UP;
					break;
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = -1;
				camstate.agent[i].y ++;
				if (camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] != -1)
				{
					camstate.agent[i].direction = CADIR_LEFT;
					sthread_rotate_cam_agent(camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y]);
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = i;
				break;
		 case CADIR_LEFT:
		 	if (camstate.agent [i].x == 0)
				{
     sthread_cam_note(i, camstate.agent[i].y, stress);
					camstate.agent[i].direction = CADIR_RIGHT;
					break;
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = -1;
				camstate.agent[i].x --;
				if (camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] != -1)
				{
					camstate.agent[i].direction = CADIR_UP;
					sthread_rotate_cam_agent(camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y]);
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = i;
				break;
		 case CADIR_RIGHT:
		 	if (camstate.agent [i].x == CAM_W - 1)
				{
     sthread_cam_note(i, camstate.agent[i].y, stress);
					camstate.agent[i].direction = CADIR_LEFT;
					break;
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = -1;
				camstate.agent[i].x ++;
				if (camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] != -1)
				{
					camstate.agent[i].direction = CADIR_DOWN;
					sthread_rotate_cam_agent(camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y]);
				}
				camstate.cam_grid [camstate.agent[i].x] [camstate.agent[i].y] = i;
				break;
		}


		camstate.agent[i].echo_strength --;

		if (camstate.agent[i].echo_strength > 0
//			&& camstate.agent[i].echo_strength < BASE_ECHO_STRENGTH-1
			&& (camstate.agent[i].echo_strength&1))
		{
//    sthread_play_scale_sample(camstate.agent[i].echo_flip, camstate.agent[i].echo_instrument, camstate.agent[i].echo_tone, 5 + camstate.agent[i].echo_strength * 10, -50 + camstate.agent[i].echo_pan + ((camstate.agent[i].echo_strength&2) * 50));

//    sthread_play_scale_sample(camstate.agent[i].echo_flip, camstate.agent[i].echo_instrument, camstate.agent[i].echo_tone, (25 + camstate.agent[i].echo_strength * 10) * stress, -50 + camstate.agent[i].echo_pan + ((camstate.agent[i].echo_strength&2) * 50));
    sthread_play_scale_sample(camstate.agent[i].echo_flip, camstate.agent[i].echo_instrument, camstate.agent[i].echo_tone, (25 + camstate.agent[i].echo_strength * 10) * stress, -50 + camstate.agent[i].echo_pan + ((camstate.agent[i].echo_strength&2) * 50),
																														1.0);

		}

	}

	if (camstate.next_change <= 0)
	{
// if fewer than the maximum number of agents are currently active, add one:
		if (camstate.active_agents < camstate.max_agents)
		{
			sthread_place_agent_randomly(camstate.active_agents);
			camstate.active_agents ++;
		}
		sthread_change_camstate();
		camstate.next_change = 40 + sthread_rand(120);
	}


}

unsigned int sthread_rand(unsigned int rand_max)
{

 camstate.rand_seed = camstate.rand_seed * 1103515245 + 12345;
 return (unsigned int)(camstate.rand_seed / 65536) % rand_max;

}

static void sthread_cam_note(int agent_index, int tone_index, float stress)
{
//fpr("\n sthread_cam_note agent %i tone %i stress %f", agent_index, tone_index, stress);

 camstate.agent[agent_index].echo_pan = -50 + (camstate.agent[agent_index].x * 100) / CAM_W;
// sthread_play_msample(camstate.agent[agent_index].msample_index, scale [camstate.scale_index] [tone_index] + camstate.note_offset, 100, camstate.agent[agent_index].echo_pan);
 sthread_play_scale_sample(camstate.instrument_sample_flip[camstate.agent[agent_index].instrument],
																											camstate.agent[agent_index].instrument,
																											tone_index,
																											100 * stress,
																											camstate.agent[agent_index].echo_pan,
																											1.0);

 camstate.agent[agent_index].echo_strength = camstate.base_echo_strength [camstate.agent[agent_index].instrument];//BASE_ECHO_STRENGTH / 2;
 camstate.agent[agent_index].echo_tone = tone_index;//scale [camstate.scale_index] [tone_index] + camstate.note_offset;
 camstate.agent[agent_index].echo_instrument = camstate.agent[agent_index].instrument;
 camstate.agent[agent_index].echo_flip = camstate.instrument_sample_flip[camstate.agent[agent_index].instrument];

}

static void sthread_rotate_cam_agent(int agent_index)
{
	camstate.agent[agent_index].direction ++;
	camstate.agent[agent_index].direction %= CADIRS;
	camstate.agent[agent_index].collision_time = camstate.total_time;
}

// makes some kind of random change to camstate.
static void sthread_change_camstate(void)
{

 int changes = 1;

 if (sthread_rand(4) == 0)
		changes += sthread_rand(5);
// there may not be this many actual changes, as some of the change types result in this function returning (to avoid double-flipping an instrument)

	int change_type;
	int i;

 while(changes > 0)
	{
		change_type = sthread_rand(12);

		switch(change_type)
		{
		 case 0: // scale change - applies to all instruments
		 	{
		 		camstate.scale_index = area_music [camstate.area_index].key_type [sthread_rand(AREA_MUSIC_KEY_TYPES)];//2;//sthread_rand(SCALE_NOTE_TYPE);
     for (i = 0; i < INSTRUMENTS; i ++)
					{
						sthread_new_instrument(i, camstate.scale_index, camstate.instrument_type [i], camstate.instrument_effect [i], camstate.note_offset);
					}
		 	}
		 	return; // return, not break
		 case 1: // note offset change - applies to all instruments
		 	{
		 		camstate.note_offset = sthread_rand(9);
     for (i = 0; i < INSTRUMENTS; i ++)
					{
						sthread_new_instrument(i, camstate.scale_index, camstate.instrument_type [i], camstate.instrument_effect [i], camstate.note_offset);
					}
		 	}
		 	return; // return, not break
		 case 2: // one instrument is changed
				{
						sthread_new_instrument(sthread_rand(INSTRUMENTS), camstate.scale_index, sthread_random_instrument_type(), area_music[camstate.area_index].effect_type[sthread_rand(AREA_MUSIC_EFFECTS)], camstate.note_offset);
				}
				return; // return, not break
			case 3: // echo strength change
				camstate.base_echo_strength [sthread_rand(INSTRUMENTS)] = 3 + sthread_rand(5);
				break;
			case 4: // cam agent direction change
				{
					int agent_index = sthread_rand(CAM_AGENTS);
				 camstate.agent[agent_index].direction = sthread_rand(CADIRS);
// doesn't matter if agent isn't active
				}
				break;
/*
			case 5:	// beat change
				{
// this doesn't reset the bar_length or beat_extra_time, as changing these doesn't sound good
					float new_stress_vol = 0;
					if (sthread_rand(2) == 0)
						new_stress_vol = sthread_rand(5);

     sthread_reset_camstate_beat(camstate.base_tempo,
																																	camstate.bar_length, // bar_length
																													    new_stress_vol * 0.05, // stress_vol
																													    camstate.beat_extra_time); // beat_extra_time

				}
				break;*/
			case 6: // reduce number of agents
				{
					int agents_to_remove = sthread_rand(7) + 1;

					if (camstate.active_agents <= agents_to_remove + 1)
						break;

					for (i = 0; i < agents_to_remove; i ++)
					{
						sthread_remove_agent(); // removes last agent and clears it from camgrid.
						 // won't remove the last one or two agents
						// removed agents will be gradually added back, like they are at the start of the piece.
					}

				}
				break;
			case 7: // change drum state
			case 8: // change drum state
			case 9: // change drum state
			case 10: // change drum state
			case 11: // change drum state
				sthread_change_drums();
				break;



// remember to change sthread_rand() call above when adding more cases!



		}

		changes --;
	}

// may not reach this far

}


static void sthread_new_instrument(int instrument, int scale_type, int instrument_type, int effect, int tone_offset)
{

	int new_flip = camstate.instrument_sample_flip [instrument] ^ 1;

 sthread_create_samples_for_scale(new_flip, instrument, scale_type, instrument_type, effect, tone_offset);

 camstate.instrument_sample_flip [instrument] = new_flip;
 camstate.instrument_type [instrument] = instrument_type;
 camstate.instrument_effect [instrument] = effect;

}


float scale_sample_data [2] [INSTRUMENTS] [SCALE_TONES] [SYNTH_SAMPLE_MAX_SIZE];

float temp_buffer [SYNTH_SAMPLE_MAX_SIZE];


static void sthread_create_samples_for_scale(int flip, int instrument, int scale_type, int instrument_type, int effect, int tone_offset)
{

	int i;

 float* current_buffer;
 int buffer_length = 8000;

 int attack_length = 50; // counted from start
 int decay_length = 200; // counted from end of attack
 int release_length = 4500; // counted back from end

 float base_freq;


// base_freq = 125 * pow(1.059463, tone_offset); // should this be something else? I don't know how it maps to Hz or whatever
 base_freq = 125 * pow(1.059463, tone_offset); // should this be something else? I don't know how it maps to Hz or whatever

 switch(effect)
 {
// 	case INSTRUMENT_EFFECT_LOW_PITCH:
//   base_freq /= 2; break;
// 	case INSTRUMENT_EFFECT_VERY_HIGH_PITCH: // * 4 doesn't sound that great
 	case INSTRUMENT_EFFECT_HIGH_PITCH:
   base_freq *= 2; break;
 	case INSTRUMENT_EFFECT_LOW_PITCH:
   base_freq /= 2; break;
// 	case INSTRUMENT_EFFECT_VERY_HIGH_PITCH:
//   base_freq *= 4; break;
  case INSTRUMENT_EFFECT_SHORT:
			buffer_length = 5000;
			release_length = 1500;
			break;
  case INSTRUMENT_EFFECT_LONG:
			buffer_length = 10000; // must not be longer than SYNTH_SAMPLE_MAX_SIZE
			break;
			// case INSTRUMENT_EFFECT_VIB:   - this is tested for below
 }

//instrument_type = INSTRUMENT_TYPE_BASIC_SINE + sthread_rand(3);

//if (sthread_rand(3) == 0)
//	instrument_type = INSTRUMENT_TYPE_DRUM;

 switch(instrument_type)
 {
/*
  case INSTRUMENT_TYPE_DRUM:
  	camstate.base_echo_strength [instrument] = 0;
	  for (i = 0; i < SCALE_TONES; i ++)
	  {

    current_buffer = scale_sample_data [flip] [instrument] [i];

//    buffer_length = 5000;

    init_waveform(current_buffer, buffer_length);

//    add_waveform_square(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 0, 0, 0.3);

    add_waveform_noise(current_buffer, buffer_length, i, instrument_type, 0.3);
//     add_waveform_square(temp_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 0, 0, 0.5);

//    add_waveform_square(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [0], instrument_type - INSTRUMENT_TYPE_SQUARE, 0, 0.5);
//    add_waveform_square(temp_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 0, 0, 0.3);
//    add_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], instrument_type, 0, 1);

    apply_adsr(current_buffer, buffer_length,//buffer_length,
												   10, // attack_length, // attack length
												   2000, 0.3, // decay_length, 0.5, // decay length, target amplification after decay
												   3000); // release_length); // release length (counted back from end


    float amplification = 0.3 - (instrument_type * 0.1);
    amplify(current_buffer, buffer_length, amplification);
    sthread_finish_instrument(flip, instrument, i, current_buffer, buffer_length);

	 }
	 break;
*/
 	case INSTRUMENT_TYPE_BASIC_SINE:
 	case INSTRUMENT_TYPE_SINE_HARM1:
	 case INSTRUMENT_TYPE_SINE_HARM2:
// sine instrument code follows:
	  for (i = 0; i < SCALE_TONES; i ++)
	  {

    current_buffer = scale_sample_data [flip] [instrument] [i];

    init_waveform(current_buffer, buffer_length);



//    add_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 0, 1);
    if (effect != INSTRUMENT_EFFECT_VIB)
     set_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], instrument_type - INSTRUMENT_TYPE_BASIC_SINE, 0, 1);
      else
       set_waveform_sine_vib(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], instrument_type - INSTRUMENT_TYPE_BASIC_SINE, 0, 1);

/*
    if (instrument_type >= INSTRUMENT_TYPE_SINE_HARM1)
				{
			  init_waveform(temp_buffer, buffer_length);
     add_waveform_sine(temp_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i] * 2, 0, 0.4);

     add_waveforms(current_buffer, temp_buffer, buffer_length);
				}

    if (instrument_type >= INSTRUMENT_TYPE_SINE_HARM2)
				{
			  init_waveform(temp_buffer, buffer_length);
     add_waveform_sine(temp_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i] * 4, 0, 0.4);

     add_waveforms(current_buffer, temp_buffer, buffer_length);
				}
*/
    apply_adsr(current_buffer, buffer_length,
												   attack_length, // attack length
												   decay_length, 0.5, // decay length, target amplification after decay
												   release_length); // release length (counted back from end


    float amplification = 0.3 - (instrument_type * 0.1);
    amplify(current_buffer, buffer_length, amplification);
    sthread_finish_instrument(flip, instrument, i, current_buffer, buffer_length);


	 }
	 break;

	 case INSTRUMENT_TYPE_SINE_SQUARE:
	  for (i = 0; i < SCALE_TONES; i ++)
	  {

    current_buffer = scale_sample_data [flip] [instrument] [i];
    init_waveform(current_buffer, buffer_length);

    if (effect != INSTRUMENT_EFFECT_VIB)
     set_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 1, 0, 1);
      else
       set_waveform_sine_vib(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 1, 0, 1);

//    if (i % 3 == 0)
//				{

			  init_waveform(temp_buffer, buffer_length);
     set_waveform_square(temp_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], 0, 0, 0.3);
/*    apply_adsr(temp_buffer, buffer_length,
												   buffer_length - 100, // attack length
												   50, 0.5, // decay length, target amplification after decay
												   50); // release length (counted back from end*/


     add_waveforms(current_buffer, temp_buffer, buffer_length);
//				}

    apply_adsr(current_buffer, buffer_length,
												   attack_length, // attack length
												   decay_length, 0.3, // decay length, target amplification after decay
												   release_length); // release length (counted back from end

    amplify(current_buffer, buffer_length, 0.2);
    sthread_finish_instrument(flip, instrument, i, current_buffer, buffer_length);
//     scale_sample [flip] [instrument] [i] = finish_sample(current_buffer, buffer_length); // finish_sample exits on failure to create sample

	 }
	 break;


	 case INSTRUMENT_TYPE_SQUARE:
  case INSTRUMENT_TYPE_SQUARE_HARM1:
  case INSTRUMENT_TYPE_SQUARE_HARM2:
	  for (i = 0; i < SCALE_TONES; i ++)
	  {

    current_buffer = scale_sample_data [flip] [instrument] [i];
    init_waveform(current_buffer, buffer_length);

    if (effect != INSTRUMENT_EFFECT_VIB)
     set_waveform_square(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], instrument_type - INSTRUMENT_TYPE_SQUARE, 0, 0.5);
      else
       set_waveform_square_vib(current_buffer, buffer_length, base_freq * scale_ratio [scale_type] [i], instrument_type - INSTRUMENT_TYPE_SQUARE, 0, 0.5);


//    if (i % 3 == 0)
//				{


    apply_adsr(current_buffer, buffer_length,
												   attack_length, // attack length
												   decay_length, 0.3, // decay length, target amplification after decay
												   release_length); // release length (counted back from end

    amplify(current_buffer, buffer_length, 0.2);

    sthread_finish_instrument(flip, instrument, i, current_buffer, buffer_length);
//    scale_sample [flip] [instrument] [i] = finish_sample(current_buffer, buffer_length); // finish_sample exits on failure to create sample

	 }
	 break;
 }

/*
 scale_note_type = 0;

	for (i = 0; i < SCALE_TONES; i ++)
	{

  current_buffer = scale_sample_data [flip] [scale_note_type] [i];
  init_waveform(current_buffer, buffer_length);

  add_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [0] [i], 0, 1);

  apply_adsr(current_buffer, buffer_length,
												 50, // attack length
												 200, 0.5, // decay length, target amplification after decay
												 3500); // release length (counted back from end

 amplify(current_buffer, buffer_length, 0.1);
  scale_sample [scale_note_type] [i] = finish_sample(current_buffer, buffer_length); // finish_sample exits on failure to create sample

	}


 scale_note_type = 1;

	for (i = 0; i < SCALE_TONES; i ++)
	{

  current_buffer = scale_sample_data [flip] [scale_note_type] [i];
  init_waveform(current_buffer, buffer_length);

  add_waveform_sine(current_buffer, buffer_length, base_freq * scale_ratio [0] [i] * 2, 0, 1);


/ *
   init_waveform(temp_buffer, buffer_length);
   add_waveform_sine(temp_buffer, buffer_length, base_freq * scale_ratio [0] [i] * 4, 0, 0.4);

   add_waveforms(current_buffer, temp_buffer, buffer_length);
* /

   init_waveform(temp_buffer, buffer_length);
   add_waveform_sine(temp_buffer, buffer_length, base_freq * scale_ratio [0] [i], 0, 0.4);

   add_waveforms(current_buffer, temp_buffer, buffer_length);



  apply_adsr(current_buffer, buffer_length,
												 50, // attack length
												 200, 0.5, // decay length, target amplification after decay
												 3500); // release length (counted back from end

 amplify(current_buffer, buffer_length, 0.1);
  scale_sample [scale_note_type] [i] = finish_sample(current_buffer, buffer_length); // finish_sample exits on failure to create sample

	}

*/

}


static void sthread_finish_instrument(int flip, int instr, int note_index, float* buffer, int buffer_length)
{

// scale_sample array should have been initialised to NULL
	if (scale_sample [flip] [instr] [note_index] != NULL)
		al_destroy_sample(scale_sample [flip] [instr] [note_index]);

	scale_sample [flip] [instr] [note_index] = finish_sample(buffer, buffer_length);

}



// randomly selects a bar type
// should only be done when the whole camstate is reset, otherwise it sounds terrible
static void sthread_set_bar_type(void)
{

 int i;

 for (i = 0; i < BAR_LENGTH_MAX; i ++)
	{
		camstate.bar_beat_drum [i] = DRUM_NONE;
	}


	switch(area_music[camstate.area_index].bar_type)
	{
	 case 0:
		default:
// equal 4 beats, no stress:
			camstate.bar_length = 4;
			camstate.bar_beat_time [0] = 0.20;
			camstate.bar_beat_vol [0] = 1.1;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.20;
			camstate.bar_beat_vol [1] = 1.1;
 		camstate.bar_beat_drum [1] = DRUM_2;
			camstate.bar_beat_time [2] = 0.20;
			camstate.bar_beat_vol [2] = 1.1;
 		camstate.bar_beat_drum [2] = DRUM_3;
			camstate.bar_beat_time [3] = 0.20;
			camstate.bar_beat_vol [3] = 1.1;
 		camstate.bar_beat_drum [3] = DRUM_4;
			break;

	 case 1:
// equal 4 beats, stress on 0 and 2:
			camstate.bar_length = 4;
			camstate.bar_beat_time [0] = 0.20;
			camstate.bar_beat_vol [0] = 1.3;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.20;
			camstate.bar_beat_vol [1] = 0.8;
 		camstate.bar_beat_drum [1] = DRUM_2;
			camstate.bar_beat_time [2] = 0.20;
			camstate.bar_beat_vol [2] = 1.3;
 		camstate.bar_beat_drum [2] = DRUM_3;
			camstate.bar_beat_time [3] = 0.20;
			camstate.bar_beat_vol [3] = 0.8;
 		camstate.bar_beat_drum [3] = DRUM_4;
			break;

	 case 2:
// 4 beats, delay and stress on 0 and 2:
			camstate.bar_length = 4;
			camstate.bar_beat_time [0] = 0.36;
			camstate.bar_beat_vol [0] = 1.3;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.20;
			camstate.bar_beat_vol [1] = 0.8;
 		camstate.bar_beat_drum [1] = DRUM_2;
			camstate.bar_beat_time [2] = 0.36;
			camstate.bar_beat_vol [2] = 1.3;
 		camstate.bar_beat_drum [2] = DRUM_3;
			camstate.bar_beat_time [3] = 0.20;
			camstate.bar_beat_vol [3] = 0.8;
 		camstate.bar_beat_drum [3] = DRUM_4;
			break;

	 case 3:
// 5 beats, delay and stress on 0 and 2:
			camstate.bar_length = 5;
			camstate.bar_beat_time [0] = 0.36;
			camstate.bar_beat_vol [0] = 1.3;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.20;
			camstate.bar_beat_vol [1] = 0.8;
 		camstate.bar_beat_drum [1] = DRUM_2;
			camstate.bar_beat_time [2] = 0.36;
			camstate.bar_beat_vol [2] = 1.3;
 		camstate.bar_beat_drum [2] = DRUM_3;
			camstate.bar_beat_time [3] = 0.20;
			camstate.bar_beat_vol [3] = 0.8;
 		camstate.bar_beat_drum [3] = DRUM_4;
			camstate.bar_beat_time [4] = 0.20;
			camstate.bar_beat_vol [4] = 0.8;
			break;

	 case 4:
// 3 beats, delay and stress on 0:
			camstate.bar_length = 3;
			camstate.bar_beat_time [0] = 0.42;
			camstate.bar_beat_vol [0] = 1.3;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.17;
			camstate.bar_beat_vol [1] = 0.8;
 		camstate.bar_beat_drum [1] = DRUM_2;
			camstate.bar_beat_time [2] = 0.17;
			camstate.bar_beat_vol [2] = 0.8;
			break;

  case 5:
// ! ....! ! ....!
				camstate.bar_length = 6;
				camstate.bar_beat_time [0] = 0.4;
				camstate.bar_beat_vol [0] = 1.3;
 		 camstate.bar_beat_drum [0] = DRUM_1;
				camstate.bar_beat_time [1] = 0.18;
				camstate.bar_beat_vol [1] = 0.9;
				camstate.bar_beat_time [2] = 0.18;
				camstate.bar_beat_vol [2] = 0.9;
 		 camstate.bar_beat_drum [2] = DRUM_2;
				camstate.bar_beat_time [3] = 0.18;
				camstate.bar_beat_vol [3] = 0.9;
				camstate.bar_beat_time [4] = 0.4;
				camstate.bar_beat_vol [4] = 0.9;
 		 camstate.bar_beat_drum [4] = DRUM_3;
				camstate.bar_beat_time [5] = 0.4;
				camstate.bar_beat_vol [5] = 1.1;
			break;
  case 6:
// ! ....!! ! ....!!
				camstate.bar_length = 7;
				camstate.bar_beat_time [0] = 0.4;
				camstate.bar_beat_vol [0] = 1.3;
  		camstate.bar_beat_drum [0] = DRUM_1;
				camstate.bar_beat_time [1] = 0.18;
				camstate.bar_beat_vol [1] = 0.9;
				camstate.bar_beat_time [2] = 0.18;
				camstate.bar_beat_vol [2] = 0.9;
	 		camstate.bar_beat_drum [2] = DRUM_2;
 			camstate.bar_beat_time [3] = 0.18;
				camstate.bar_beat_vol [3] = 0.9;
				camstate.bar_beat_time [4] = 0.4;
				camstate.bar_beat_vol [4] = 0.9;
	 		camstate.bar_beat_drum [4] = DRUM_3;
				camstate.bar_beat_time [5] = 0.4;
				camstate.bar_beat_vol [5] = 1.1;
				camstate.bar_beat_time [6] = 0.4;
				camstate.bar_beat_vol [6] = 1.1;
	 		camstate.bar_beat_drum [6] = DRUM_4;
			break;

  case 7:
// increasing speed and stress
				camstate.bar_length = 7;
				camstate.bar_beat_time [0] = 0.18;
				camstate.bar_beat_vol [0] = 1.3;
	 		camstate.bar_beat_drum [0] = DRUM_1;
				camstate.bar_beat_time [1] = 0.20;
				camstate.bar_beat_vol [1] = 1.2;
				camstate.bar_beat_time [2] = 0.22;
				camstate.bar_beat_vol [2] = 1.1;
	 		camstate.bar_beat_drum [2] = DRUM_2;
				camstate.bar_beat_time [3] = 0.25;
				camstate.bar_beat_vol [3] = 1.0;
				camstate.bar_beat_time [4] = 0.28;
				camstate.bar_beat_vol [4] = 0.9;
	 		camstate.bar_beat_drum [4] = DRUM_3;
				camstate.bar_beat_time [5] = 0.31;
				camstate.bar_beat_vol [5] = 0.8;
				camstate.bar_beat_time [6] = 0.35;
				camstate.bar_beat_vol [6] = 0.7;
			break;

	 case 8:
// ! ., ! ., !
			camstate.bar_length = 3;
			camstate.bar_beat_time [0] = 0.42;
			camstate.bar_beat_vol [0] = 1.3;
 		camstate.bar_beat_drum [0] = DRUM_1;
			camstate.bar_beat_time [1] = 0.17;
			camstate.bar_beat_vol [1] = 0.8;
			camstate.bar_beat_time [2] = 0.28;
			camstate.bar_beat_vol [2] = 1.1;
			break;

	}

}


static void sthread_play_scale_sample(int instrument_flip, int instrument, int play_note, float vol, int pan, float freq)
{
//fpr("\n play flip %i instr %i note %i", instrument_flip, instrument, play_note);
// if (instrument == 0)
  //al_play_sample(scale_sample [instrument_flip] [instrument] [play_note], vol * 0.01 * sound_config.music_volume, pan * 0.01, 1.0 + (sthread_rand(100) * 0.001), ALLEGRO_PLAYMODE_ONCE, NULL);
   //else
    al_play_sample(scale_sample [instrument_flip] [instrument] [play_note], vol * 0.01 * sound_config.music_volume, pan * 0.01, freq, ALLEGRO_PLAYMODE_ONCE, NULL);

}



#ifdef OLD_MUSIC

//#define MBOX_W 11
//#define MBOX_H 11

// TRACKS is the number of simultaneous tracks:
#define TRACKS 4


struct mustatestruct
{
 int piece; // which piece is being played
 int piece_pos; // position in the piece command list
 int beat; // time position in bar
 int track_playing [TRACKS]; // which bar the track is playing
 int track_pos [TRACKS]; // data position in bar

// scales not currently used
 int track_scale [TRACKS]; // this is which scale the track is using
 int base_track_scale [TRACKS]; // this is which scale the track will return to after track is finished

 int track_note_offset [TRACKS];

 int note_list [TRACKS] [NOTES]; // this relates a note value to an index in the msample array.

 int track_length; // length of each track

};

#define PIECE_LENGTH 64

enum
{
PIECE_BASIC,
PIECES
};

struct piece_elementstruct
{
 int piece_command;
 int value1;
 int value2;
 int value3;
};

// A piece is a list of commands:
enum
{
PC_END,
PC_PLAY_TRACK, // values: track index, bar index
PC_MUTE, // values: track index
PC_WAIT, // values: bars to wait (not yet implemented)
PC_SCALE, // values: track index, new scale
PC_BASE_SCALE, // values: track index, new base scale
PC_BAR_LENGTH, // values: new bar length
PC_SET_NOTE, // values: track index, NOTE index, msample index
PC_SET_TRACK_NOTE_OFFSET, // values: track_index, offset
};

#define BAR_VALUES 4
#define BAR_LENGTH 16

//A bar is also a list of commands
enum
{
BC_END,
BC_PLAY, // plays sample value [0], tone value [1], time value [2], volume offset value [3]
};

struct barstruct
{
 int bar_command;
 int value [BAR_VALUES];
};

enum
{
BAR_EMPTY,
BAR_BASIC_BACK,
BAR_BASIC_UP,
BAR_BASIC_UP2,
BAR_BASIC_UP3,
BAR_BASIC_UP4,
BAR_BASIC_UP5,
BAR_BASIC_DOWN,

BARS
};

struct barstruct bar [BARS] [BAR_LENGTH] =
{
 {
  {BC_END},
 }, // BAR_EMPTY
 {
  {BC_PLAY, {NOTE_1, 1, 0}},
  {BC_END},
 }, // BAR_BASIC_BACK
 {
  {BC_PLAY, {NOTE_0, TONE_2C, 0, 3}},
  {BC_PLAY, {NOTE_0, TONE_1G, 2}},
  {BC_PLAY, {NOTE_0, TONE_1F, 3, 3}},
  {BC_PLAY, {NOTE_0, TONE_2C, 5}},
  {BC_PLAY, {NOTE_0, TONE_1E, 7, 3}},
  {BC_PLAY, {NOTE_0, TONE_2E, 9}},
  {BC_PLAY, {NOTE_0, TONE_1DS, 11, 3}},
  {BC_PLAY, {NOTE_0, TONE_1A, 13}},
  {BC_END},
 }, // BAR_BASIC_UP
 {
  {BC_PLAY, {NOTE_0, TONE_2C, 0, 3}},
  {BC_PLAY, {NOTE_0, TONE_1G, 2}},
  {BC_PLAY, {NOTE_0, TONE_1F, 3, 3}},
  {BC_PLAY, {NOTE_0, TONE_2C, 5}},
  {BC_PLAY, {NOTE_0, TONE_1E, 7, 3}},
  {BC_PLAY, {NOTE_0, TONE_2E, 9}},
  {BC_PLAY, {NOTE_0, TONE_2DS, 11, 3}},
  {BC_PLAY, {NOTE_0, TONE_2D, 13}},
  {BC_END},
 }, // BAR_BASIC_UP2
 {
  {BC_PLAY, {NOTE_0, TONE_2C, 0, 3}},
  {BC_PLAY, {NOTE_0, TONE_1G, 2}},
  {BC_PLAY, {NOTE_0, TONE_1DS, 3, 3}},
  {BC_PLAY, {NOTE_0, TONE_2C, 5}},
  {BC_PLAY, {NOTE_0, TONE_1E, 7, 3}},
  {BC_PLAY, {NOTE_0, TONE_2E, 9}},
  {BC_PLAY, {NOTE_0, TONE_1DS, 11, 3}},
  {BC_PLAY, {NOTE_0, TONE_1AS, 13}},
  {BC_END},
 }, // BAR_BASIC_UP3
 {
  {BC_PLAY, {NOTE_0, TONE_3C, 0}},
  {BC_PLAY, {NOTE_0, TONE_3DS, 2}},
  {BC_PLAY, {NOTE_0, TONE_3F, 3}},
  {BC_PLAY, {NOTE_0, TONE_3G, 5}},
  {BC_END},
 }, // BAR_BASIC_UP4
 {
  {BC_PLAY, {NOTE_0, TONE_3C, 0}},
  {BC_PLAY, {NOTE_0, TONE_2A, 2}},
  {BC_PLAY, {NOTE_0, TONE_2G, 3}},
  {BC_PLAY, {NOTE_0, TONE_3G, 5}},
  {BC_END},
 }, // BAR_BASIC_UP5
 {
  {BC_PLAY, {NOTE_1, TONE_4C, 0}},
  {BC_PLAY, {NOTE_1, TONE_3C, 2}},
  {BC_PLAY, {NOTE_1, TONE_3A, 3}},
  {BC_PLAY, {NOTE_1, TONE_3F, 5}},
  {BC_END},
 }, // BAR_BASIC_DOWN


};

struct piece_elementstruct piece [PIECES] [PIECE_LENGTH] =
{

// PIECE_BASIC
{
// {PC_PLAY_TRACK, 0, BAR_BASIC_BACK},
 {PC_BAR_LENGTH, 1},
 {PC_SET_NOTE, 0, NOTE_0, MSAMPLE_NOTE},
 {PC_SET_NOTE, 0, NOTE_1, MSAMPLE_NOTE},
 {PC_SET_NOTE, 1, NOTE_0, MSAMPLE_NOTE},
 {PC_SET_NOTE, 1, NOTE_1, MSAMPLE_NOTE},
 {PC_WAIT}, // always need to start with wait
 {PC_BAR_LENGTH, 15},
 {PC_SET_TRACK_NOTE_OFFSET, 0, 15},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP2},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP3},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP2},
 {PC_WAIT},
/* {PC_PLAY_TRACK, 0, BAR_BASIC_UP2},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP3},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP4},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP2},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP5},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_PLAY_TRACK, 1, BAR_BASIC_DOWN},
 {PC_WAIT},
 {PC_PLAY_TRACK, 0, BAR_BASIC_UP},
 {PC_PLAY_TRACK, 1, BAR_BASIC_DOWN},
 {PC_WAIT},*/
 {PC_END},
}, // end PIECE_BASIC

};

struct mustatestruct mustate;

enum
{
SCALE_BASIC,
SCALE_MINOR,
SCALE_MAJOR,
SCALE_TYPES
};

#define SCALE_SIZE 11

int scale [SCALE_TYPES] [SCALE_SIZE] =
{

 { // SCALE_BASIC (pentatonic)
  TONE_2C,
  TONE_2D,
  TONE_2E,
  TONE_2G,
  TONE_2A,
  TONE_3C,
  TONE_3D,
  TONE_3E,
  TONE_3G,
  TONE_2C,
  TONE_2D
 },

//C, D, E, G, and A.
 { // SCALE_MINOR
  TONE_2C,
  TONE_2DS,
  TONE_2F,
  TONE_2G,
  TONE_2AS,
  TONE_3C,
  TONE_3DS,
  TONE_3F,
  TONE_3G,
  TONE_1AS,
  TONE_2C
 },
  { // SCALE_MAJOR
  TONE_2C,
  TONE_2D,
  TONE_2E,
  TONE_2G,
  TONE_2A,
  TONE_3C,
  TONE_3D,
  TONE_3E,
  TONE_3G,
  TONE_3A,
  TONE_1A
 }
};




// This function is called whenever a new piece is started.
void sthread_init_mustate(int play_piece)
{
 int i, j;

 mustate.piece = play_piece;
 mustate.piece_pos = 0;
 mustate.beat = 0;
 mustate.track_length = BAR_LENGTH; // should be reset by piece
 for (i = 0; i < TRACKS; i ++)
 {
  mustate.track_playing [i] = BAR_EMPTY;
  mustate.track_playing [i] = 0;
  mustate.track_pos [i] = 0;
  mustate.track_scale [i] = SCALE_BASIC;
  mustate.base_track_scale [i] = SCALE_BASIC;
  mustate.track_note_offset [i] = 0;
  for (j = 0; j < NOTES; j ++)
		{
			mustate.note_list [i] [j] = MSAMPLE_NOTE;
		}
 }


}

// returns 1 if piece still playing, or 0 if it's finished
int sthread_play_current_piece(void)
{

// Not currently used because I haven't written any music for it yet. Will be returned to service at some point.
 // should be static

 int i;

// First we go through the tracks:
 for (i = 0; i < TRACKS; i ++)
 {
  while(TRUE)
  {

   switch(bar [mustate.track_playing [i]] [mustate.track_pos [i]].bar_command)
   {
    case BC_PLAY:
     if (mustate.beat >= bar [mustate.track_playing [i]] [mustate.track_pos [i]].value [2])
     {
      sthread_play_msample(mustate.note_list [i] [bar [mustate.track_playing [i]] [mustate.track_pos [i]].value [0]], // Note type
																							    bar [mustate.track_playing [i]] [mustate.track_pos [i]].value [1] + mustate.track_note_offset [i], // pitch
																							    bar [mustate.track_playing [i]] [mustate.track_pos [i]].value [3], 0);
//                        scale [mustate.track_scale [i]] [bar [mustate.track_playing [i]] [mustate.track_pos [i]].value [1]],
//                        1);
      break; // will increment mustate.track_pos [i] (after this loop) then continue the loop
     }
     goto finished_track; // not ready to play yet
    case BC_END:
     goto finished_track;
// other BC types might just break instead of exiting the loop.
   }
   mustate.track_pos [i]++;
  };
  finished_track:
   continue;
 }


 mustate.beat++;

// now we check the piece commands:
// int value1, value2;

 while(TRUE)
 {
// these continue until they reach PC_WAIT or PC_END
  switch(piece [mustate.piece] [mustate.piece_pos].piece_command)
  {
   case PC_WAIT:
    if (mustate.beat >= mustate.track_length)
    {
     mustate.beat = 0;
     mustate.piece_pos++;
     continue;
    }
    return 1;
   case PC_PLAY_TRACK: // values: track index, bar index
    mustate.track_playing [piece [mustate.piece] [mustate.piece_pos].value1] = piece [mustate.piece] [mustate.piece_pos].value2;
    mustate.track_pos [piece [mustate.piece] [mustate.piece_pos].value1] = 0;
    mustate.track_scale [piece [mustate.piece] [mustate.piece_pos].value1] = mustate.base_track_scale [piece [mustate.piece] [mustate.piece_pos].value1];
    mustate.piece_pos++;
    break;
   case PC_MUTE:  // values: track index
    mustate.track_playing [piece [mustate.piece] [mustate.piece_pos].value1] = BAR_EMPTY;
    mustate.piece_pos++;
    break;
   case PC_BAR_LENGTH: // values: bar length
    mustate.track_length = piece [mustate.piece] [mustate.piece_pos].value1;
    mustate.piece_pos++;
    break;
   case PC_SCALE: // values: track index, scale index
    mustate.track_scale [piece [mustate.piece] [mustate.piece_pos].value1] = piece [mustate.piece] [mustate.piece_pos].value2;
    mustate.piece_pos++;
    break;
   case PC_BASE_SCALE: // values: track index, scale index
    mustate.base_track_scale [piece [mustate.piece] [mustate.piece_pos].value1] = piece [mustate.piece] [mustate.piece_pos].value2;
    mustate.piece_pos++;
    break;
   case PC_SET_NOTE: // values: track index, NOTE index, msample index
				mustate.note_list [piece [mustate.piece] [mustate.piece_pos].value1] [piece [mustate.piece] [mustate.piece_pos].value2] = piece [mustate.piece] [mustate.piece_pos].value3;
    mustate.piece_pos++;
				break;
			case PC_SET_TRACK_NOTE_OFFSET:
    mustate.track_note_offset [piece [mustate.piece] [mustate.piece_pos].value1] = piece [mustate.piece] [mustate.piece_pos].value2;
    mustate.piece_pos++;
				break;
   case PC_END:
    return 0;
// remember mustate.piece_pos++; if needed!
  }
 };

  return 0; // should never reach here

}


static void sthread_play_msample(int msample_index, int play_tone, int vol, int pan)
{

 al_play_sample(msample [msample_index], vol * 0.01, pan * 0.01, tone [play_tone + 8], ALLEGRO_PLAYMODE_ONCE, NULL);
//			fpr(" vol %f pan %f", vol * 0.01, pan * 0.01);


// al_play_sample(msample [msample_index], 0.5 + ((float) vol * 0.1), 0, tone [play_tone], ALLEGRO_PLAYMODE_ONCE, NULL);

// sthread_add_mbox_echo(mb, note_type, play_tone);
// could pan to left/right depending on x position?

}

#endif

