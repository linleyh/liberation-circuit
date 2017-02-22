#include <allegro5/allegro.h>
#include "allegro5/allegro_audio.h"
#include "allegro5/allegro_acodec.h"

#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

//#include "g_header.h"

#include "g_misc.h"

#include "x_sound.h"
#include "x_init.h"
#include "x_synth.h"
#include "x_music.h"



/*

How will the synth work?

- This file contains functions that accept a pointer to a float buffer (with a specified size) and fill the buffer with samples based on the current state of the synth struct.
- It's then up to the sound program to use the results.
- *hopefully* this shouldn't affect game performance as it's all run in a different thread.

- The synth struct contains:
 - base frequency
 - phase
 - length
 - an array of filters
  - each filter does something to the sample it's called for

*/

struct synthstruct synth;
//float synth_sample [SYNTH_SAMPLES] [SYNTH_SAMPLE_MAX_SIZE];


/*

New structure:

void init_waveform(float* buffer, int buffer_length);
void add_waveform_sine(float* buffer, int buffer_length, float freq, float phase);
void add_waveform_square(float* buffer, int buffer_length, float freq, float phase);
void apply_linear_attack(float* buffer, int buffer_length, int attack_length);
void apply_linear_decay(float* buffer, int buffer_length, int attack_length);

ALLEGRO_SAMPLE* create_sample(float* buffer, int buffer_length)


*/


void init_waveform(float* buffer, int buffer_length);
void set_waveform_sine(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);
void set_waveform_square(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp);
void apply_linear_attack(float* buffer, int buffer_length, float attack_length);
void apply_linear_decay(float* buffer, int buffer_length, float decay_length);
void amplify(float* buffer, int buffer_length, float amplification);
ALLEGRO_SAMPLE* finish_sample(float* buffer, int buffer_length);

void init_waveform(float* buffer, int buffer_length)
{

	if (buffer_length > SYNTH_SAMPLE_MAX_SIZE)
	{
		fpr("\n Error: x_synth.c: init_waveform(): buffer too large (%i)", buffer_length);
		error_call();
	}

	int i;

	for (i = 0; i < buffer_length; i ++)
	{
		buffer [i] = 0;
	}

}

unsigned int rand_seed;


float harmonic_factor [4] = {1.0, 2.0, 4.0, 8.0};

void set_waveform_sine(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp)
{

 const float dt = 1.0 / SAMPLE_FREQUENCY;
 float ti;
 float angle;
 int i;

 switch(harmonics)
 {
	 case 0:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  buffer [i] = sin(TWOPI * freq * ti) * amp;
   }
		 break; // end harmonics 0
	 case 1:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = ti * TWOPI * freq;
 	  buffer [i] = sin(angle) * amp
               + sin(angle * harmonic_factor [1]) * amp;
   }
		 break; // end harmonics 0
	 case 2:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = ti * TWOPI * freq;
 	  buffer [i] = sin(angle) * amp
               + sin(angle * harmonic_factor [1]) * amp
               + sin(angle * harmonic_factor [2]) * amp;
   }
		 break; // end harmonics 0

 }

/*
 for (i = 0; i < buffer_length; i ++)
 {

 	ti = phase + i * dt;
 	vibr_amount = i * 0.000002;
 	for (j = 0; j < harmonics + 1; j ++)
		{
   if (j == 0)
 	  buffer [i] += sin(TWOPI * freq * ti * harmonic_factor [j]) * amp;
 	   else
   	  buffer [i] += sin(TWOPI * freq * ti * harmonic_factor [j] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp;
		}

 }
*/

//low_pass_filter(buffer, buffer_length, 0.025);


}


