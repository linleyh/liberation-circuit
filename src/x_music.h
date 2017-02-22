

#ifndef H_X_MUSIC
#define H_X_MUSIC

void sthread_init_mustate(int play_piece);
int sthread_play_current_piece(void);

void init_camstate(int status, int area_index, int region_index, unsigned int rand_seed);
void sthread_init_sample_pointers(void);
void sthread_run_camstate(void);
//void sthread_create_samples_for_scale(int scale_note_type, int tone_offset);
unsigned int sthread_rand(unsigned int rand_max);

//#define SCALE_NOTE_TYPE 2
enum
{
	SCALE_TONE_0,
	SCALE_TONE_1,
	SCALE_TONE_2,
	SCALE_TONE_3,
	SCALE_TONE_4,
	SCALE_TONE_5,
	SCALE_TONE_6,
	SCALE_TONE_7,
	SCALE_TONE_8,
	SCALE_TONES,

};

#endif

