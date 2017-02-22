
#ifdef S_TURNS

#include <allegro5/allegro.h>
//#include <allegro5/allegro_primitives.h>
//#include <allegro5/allegro_font.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "t_template.h"
#include "t_files.h"
#include "e_log.h"

#include "s_turn.h"

/*

This file contains functions to deal with the pregame and inter-turn phase.

During an interturn phase, the game is soft paused
A button in the system menu allows advancement to next turn.

Pregame is exactly the same
Before pregame, the system program can run silently (no display) until it triggers some kind of event (maybe there should be a limit on number of ticks). This lets it set things up for the first round.
Perhaps it can also cause the game to run.
During this time everything runs normally, except that input methods don't work

*/


extern struct game_struct game;
//extern struct templstruct templ [TEMPLATES]; // defined in t_template.c

void game_time_expired(void);
void start_interturn_phase(void);
//void setup_player_programs_from_templates(void);
void clear_player_program_templates(void);
int find_player_client_template(int p, int allow_operator);

// call this function each tick. It manages turn timer countdowns and similar, and can call start_interturn_phase and similar
// returns 1 if still in world phase, 0 otherwise
// note that this function is not called during soft pause (as turn timers don't count down during soft pause) or pregame
int run_turns(void)
{


// if game.phase is GAME_PHASE_OVER or GAME_PHASE_TURN we don't need to do anything; just wait for player to do something in sysmenu
//  (but actually this function shouldn't be called if not in GAME_PHASE_WORLD)

 if (game.phase != GAME_PHASE_WORLD)
  return 0;

// if (game.type == GAME_TYPE_INDEFINITE)
//  return 1;

 if (game.minutes_each_turn > 0)
 {
  game.current_turn_time_left --;

  if (game.current_turn_time_left <= 0)
  {
   game.current_turn_time_left = 0;
// turn over
   if (game.current_turn == game.turns)
   {
    game_time_expired();
    return 0; // no longer in world phase
   }
   start_interturn_phase();
   return 0; // no longer in world phase
  }
 }

 if (game.force_turn_finish == 1) // can be set by SY_MANAGE method
 {
   game.force_turn_finish = 0;
   game.current_turn_time_left = 0;
   if (game.current_turn == game.turns)
   {
    game_time_expired();
    return 0; // no longer in world phase
   }
   start_interturn_phase();
   return 0; // no longer in world phase
 }

 return 1; // still in world phase

}

// call this function once each tick during pregame
// should be called even if in soft pause (as soft pause affects procs but not system program)
// enters interturn phase when pregame_time_left runs out
void run_pregame(void)
{

//  fprintf(stdout, "\nPregame counter %i", game.pregame_time_left);
  game.pregame_time_left --;

// note that pregame_time_left can be set to zero by the SY_MANAGE method END_PREGAME command
  if (game.pregame_time_left <= 0)
  {
//   fprintf(stdout, "\nPregame finished.");
   game.pregame_time_left = 0;
   start_interturn_phase();
  }

}

void start_interturn_phase(void)
{

//   fprintf(stdout, "\nInterturn phase.");
 game.phase = GAME_PHASE_TURN;
 game.fast_forward = FAST_FORWARD_OFF;
 game.fast_forward_type = FAST_FORWARD_TYPE_SMOOTH;

}

void game_time_expired(void)
{

 game.phase = GAME_PHASE_OVER;
 game.game_over_status = GAME_END_TIME;

// if there is a particular win/loss status associated with running out of time (e.g. player 1 wins if time expires) the system program can set it just before the time runs out

}

// call this function when the player clicks on the next turn button in the sysmenu (increment_current_turn = 1)
//  or when starting from pregame (increment_current_turn = 0)
void start_new_turn(void)
{

//   fprintf(stdout, "\nStarting new turn.");

 game.current_turn ++;

 game.current_turn_time_left = game.minutes_each_turn * 2000;
 game.phase = GAME_PHASE_WORLD;
 game.fast_forward = FAST_FORWARD_OFF;
 game.fast_forward_type = FAST_FORWARD_TYPE_SMOOTH;

// The following things are no longer done automatically - the system program has control
// if player loaded a new program into a program template (client/operator/observer), need to copy it to the relevant programstruct before starting turn:
// setup_player_programs_from_templates();
// now clear all player program templates
// clear_player_program_templates();

}