void set_waveform_sine_vib(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp)
{

 const float dt = 1.0 / SAMPLE_FREQUENCY;
 float ti;
 float angle;
 int i;

 float vibr_speed = 0.000125;//0.000125;
 float vibr_amount = 0.004;
 float vibr_phase = sthread_rand(1000) * 0.002 * PI;

 switch(harmonics)
 {
	 case 0:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  vibr_amount = i * 0.000002;
 	  buffer [i] = sin(TWOPI * freq * ti * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp;
   }
		 break; // end harmonics 0
	 case 1:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = ti * TWOPI * freq;
 	  vibr_amount = i * 0.000002;
 	  buffer [i] = sin(angle) * amp
               + sin(angle * harmonic_factor [1] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp;
   }
		 break; // end harmonics 0
	 case 2:
		 for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = ti * TWOPI * freq;
 	  vibr_amount = i * 0.000002;
 	  buffer [i] = sin(angle) * amp
               + sin(angle * harmonic_factor [1] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp
               + sin(angle * harmonic_factor [2] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp;
   }
		 break; // end harmonics 0

 }

/*
 for (i = 0; i < buffer_length; i ++)
 {

 	ti = phase + i * dt;
 	vibr_amount = i * 0.000002;
 	for (j = 0; j < harmonics + 1; j ++)
		{
   if (j == 0)
 	  buffer [i] += sin(TWOPI * freq * ti * harmonic_factor [j]) * amp;
 	   else
   	  buffer [i] += sin(TWOPI * freq * ti * harmonic_factor [j] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) * amp;
		}

 }
*/

//low_pass_filter(buffer, buffer_length, 0.025);


}



void set_waveform_noise(float* buffer, int buffer_length, int freq, int harmonics, float amp)
{

// const float dt = 1.0 / SAMPLE_FREQUENCY;
// float ti;
 int i;

 int square_length = 100;
 int square_sign = 1;


 for (i = 0; i < buffer_length; i ++)
 {
 	//ti = 0 + i * dt;
// 	 buffer [i] += sin(TWOPI * freq * ti) * amp;
// 	for (j = 0; j < harmonics + 1; j ++)
		{
 	 if (square_length <= 0)
			{
				square_sign *= -1;
				square_length = 6 + 0 + sthread_rand(3);//(freq + sthread_rand(freq));// * 0.3;
			}
			square_length --;
 	 buffer [i] = square_sign * amp;
		}
//		if ((i / 400) % 2 == 0)
//			buffer [i] += 0.2;
//		  else
//			buffer [i] -= 0.2;
 }


//low_pass_filter(buffer, buffer_length, 0.5);


}



void low_pass_filter(float* buffer, int buffer_length, float lpf_beta)
{
    int i;

    float filtered_data = 0;

//    lpf_beta = 0.3;

    for (i = 0; i < buffer_length; i ++)
				{

//       filtered_data = filtered_data - (lpf_beta * (filtered_data - buffer [i]));
       filtered_data = filtered_data - (lpf_beta * (filtered_data - buffer [i]));
       buffer [i] = filtered_data;

    }
}



void set_waveform_square(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp)
{


 const float dt = 1.0 / SAMPLE_FREQUENCY;
 float ti;
 int i;
// float value;
 float angle;

// amp /= (float) (harmonics + 1);

 float harmonic_amp [5];

 harmonic_amp [0] = amp;
 harmonic_amp [1] = amp * 0.5;
 harmonic_amp [2] = amp * 0.25;
 harmonic_amp [3] = amp * 0.125;
 harmonic_amp [4] = amp * 0.0625;


// float vibr_speed = 0.00125;//0.0005;
// float vibr_amount = 0.004;
// float vibr_phase = sthread_rand(1000) * 0.002 * PI;

 switch(harmonics)
 {
 	case 0:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
//	  value = sin(TWOPI * freq * ti * harmonic_factor [j]); // THERE MUST BE A BETTER WAY TO DO THIS
 	  if (sin(TWOPI * freq * ti) < 0)
			  buffer [i] = amp;//harmonic_amp [j];
		    else
  	 		 buffer [i] = 0 - amp;//0 - harmonic_amp [j];
   }
  	break;
 	case 1:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = TWOPI * freq * ti;
//	   value = sin(TWOPI * freq * ti * harmonic_factor [j]); // THERE MUST BE A BETTER WAY TO DO THIS
 	  if (sin(angle) < 0)
			  buffer [i] = amp;
		    else
  	 		 buffer [i] = 0 - amp;
 	  if (sin(angle * harmonic_factor [1]) < 0)
			  buffer [i] += harmonic_amp [1];
		    else
  	 		 buffer [i] -= harmonic_amp [1];
   }
  	break;
 	case 2:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = TWOPI * freq * ti;
//	   value = sin(TWOPI * freq * ti * harmonic_factor [j]); // THERE MUST BE A BETTER WAY TO DO THIS
 	  if (sin(angle) < 0)
			  buffer [i] = amp;
		    else
  	 		 buffer [i] = 0 - amp;
 	  if (sin(angle * harmonic_factor [1]) < 0)
			  buffer [i] += harmonic_amp [1];
		    else
  	 		 buffer [i] -= harmonic_amp [1];
 	  if (sin(angle * harmonic_factor [2]) < 0)
			  buffer [i] += harmonic_amp [2];
		    else
  	 		 buffer [i] -= harmonic_amp [2];
   }
  	break;



 }
