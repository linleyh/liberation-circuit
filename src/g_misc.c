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
extern struct settingsstruct settings;

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


/*

FILE* open_file_from_standard_path(const char* basic_file_name, const char* mode, int path_type)
{

#define TEMP_PATH_LENGTH 300

 char temp_path [TEMP_PATH_LENGTH] = "";

 switch(path_type)
 {
 	case PATH_TYPE_DATA:
 		if (settings.option [OPTION_STANDARD_PATHS] == STANDARD_PATHS_EXECUTABLE)
				strcpy(temp_path, settings.path_to_executable);
   strncat(temp_path, DIR_DATA, TEMP_PATH_LENGTH - 100);
   break;
  case PATH_TYPE_MAIN_DIRECTORY:
			strcpy(temp_path, settings.path_to_executable);
			break;
 	case PATH_TYPE_USER:
 		if (settings.option [OPTION_STANDARD_PATHS] == STANDARD_PATHS_EXECUTABLE)
				strcpy(temp_path, settings.path_to_executable);
   strncat(temp_path, DIR_USER, TEMP_PATH_LENGTH - 100);
   break;
  case PATH_TYPE_STORY:
 		if (settings.option [OPTION_STANDARD_PATHS] == STANDARD_PATHS_EXECUTABLE)
				strcpy(temp_path, settings.path_to_executable);
   strncat(temp_path, DIR_STORY, TEMP_PATH_LENGTH - 100);
   break;
  default: // PATH_TYPE_COMPLETE
			break; // shouldn't need to change anything
 }

 strcat(temp_path, basic_file_name);

	return fopen(temp_path, mode);

}
*/


// standard paths not yet implemented
void init_standard_paths(void)
{


	settings.path_to_executable [0] = 0;

 if (settings.option [OPTION_STANDARD_PATHS] == STANDARD_PATHS_EXECUTABLE)
	{
// Unfortunately there does not seem to be any simple way to find the execution directory.
// We can only get the full path of the executable, including the file name.
// So we need to remove the file name from the end of the path:

  ALLEGRO_PATH* executable_path;
  executable_path = al_get_standard_path(ALLEGRO_EXENAME_PATH);

  if (executable_path == NULL)
		{
// may still be okay...
			fpr("\nFailed to get executable path. Attempting to run using relative paths...");
			return;
		}

  char filename [100]; // 100 should be plenty of room
  strncpy(filename, al_get_path_filename(executable_path), 95); // 95 should too
  int filename_length = strlen(filename);

//  const char* temp_path = al_path_cstr(executable_path, ALLEGRO_NATIVE_PATH_SEP); // this pointer should be valid until the path is modified
//  int temp_path_length = strlen(temp_path);
  char file_path [FILE_PATH_LENGTH];
  strncpy(file_path, al_path_cstr(executable_path, ALLEGRO_NATIVE_PATH_SEP), FILE_PATH_LENGTH - 5);
  int file_path_length = strlen(file_path);
  if (file_path_length >= FILE_PATH_LENGTH - 10)
  {
	  fpr("\nSorry, your file path (%s) is too long (%i characters; the maximum is %i).", file_path, file_path_length, FILE_PATH_LENGTH - 10);
			fpr("\nAttempting to run using relative paths...");
			return;
  }
  file_path [file_path_length - filename_length] = 0;

		strcpy(settings.path_to_executable, file_path);

  al_destroy_path(executable_path);

		return;
	}

 if (settings.option [OPTION_STANDARD_PATHS] == STANDARD_PATHS_VARIOUS)
	{



/*
  ALLEGRO_PATH* standard_path;

// PATH_TYPE_RESOURCES:
  standard_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);

  if (standard_path == NULL)
// may still be okay...
			fpr("\nFailed to get resources path. Attempting to run using relative path...");
			 else
				{
					  char temp_path [FILE_PATH_LENGTH];
       strncpy(temp_path, al_path_cstr(standard_path, ALLEGRO_NATIVE_PATH_SEP), FILE_PATH_LENGTH - 10);
       int temp_path_length = strlen(temp_path);
       if (temp_path_length >= FILE_PATH_LENGTH - 20)
							{
									  fpr("\nSorry, your resources path (%s) is too long (maximum %i).", temp_path, FILE_PATH_LENGTH - 20);
			        fpr("\nAttempting to run using relative path...");
							}
							 else
								{
									strcpy(settings.standard_path [PATH_TYPE_RESOURCES], temp_path;
								}
      al_destroy_path(standard_path);
				}
*/
	}




// settings.option [OPTION_STANDARD_PATHS] is probably STANDARD_PATHS_NONE


		return;

}
