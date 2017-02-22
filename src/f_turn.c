
#ifdef NOT_USED

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "i_console.h"
#include "t_template.h"

#include "e_log.h"
#include "e_editor.h"
#include "g_game.h"

#include "f_save.h"
#include "f_load.h"

// This file contains functions to load and save player and game turnfiles
// it uses some of the functions in f_save.c and f_load.c

/*

Here's how turnfiles will work:

First, someone creates a gamefile (.gam)
Each player receives the gamefile.
Each player loads the gamefile and sets up templates.
Each player saves a turnfile using a sysmenu button?? (actually probably a template button)
 - turnfile has free-form name, although the default extension is .tf (it's up to players to agree to naming conventions).
Players share turnfiles.
Each player can:
 - load the gamefile if turn 0, or otherwise load their saved game (which should have been saved at the end of the previous turn).
 - load in all players' turnfiles.
 - start the game, which will run identically for everyone.

This means that turnfiles just need to store:
 - the contents of each of the player's templates.
 - basic information about the game being played, to check consistency? not sure this is even needed.
I'm pretty sure that's all.

*/

extern ALLEGRO_DISPLAY* display; // used to display the native file dialog

extern struct save_state_struct save_state; // in f_save.c
//extern struct templstruct templ [TEMPLATES]; // see t_template.c
extern struct game_struct game;

//static int save_turnfile_to_file(int p);




// Call this function to save the contents of a player's templates to disk. p is player index.
// uses various save functions in f_save.c
// if an error occurs, it writes to the mlog then returns 0
int save_turnfile(int p)
{
/*
#ifdef SANITY_CHECK
// This shouldn't be possible, as the "save turnfile" button should only be available to valid players.
 if (p < 0 || p >= w.players)
 {
  fprintf(stdout, "\nError: f_turn.c: save_turnfile(): invalid player index %i (%i players in game)", p, w.players);
 }
#endif

 if (!open_save_file("Save turnfile", "*.tf"))
  return 0; // no file opened, so don't goto save_gamefile_fail

 save_state.bp = 0;
 save_state.error = 0;

 if (!save_turnfile_to_file(p))
  goto save_turnfile_fail;

 if (save_state.bp != 0) // means there is probably something left in the buffer
  write_save_buffer();

 flush_game_event_queues(); // may not be necessary

 close_save_file();

 if (save_state.error == 1)
  goto save_turnfile_fail;

 start_log_line(MLOG_COL_FILE);
 write_to_log("Turnfile for player ");
 write_number_to_log(p);
 write_to_log(" saved.");
 finish_log_line();
*/
 return 1; // success!
/*
save_turnfile_fail:
 start_log_line(MLOG_COL_ERROR); // A more relevant error message should already have been written.
 write_to_log("Turnfile for player ");
 write_number_to_log(p);
 write_to_log(" not saved.");
 finish_log_line();
 close_save_file();
 return 0;
*/
}

/*
// This function saves the contents of a player's templates to a turnfile
static int save_turnfile_to_file(int p)
{

 int t;

// There are probably smart ways of saving turnfiles, but I'm going for a lazy approach:
// This function will just save the player's index and the turn number, then save each template that belongs
// to the player (even empty ones).
// I think this should work.

 save_int(p); // these values are just saved for verification
 save_int(game.current_turn);

 t = 0;

 for (t = 0; t < TEMPLATES; t ++)
 {

  if (templ[t].type == TEMPL_TYPE_CLOSED)
   break; // finished
  if (templ[t].player != p
   || templ[t].fixed == 1)
   continue;
// Now we know we've found an open (although not necessarily loaded) template that belongs to this player. Save it.
  save_template(t, 0); // 0 means template initialisation stuff not saved (as this will come from gamefile or saved game file)
  if (save_state.error == 1)
   return 0;
 }

 if (save_state.error == 1)
  return 0;

 return 1;

}
*/



// Loading:

extern struct load_state_struct load_state;

//static int load_turnfile_from_file(int p);


// call this when load turnfile item selected from template menu
// opens native file dialogue and loads file
// writes to mlog on failure or error
int load_turnfile(int p)
{
/*
 if (!open_load_file("Load turnfile", "*.*"))
  return 0;

 load_state.bp = -1; // means first entry read will be 0
 load_state.error = 0;
 load_state.current_buffer_size = 0;

// load first buffer:
 if (!read_load_buffer())
  goto load_turnfile_fail;

 if (!load_turnfile_from_file(p))
  goto load_turnfile_fail;

 flush_game_event_queues(); // Probably not necessary

 close_load_file();

 start_log_line(MLOG_COL_FILE);
 write_to_log("Turnfile for player ");
 write_number_to_log(p);
 write_to_log(" loaded.");
 finish_log_line();
*/
 return 1; // success!!
/*

load_turnfile_fail:

  close_load_file();
//  reset_templates();
  start_log_line(MLOG_COL_ERROR); // A more relevant error message should already have been written.
  write_to_log("Turnfile for player ");
  write_number_to_log(p + 1);
  write_to_log(" not loaded.");
  finish_log_line();
  return 0;
*/

}

/*
static int load_turnfile_from_file(int p)
{

 int t;

 int player_index = 0, current_turn = 0;

 load_int(&player_index, 0, w.players - 1, "player index");
 if (load_state.error == 1)
  return 0; // may mean player_index completely out of bounds.
 if (player_index != p)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error: turnfile is for player ");
     write_number_to_log(player_index);
     write_to_log("; expected player ");
     write_number_to_log(p);
     write_to_log(".");
     finish_log_line();
     load_state.error = 1;
     return 0;
 }

 load_int(&current_turn, -1, 100000, "current turn");
 if (load_state.error == 1)
  return 0; // may mean current_turn completely out of bounds.
 if (current_turn != game.current_turn)
 {
     start_log_line(MLOG_COL_ERROR);
     write_to_log("Error: turnfile is for turn ");
     write_number_to_log(current_turn);
     write_to_log("; expected turn ");
     write_number_to_log(game.current_turn);
     write_to_log(".");
     finish_log_line();
     load_state.error = 1;
     return 0;
 }

// This assumes that the templates have already been set up
 for (t = 0; t < TEMPLATES; t ++)
 {
  if (templ[t].type == TEMPL_TYPE_CLOSED)
   break; // finished
  if (templ[t].player != p
   || templ[t].fixed == 1)
   continue;
// Now we know we've found an open (although not necessarily loaded) template that belongs to this player. Save it.
  load_template(t, 0); // 0 means template initialisation stuff not loaded (as the template has already been set up)
  set_opened_template(t, templ[t].type, 0);
  templ[t].contents.origin = TEMPL_ORIGIN_TURNFILE;

  if (load_state.error == 1)
   return 0;
 }

 if (load_state.error == 1)
  return 0;

 return 1;
}
*/

#endif
