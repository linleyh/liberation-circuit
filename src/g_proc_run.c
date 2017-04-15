
#include <allegro5/allegro.h>

#include <stdio.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "g_header.h"

#include "g_group.h"
#include "g_motion.h"
#include "g_method.h"
#include "g_proc.h"

#include "g_method_pr.h"
#include "g_method_std.h"

#include "v_interp.h"
#include "v_draw_panel.h"
#include "x_sound.h"

extern struct template_struct templ [PLAYERS] [TEMPLATES_PER_PLAYER];
extern struct game_struct game;
extern struct view_struct view;
extern struct bcode_panel_state_struct bcp_state;

static void	place_under_attack_marker(al_fixed marker_x, al_fixed marker_y);

// resume_loop_after_watch_with_core should be -1 if we're not resuming after watching
void run_cores_and_procs(int resume_loop_after_watch_with_core)
{

 int first_core;

 if (resume_loop_after_watch_with_core == -1)
	{
  run_motion();
  first_core = 0;
	}
	 else
			first_core = resume_loop_after_watch_with_core;

//fpr("\n %i: %i", resume_loop_after_watch_with_core, first_core);

 int c;
 struct core_struct* core;

 for (c = first_core; c < w.max_cores; c ++)
	{

  if (w.core[c].exists == 0)
   continue;

  core = &w.core[c];

// run code:

  w.debug_mode = w.debug_mode_general; // 0 or 1

  if (core->next_execution_timestamp == w.world_time)
  {

  	if (c != resume_loop_after_watch_with_core)
			{
// none of the following code needs to run if we're resuming after pausing to watch a process execute:
//  (however, some of the later code does need to run)
    if (core->player_index == game.user_player_index
			 	&& core->damage_this_cycle > 0)
			 {
			 	place_under_attack_marker(core->core_position.x, core->core_position.y);
			 }

			 core->power_left = core->power_capacity;
			 core->power_use_excess = 0;

    run_objects_before_execution(core);

    core->last_execution_timestamp = w.world_time;
    core->next_execution_timestamp = w.world_time + EXECUTION_COUNT;
    core->cycles_executed ++;

    if (game.watching == WATCH_ON
				 && bcp_state.watch_core_index == c
				 && bcp_state.watch_core_timestamp == core->created_timestamp)
			 {
 				game.watching = WATCH_PAUSED_TO_WATCH;
// 				game.pause_watch = 1;
     init_bcode_execution_for_watch(core, &templ[core->player_index][core->template_index].bcode, core->memory);
     return;
			 }
 			 else
      execute_bcode(core, &templ[core->player_index][core->template_index].bcode, core->memory);
			} // end if (c != first_core)


   if (core->self_destruct)
			{
				core_proc_explodes(&w.proc[core->process_index], core->player_index);
				continue;
			}

   core->messages_received = 0;
   core->message_reading = -1; // starts at -1 because next_message() increments it. If this is >= core->messages_received, core has finished reading messages
   core->message_position = 0;

   core->contact_core_index = -1;
   core->damage_this_cycle = 0;
   core->damage_source_core_index = -1;

   run_objects_after_execution(core);
  }

  run_objects_each_tick(core);

	} // end of for c loop

/*
 for (p = 0; p < w.max_procs; p ++)
 {

  if (w.proc[p].exists == 0)
   continue;

  proc = &w.proc[p];
*/
/*
// if the proc has just been destroyed, it may still be deallocating (if so exists will be -1). A deallocating proc doesn't do anything:
  if (proc->exists == -1)
  {
   if (proc->destroyed_timestamp + DEALLOCATE_COUNTER < w.world_time)
   {
    proc->exists = 0;
    w.player[proc->player_index].processes --;
   }
   continue;
  }*/

// active object code here!
//   active_method_pass_after_execution(pr); // currently this is called immediately after each execution. If this is ever changed so that it runs after all procs have executed, may need to change the way some things work (like LISTEN)


// } // end of for p loop



// active_method_pass_each_tick(); // deals with methods that do stuff even when the proc isn't executing (e.g. acceleration, which accelerates for a certain duration)


}

static void	place_under_attack_marker(al_fixed marker_x, al_fixed marker_y)
{

	int marker_index = 0;

	if (view.under_attack_marker_last_time >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
	{
// first make sure there isn't already one here:
  int i;
  for (i = 0; i < UNDER_ATTACK_MARKERS; i ++)
		{
			if (view.under_attack_marker [i].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION
			 && abs(marker_x - view.under_attack_marker[i].position.x) < al_itofix(2000)
				&& abs(marker_y - view.under_attack_marker[i].position.y) < al_itofix(2000))
					return;
		}
		while(view.under_attack_marker [marker_index].time_placed_world >= w.world_time - UNDER_ATTACK_MARKER_DURATION)
		{
			marker_index ++;
		}
		if (marker_index >= UNDER_ATTACK_MARKERS)
			return; // already too many
	}

	view.under_attack_marker[marker_index].time_placed_world = w.world_time;
	view.under_attack_marker_last_time = w.world_time;
	view.under_attack_marker[marker_index].position.x = marker_x;
	view.under_attack_marker[marker_index].position.y = marker_y;

 play_interface_sound(SAMPLE_BLIP4, TONE_1A);

}