/*
 for (i = 0; i < buffer_length; i ++)
 {

	 ti = phase + i * dt;
 	vibr_amount = i * 0.000002;

  for (j = 0; j < harmonics + 1; j ++)
		{
			if (j == 0 && harmonics > 0)
 	  value = sin(TWOPI * freq * ti * harmonic_factor [j]); // THERE MUST BE A BETTER WAY TO DO THIS
 	   else
   	  value = sin(TWOPI * freq * ti * harmonic_factor [j] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)); // THERE MUST BE A BETTER WAY TO DO THIS

 	 if (value < 0)
			 buffer [i] += harmonic_amp [j];
		   else
  			 buffer [i] -= harmonic_amp [j];

		}
*/
/*

 	ti = phase + i * dt;
 	buffer [i] = sin(TWOPI * freq * ti) * amp;

 	if (buffer[i] < 0)
			ti = 0.5;
		  else
  			ti = -0.5;
*/


/*
 	if ((int) (((i * 16) / freq)) % 2)
			ti = 0.5;
		  else
					ti = -0.5;*/
// 	ti = phase + i * dt;
// 	buffer [i] = ti * amp;//sin(TWOPI * freq * ti);
// }

//low_pass_filter(buffer, buffer_length, 0.025);

}




void set_waveform_square_vib(float* buffer, int buffer_length, float freq, int harmonics, float phase, float amp)
{

 const float dt = 1.0 / SAMPLE_FREQUENCY;
 float ti;
 int i;
// float value;
 float angle;

// amp /= (float) (harmonics + 1);

 float harmonic_amp [5];

 harmonic_amp [0] = amp;
 harmonic_amp [1] = amp * 0.5;
 harmonic_amp [2] = amp * 0.25;
 harmonic_amp [3] = amp * 0.125;
 harmonic_amp [4] = amp * 0.0625;


 float vibr_speed = 0.00125;//0.0005;
 float vibr_amount = 0.004;
 float vibr_phase = sthread_rand(1000) * 0.002 * PI;

 switch(harmonics)
 {
 	case 0:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  if (sin(TWOPI * freq * ti * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) < 0)
			  buffer [i] = amp;//harmonic_amp [j];
		    else
  	 		 buffer [i] = 0 - amp;//0 - harmonic_amp [j];
   }
  	break;
 	case 1:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = TWOPI * freq * ti;
 	  if (sin(angle) < 0)
			  buffer [i] = amp;
		    else
  	 		 buffer [i] = 0 - amp;
 	  if (sin(angle * harmonic_factor [1] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) < 0)
			  buffer [i] += harmonic_amp [1];
		    else
  	 		 buffer [i] -= harmonic_amp [1];
   }
  	break;
 	case 2:
   for (i = 0; i < buffer_length; i ++)
   {
 	  ti = phase + i * dt;
 	  angle = TWOPI * freq * ti;
 	  if (sin(angle) < 0)
			  buffer [i] = amp;
		    else
  	 		 buffer [i] = 0 - amp;
 	  if (sin(angle * harmonic_factor [1] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) < 0)
			  buffer [i] += harmonic_amp [1];
		    else
  	 		 buffer [i] -= harmonic_amp [1];
 	  if (sin(angle * harmonic_factor [2] * (1.0 + sin(vibr_phase + i * vibr_speed) * vibr_amount)) < 0)
			  buffer [i] += harmonic_amp [2];
		    else
  	 		 buffer [i] -= harmonic_amp [2];
   }
  	break;




 }


}




void apply_linear_attack(float* buffer, int buffer_length, float attack_length)
{

 if (attack_length > buffer_length)
		attack_length = buffer_length; // probably pointless

 int i;

	for (i = 0; i < attack_length; i ++)
	{
	 buffer [i] *= i / attack_length;
 }


}

void apply_linear_decay(float* buffer, int buffer_length, float decay_length)
{

 if (decay_length > buffer_length)
		decay_length = buffer_length; // probably pointless

 int i;

	for (i = buffer_length - decay_length; i < buffer_length; i ++)
	{
	 buffer [i] *= (buffer_length - i) / decay_length;
 }


}


void apply_adsr(float* buffer, int buffer_length,
												int attack_length,
												int decay_length, float sustain_amp, // decay length, target amplification after decay
												int release_length) // release length (counted back from end)
{

 int i;

	for (i = 0; i < attack_length; i ++)
	{
	 buffer [i] *= (float) i / (float) attack_length;
 }

 float decay_proportion = (1 - sustain_amp) / decay_length;

	for (i = 0; i < decay_length; i ++)
	{
		buffer [attack_length + i] *= 1 - ((float) i * decay_proportion);
	}

	for (i = attack_length + decay_length; i < buffer_length - release_length; i ++)
	{
		buffer [i] *= sustain_amp;
	}

	for (i = buffer_length - release_length; i < buffer_length; i ++)
	{
	 buffer [i] *= ((buffer_length - i) / (float) release_length) * sustain_amp;
 }



}