/*
// Call this function at start of each turn, after interturn phase.
//  (don't call at start of pregame - only at start of turn immediately following pregame)
// It does the following:
//  - check each program template
//  - if loaded - copy to relevant programstruct
//  - if not loaded - ignore (and don't reset programstructs as it may already have something loaded in)
//  - because of this, it assumes that program templates are cleared just before interturn phase begins (otherwise it would automatically reset all player programs, which we don't want)
// It doesn't deal with the system program, which can only be loaded in right at the very start of the game.
// Also doesn't deal with process templates.
// If it encounters an error of some kind, it will write to mlog and continue.

*** no longer used - this is now up to the system program (MANAGE method)

void setup_player_programs_from_templates(void)
{

 int i;
 int t;

// if (w.allow_observer) - can assume that there will only be an observer template if the observer is allowed
 {
  for (t = 0; t < TEMPLATES; t ++)
  {
   if (templ[t].type == TEMPL_TYPE_OBSERVER)
   {
/ *    if (templ[i].loaded == 0)
    {
     if (templ[i].fixed == 0) // don't need to mention this if player can't load the observer program
      write_line_to_log("No observer program loaded.", MLOG_COL_TEMPLATE); // not an error
     break;
    }* /
    if (templ[t].loaded == 1
    && !copy_template_to_program(t, &w.observer_program, PROGRAM_TYPE_OBSERVER, -1)) // -1 is player (no player)
    {
     write_line_to_log("Warning: Couldn't use observer template.", MLOG_COL_ERROR);
//     return 0;
    }
    break;
   }
  }
 }
* /

// now deal with clients and operators

 int program_type = 0;

 for (i = 0; i < w.players; i ++)
 {
  switch(w.player[i].program_type_allowed)
  {
   case PLAYER_PROGRAM_ALLOWED_NONE:
    continue; // continue for i loop to next player
   case PLAYER_PROGRAM_ALLOWED_DELEGATE:
    t = find_player_client_template(i, 0); // 0 means operator not recognised
    program_type = PROGRAM_TYPE_DELEGATE;
    break;
   case PLAYER_PROGRAM_ALLOWED_OPERATOR:
    t = find_player_client_template(i, 1); // 0 means operator recognised (but client also recognised)
    program_type = PROGRAM_TYPE_OPERATOR;
    break;
   default: // should never happen
    t = -1;
    break;
  } // end switch
  if (t == -1)
  {
// this should not be possible
   write_line_to_log("Warning: player program template not found???", MLOG_COL_ERROR);
   continue;
  }
// now check that the template has anything in it:
     if (templ[t].loaded == 1)
     {
      if (!copy_template_to_program(t, &w.player[i].client_program, program_type, i)) // i is player
      {
       start_log_line(MLOG_COL_ERROR);
       write_to_log("Warning: couldn't use player ");
       write_number_to_log(i + 1);
       write_to_log("'s client program.");
       finish_log_line();
      }
     }
 } // end player i loop


 return;

}
*/

// returns template index for client template for player[p]
// if allow_operator==0 will not recognise an operator template (if ==1, will do so)
// returns -1 if no template found (shouldn't happen, but I suppose it's possible if a saved game file is corrupted)
int find_player_client_template(int p, int allow_operator)
{
/* int i;

 for (i = 0; i < TEMPLATES; i ++)
 {
  if (templ[i].player == p
   && templ[i].type == TEMPL_TYPE_DELEGATE)
    return i;

  if (templ[i].player == p
   && templ[i].type == TEMPL_TYPE_OPERATOR
   && allow_operator == 1)
    return i;
 }
*/
 return -1; // not found

}


// this just sets all client, operator and observer templates to unloaded
void clear_player_program_templates(void)
{
/*
 int t;

 for (t = 0; t < TEMPLATES; t ++)
 {
  if (templ[t].type == TEMPL_TYPE_DELEGATE
   || templ[t].type == TEMPL_TYPE_OPERATOR
   || templ[t].type == TEMPL_TYPE_OBSERVER)
    clear_template(t);
 }*/

}



#endif

