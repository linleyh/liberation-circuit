
#ifndef H_X_SYNTH
#define H_X_SYNTH

#define TWOPI                 (2.0 * PI)
//#define SAMPLES_PER_BUFFER    (1024)
//#define SAMPLE_FREQUENCY      (44100)
#define SAMPLE_FREQUENCY      (22050)


#define FILTERS 8

enum
{
SYNTH_BASE_SINE,
SYNTH_BASE_SQUARE,
};

enum
{
FILTER_SINE_ADD,
FILTER_FLANGER
//FILTER_ATTACK_LINEAR,
//FILTER_DECAY_LINEAR
};

struct filterstruct
{
	int type;
	int start_time;
	int end_time;
	float base_freq;
	float phase;
};

struct synthstruct
{
 int base_type;
 int length;
 float linear_attack; // can be zero if a more complex filter attack/decay is being used
 float linear_decay;
 float base_freq;
 float phase;
 int filters;
 struct filterstruct filter [FILTERS];

};

void set_synth(int base_type, int length, int linear_attack, int linear_decay, int base_freq, int phase);
int add_synth_filter(int type, float base_freq, float phase);
ALLEGRO_SAMPLE* synthesise(int ssi);
#define SYNTH_SAMPLES 8
#define SYNTH_SAMPLE_MAX_SIZE 10001

extern struct synthstruct synth;
//extern float synth_sample [SYNTH_SAMPLES] [SYNTH_SAMPLE_MAX_SIZE];




void init_waveform(float* buffer, int buffer_length);
void set_waveform_sine(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);
void set_waveform_square(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);
void set_waveform_noise(float* buffer, int buffer_length, int freq, int harmonics, float amp);
void set_waveform_sine_vib(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);
void set_waveform_square_vib(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);

void apply_linear_attack(float* buffer, int buffer_length, float attack_length);
void apply_linear_decay(float* buffer, int buffer_length, float decay_length);
void amplify(float* buffer, int buffer_length, float amplification);
ALLEGRO_SAMPLE* finish_sample(float* buffer, int buffer_length);
void add_waveforms(float* target_buffer, float* added_buffer, int buffer_length);

void apply_adsr(float* buffer, int buffer_length,
												int attack_length,
												int decay_length, float sustain_amp, // decay length, target amplification after decay
												int release_length); // release length (counted back from end)



#endif
