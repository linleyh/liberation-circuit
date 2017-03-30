
#ifndef H_X_SOUND
#define H_X_SOUND


struct sound_configstruct
{
 float music_volume;
 float effect_volume;
};



void play_interface_sound(int s, int pitch);
void play_game_sound(int s, int pitch, int vol, int priority, al_fixed x, al_fixed y);
//void	play_view_sound(void);

void play_sound_list(void);
void clear_sound_list(void);
void reset_music(int area_index, int region_index, unsigned int rand_seed);
void turn_music_off(void);
void pause_music(void);
void unpause_music(void);

void check_sound_queue(void);
void *thread_check_sound_queue(ALLEGRO_THREAD *thread, void *arg);

enum
{
SAMPLE_BLIP1,
SAMPLE_BLIP2,
SAMPLE_BLIP3,
SAMPLE_BLIP4,
SAMPLE_KILL,
SAMPLE_CHIRP,
SAMPLE_OVER,
SAMPLE_ALLOC,
SAMPLE_NEW,

SAMPLE_BANG,
SAMPLE_BANG2,
SAMPLE_INT_UP,
SAMPLE_INT_BREAK,
SAMPLE_RESTORE,
SAMPLE_STREAM1,
SAMPLE_STREAM2,
SAMPLE_ZAP,
SAMPLE_SPIKE,
SAMPLE_SLICE,
SAMPLE_ULTRA,
SAMPLE_BUBBLE,
/*
Need sounds for:

build
zap?
stream
spike
repair
restore
raise interface
break interface
explode
transfer data?
start move?

hit?
hit interface?


*/

//SAMPLE_BLIP3,
//SAMPLE_BLIP4,
SAMPLE_DRUM1,
#define FIRST_DRUM_SAMPLE SAMPLE_DRUM1
SAMPLE_DRUM2,
SAMPLE_DRUM3,
SAMPLE_CLICK,
SAMPLE_THUMP,
#define DRUM_SAMPLES 5

//SAMPLE_BEAT2,
SAMPLES
};

enum
{
MSAMPLE_NOTE,
MSAMPLE_NOTE2,
MSAMPLE_NOTE3,
MSAMPLE_NOTE4,
MSAMPLES

};

enum
{
TONE_1C,
TONE_1CS,
TONE_1D,
TONE_1DS,
TONE_1E,
TONE_1F,
TONE_1FS,
TONE_1G,
TONE_1GS,
TONE_1A,
TONE_1AS,
TONE_1B,
TONE_2C,
TONE_2CS,
TONE_2D,
TONE_2DS,
TONE_2E,
TONE_2F,
TONE_2FS,
TONE_2G,
TONE_2GS,
TONE_2A,
TONE_2AS,
TONE_2B,
TONE_3C,
TONE_3CS,
TONE_3D,
TONE_3DS,
TONE_3E,
TONE_3F,
TONE_3FS,
TONE_3G,
TONE_3GS,
TONE_3A,
TONE_3AS,
TONE_3B,
TONE_4C,
TONE_4CS,
TONE_4D,
TONE_4DS,
TONE_4E,
TONE_4F,
TONE_4FS,
TONE_4G,
TONE_4GS,
TONE_4A,
TONE_4AS,
TONE_4B,
TONE_5C,
TONE_5CS,
TONE_5D,
TONE_5DS,
TONE_5E,
TONE_5F,
TONE_5FS,
TONE_5G,
TONE_5GS,
TONE_5A,
TONE_5AS,
TONE_5B,

TONES
};


#endif
