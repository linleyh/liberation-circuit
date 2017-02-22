#include <allegro5/allegro.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "g_header.h"
#include "g_misc.h"
#include "x_init.h"

extern ALLEGRO_EVENT_QUEUE* event_queue;
extern ALLEGRO_DISPLAY* display;
extern ALLEGRO_TIMER* timer;
extern ALLEGRO_TIMER* timer_1_second;


//void error_call(void);
//void wait_for_space(void);

#define IRAND_BUFFER_SIZE 1024


// irand (interface rand) is a random number generator to be used for all random number generation that doesn't affect the game state (so that the state of rand isn't affected)
//unsigned int irand_buffer [IRAND_BUFFER_SIZE]; // list of pseudorandom numbers - entropy doesn't really matter that much for irand as it's really just used for interface stuff
//int irand_pos; // position in the irand buffer

void init_random_numbers(int grand_seed)
{
// srand(grand_seed);
// TO DO: need to be able to init grand when starting a world - and should use saved seed if loading game from disk

// int i;

// for (i = 0; i < IRAND_BUFFER_SIZE; i ++)
// {
  //irand_buffer [i] = (int) ((int) rand() + ((int) rand() << 16)); // assumes that rand returns 16 bits
// }

}

// game rand - used for random numbers that may affect the game state
unsigned int grand(unsigned int max)
{

 return (rand() + (rand() << 16)) % max; // needs two calls to rand as rand returns a 16-bit number (I think)

}

unsigned int irand(unsigned int max)
{
/*
 irand_pos ++;

 if (irand_pos == IRAND_BUFFER_SIZE)
  irand_pos = 0;
*/
 return 1;//irand_buffer [irand_pos] % max; // irand_buffer is int so it only needs one value, not 2 as in grand

}


void error_call(void)
{

 ALLEGRO_EVENT ev;

 ALLEGRO_KEYBOARD_STATE error_key_State;

 fprintf(stdout, "\n\r\n\rPress space to exit (with game window as focus)");

 while(TRUE)
 {
  al_wait_for_event(event_queue, &ev);
  al_get_keyboard_state(&error_key_State);

  if(al_key_down(&error_key_State, ALLEGRO_KEY_ESCAPE)
  || al_key_down(&error_key_State, ALLEGRO_KEY_SPACE))
   safe_exit(1);
 };

}

void safe_exit(int exit_value)
{
fprintf(stdout, "\nStopping sound thread.");
 stop_sound_thread(); // will only stop the sound thread if it's been initialised
fprintf(stdout, "\nDestroying display.");

 if (display != NULL) // display is initialised to NULL right at the start
  al_destroy_display(display);
fprintf(stdout, "\nDestroying timer.");
 if (timer != NULL) // same
  al_destroy_timer(timer);

fpr("\nClosing down Allegro system.");
 al_uninstall_system();

fprintf(stdout, "\nExiting with value %i.", exit_value);
 exit(exit_value);

}

void wait_for_space(void)
{

 ALLEGRO_EVENT ev;

 ALLEGRO_KEYBOARD_STATE wait_key_State;

 fprintf(stdout, "\n     press space to continue\n\r");

 int unpressed = 0;

 while(TRUE)
 {
  al_wait_for_event(event_queue, &ev);
  al_get_keyboard_state(&wait_key_State);

  if(!al_key_down(&wait_key_State, ALLEGRO_KEY_SPACE))
   unpressed = 1;

  if(al_key_down(&wait_key_State, ALLEGRO_KEY_ESCAPE))
   safe_exit(0);

  if(unpressed && al_key_down(&wait_key_State, ALLEGRO_KEY_SPACE))
   return;
 };


}

void print_binary(int num)
{
 int i;

 for (i = 15; i > -1; i --)
 {
  if (num & (1 << i))
   fprintf(stdout, "1");
    else
     fprintf(stdout, "0");
 }

}


void print_binary8(int num)
{
 int i;

 for (i = 7; i > -1; i --)
 {
  if (num & (1 << i))
   fprintf(stdout, "1");
    else
     fprintf(stdout, "0");
 }

}


void print_binary32(int num)
{
 int i;

 for (i = 31; i > -1; i --)
 {
  if (num & (1 << i))
   fprintf(stdout, "1");
    else
     fprintf(stdout, "0");
 }

}