void add_waveforms(float* target_buffer, float* added_buffer, int buffer_length)
{
	int i;

	for (i = 0; i < buffer_length; i ++)
	{
		target_buffer [i] += added_buffer [i];
	}

}

void amplify(float* buffer, int buffer_length, float amplification)
{

	int i;

 for (i = 0; i < buffer_length; i ++)
 {
 	buffer [i] *= amplification;
 }

}

ALLEGRO_SAMPLE* finish_sample(float* buffer, int buffer_length)
{


 ALLEGRO_SAMPLE* new_sample = al_create_sample(buffer, buffer_length, SAMPLE_FREQUENCY, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_1, 0);

 if (new_sample == NULL)
	{
		  fprintf(stdout, "\nError: x_synth.c: finish_sample(): failed to create sample.");
    error_call();
	}

 return new_sample;

}















void set_synth(int base_type, int length, int linear_attack, int linear_decay, int base_freq, int phase)
{

 synth.base_type = base_type;
 synth.length = length;
 synth.linear_attack = linear_attack;
 synth.linear_decay = linear_decay;
 synth.base_freq = base_freq;
 synth.phase = phase;
 synth.filters = 0;	// can be added later

}

int add_synth_filter(int type, float base_freq, float phase)
{

	if (synth.filters >= FILTERS)
		return 0;

	synth.filter[synth.filters].type = type;
//	synth.filter[synth.filters].start_time = start_time; - not currently used
//	synth.filter[synth.filters].end_time = end_time;
	synth.filter[synth.filters].base_freq = base_freq;
	synth.filter[synth.filters].phase = phase;

	synth.filters++;

	return 1;

}

/*

ALLEGRO_SAMPLE* synthesise(int ssi)
{

#ifdef SANITY_CHECK
 if (synth.length >= SYNTH_SAMPLE_MAX_SIZE)
	{
		fprintf(stdout, "\nError: x_synth.c: synthesise(): synth.length too high (%i; max is %i)", synth.length, SYNTH_SAMPLE_MAX_SIZE);
		error_call();
	}
	if (ssi >= SYNTH_SAMPLES)
	{
		fprintf(stdout, "\nError: x_synth.c: synthesise(): synth_sample index too high (ssi %i; max is %i)", ssi, SYNTH_SAMPLES);
		error_call();
	}
#endif

 const float dt = 1.0 / SAMPLE_FREQUENCY;
// float t = 0;
 float ti;//, tj;
 int i, j;
 float total_amplitude = 1;

 for (i = 0; i < synth.length; i ++)
 {
 	ti = synth.phase + i * dt;
 	synth_sample [ssi] [i] = sin(TWOPI * synth.base_freq * ti + 0);
 }

		for (j = 0; j < synth.filters; j ++)
		{
			switch(synth.filter[j].type)
			{
			 case FILTER_SINE_ADD:
			 	total_amplitude += 0.5;
			 	for (i = 0; i < synth.length; i ++)
					{
   	  ti = synth.filter[j].phase + i * dt;
    	 synth_sample [ssi] [i] += sin(TWOPI * synth.filter[j].base_freq * ti + 0);
					}
			 	break;
			 case FILTER_FLANGER: // doesn't work at the moment
			 	total_amplitude += 0.1;
			 	int delay_time;
			 	for (i = 0; i < synth.length; i ++)
					{
						delay_time = ((sin((float) i * dt * TWOPI * 50)) * (float) synth.filter[j].start_time) + synth.filter[j].start_time;
 			 	if (i > delay_time)
	 					synth_sample [ssi] [i] += synth_sample [ssi] [i - delay_time];
					}
				 break;
			}

		}

 	if (synth.linear_attack > 0)
		{
			for (i = 0; i < synth.linear_attack; i ++)
			{
			 synth_sample [ssi] [i] *= i / synth.linear_attack;
		 }
		}

		if (synth.linear_decay > 0)
		{
			for (i = synth.length - synth.linear_decay; i < synth.length; i ++)
			{
			 synth_sample [ssi] [i] *= (synth.length - i) / synth.linear_decay;
			}
		}

		for (i = 0; i < synth.length; i ++)
		{
		 synth_sample [ssi] [i] /= (total_amplitude * 5);
		}

// finally, create the sample

 ALLEGRO_SAMPLE* temp_sample = al_create_sample(synth_sample [ssi], synth.length, SAMPLE_FREQUENCY, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_1, 0);

 if (temp_sample == NULL)
	{
		  fprintf(stdout, "\nError: x_synth.c: synthesise(): failed to create sample.");
    error_call();
	}

 return temp_sample;

}
*/






