
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "e_header.h"
#include "e_editor.h"
#include "e_slider.h"
#include "m_globvars.h"
#include "m_input.h"
#include "i_input.h"
#include "i_header.h"
#include "e_log.h"
#include "e_help.h"
#include "e_inter.h"
#include "i_buttons.h"

#include "g_misc.h"

#include "f_load.h"
#include "f_save.h"
#include "s_turn.h"

#include "p_panels.h"

extern struct game_struct game;
extern struct view_struct view;


enum
{
//SYSMENU_NEXT_TURN,
SYSMENU_PAUSE,
//SYSMENU_HALT,
//SYSMENU_FF, - not currently supported as it's difficult to turn off once started. Use function keys instead.
//SYSMENU_SAVE,
//SYSMENU_LOAD, - I've removed loading from the sysmenu. Should I put it back in? Not sure.
SYSMENU_QUIT,
SYSMENU_QUIT_CONFIRM, // displayed only after player clicks Quit button
SYSMENU_BUTTONS
};

struct sysmenustatestruct
{

  ALLEGRO_BITMAP* bmp;
//  int panel_x, panel_y;
//  int button_highlight;
  int quit_confirm;

//  ALLEGRO_BITMAP* log_sub_bmp;

};

struct sysmenustatestruct sysmstate;

/*
#define SYSMENU_X 100
#define SYSMENU_Y 100
#define SYSMENU_H 50
#define SYSMENU_W 300
*/
/*
struct sysmenu_buttonstruct
{
 int x1, y1, x2, y2;
 int highlight;
};

struct sysmenu_buttonstruct sysmenu_button [SYSMENU_BUTTONS];*/

extern struct editorstruct editor;
extern struct fontstruct font [FONTS];
extern ALLEGRO_DISPLAY* display;

extern struct log_struct mlog; // in e_log.c


int sysmenu_input(void);


void init_sysmenu(void)
{


// sysmstate.bmp = editor.sub_bmp;
// sysmstate.log_sub_bmp = editor.log_sub_bmp;

// sysmstate.button_highlight = -1;
// sysmstate.panel_x = settings.editor_x_split;
// sysmstate.panel_y = 0;
 sysmstate.quit_confirm = 0;
/*
 int i;

 for (i = 0; i < SYSMENU_BUTTONS; i ++)
 {
  sysmenu_button[i].x1 = SYSMENU_X;
  sysmenu_button[i].x2 = sysmenu_button[i].x1 + SYSMENU_W;
  sysmenu_button[i].y1 = SYSMENU_Y + (SYSMENU_H * i);
  sysmenu_button[i].y2 = sysmenu_button[i].y1 + SYSMENU_H - 3;

 }

*/
}

void open_sysmenu(void)
{

// settings.edit_window = EDIT_WINDOW_SYSMENU;
 inter.panel_input_capture = PANEL_SYSMENU;

 sysmstate.quit_confirm = 0;

}


void display_sysmenu(void)
{
#ifdef USING_SYSMENU
// al_set_target_bitmap(sysmstate.bmp);
// al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(editor.panel_x, editor.panel_y, editor.panel_w, editor.panel_h);

 al_clear_to_color(colours.base [COL_BLUE] [SHADE_MIN]);

 reset_i_buttons();

 al_draw_textf(font[FONT_SQUARE_BOLD].fnt, colours.base [COL_GREY] [SHADE_MAX], 5, 2, ALLEGRO_ALIGN_LEFT, "System Menu");

 int i;

#define BUTTON_TEXT_LENGTH 30
 char button_text [BUTTON_TEXT_LENGTH] = "";
 int button_colour;
 int button_base_shade; // is increased by 1 if button highlighted, so don't set this to SHADE_MAX
 int button_selectable; // note that setting this to 0 just means the button doesn't look selectable
 int hide_button; // button completely hidden

// sysmenu buttons are a bit complicated because their appearance depends on various game state things
 for (i = 0; i < SYSMENU_BUTTONS; i ++)
 {
  button_colour = COL_BLUE;
  button_base_shade = SHADE_LOW;
  button_selectable = 1;
  hide_button = 0;

  switch(i)
  {
/*   case SYSMENU_NEXT_TURN:
    switch(game.phase)
    {
     case GAME_PHASE_TURN:
      if (game.current_turn == 0)
       strcpy(button_text, "Start execution!");
        else
         strcpy(button_text, "Next turn");
      button_colour = COL_GREEN;
      break;
     case GAME_PHASE_OVER:
      strcpy(button_text, "Game over");
//      button_colour = COL_GREEN;
      button_base_shade = SHADE_MIN;
      button_selectable = 0;
      break;
     default:
      strcpy(button_text, "Executing...");
      if (game.pause_soft)
       strcpy(button_text, "Partial execution");
      if (game.pause_hard)
       strcpy(button_text, "Halted");
      button_base_shade = SHADE_MIN;
      button_selectable = 0;
      break;
    }
    break;*/
   case SYSMENU_PAUSE:
    if (game.pause_soft == 0)
     strcpy(button_text, "Pause");
      else
      {
       strcpy(button_text, "Unpause");
       button_colour = COL_GREEN;
      }
    break;
   case SYSMENU_HALT:
    if (game.pause_hard == 0)
     strcpy(button_text, "Halt execution");
      else
      {
       strcpy(button_text, "Restart execution");
       button_colour = COL_GREEN;
      }
    break;
/*   case SYSMENU_FF:
    if (game.fast_forward == FAST_FORWARD_OFF)
     strcpy(button_text, "Fast forward");
      else
      {
       strcpy(button_text, "Stop fast forward");
       button_colour = COL_GREEN;
      }
    break;*/
   case SYSMENU_SAVE:
    strcpy(button_text, "Save game");
    break;
//   case SYSMENU_LOAD:
//    strcpy(button_text, "Load game");
//    break;
   case SYSMENU_QUIT:
    if (game.phase == GAME_PHASE_OVER)
    {
     strcpy(button_text, "Return to main menu");
     button_colour = COL_GREEN;
    }
     else
      strcpy(button_text, "Quit game");
    break;
   case SYSMENU_QUIT_CONFIRM:
    if (sysmstate.quit_confirm == 1)
    {
     strcpy(button_text, "Really quit?");
     button_colour = COL_RED;
    }
     else
     {
      button_selectable = 0;
      hide_button = 1;
     }
    break;
  }

 if (hide_button == 1)
  continue;

  if (sysmstate.button_highlight == i
   && button_selectable == 1)
//   al_draw_filled_rectangle(editor.panel_x + sysmenu_button[i].x1, editor.panel_y + sysmenu_button[i].y1, editor.panel_x + sysmenu_button[i].x2, editor.panel_y + sysmenu_button[i].y2, colours.base [button_colour] [button_base_shade + 1]);
   add_menu_button(editor.panel_x + sysmenu_button[i].x1, editor.panel_y + sysmenu_button[i].y1, editor.panel_x + sysmenu_button[i].x2, editor.panel_y + sysmenu_button[i].y2, colours.base [button_colour] [button_base_shade + 1], MBUTTON_TYPE_MENU);
    else
     add_menu_button(editor.panel_x + sysmenu_button[i].x1, editor.panel_y + sysmenu_button[i].y1, editor.panel_x + sysmenu_button[i].x2, editor.panel_y + sysmenu_button[i].y2, colours.base [button_colour] [button_base_shade], MBUTTON_TYPE_MENU);
//     al_draw_filled_rectangle(editor.panel_x + sysmenu_button[i].x1, editor.panel_y + sysmenu_button[i].y1, editor.panel_x + sysmenu_button[i].x2, editor.panel_y + sysmenu_button[i].y2, colours.base [button_colour] [button_base_shade]);

   add_menu_string(editor.panel_x + sysmenu_button[i].x1 + 15, editor.panel_y + sysmenu_button[i].y1 + 20, &colours.base [COL_GREY] [SHADE_HIGH], 0, FONT_SQUARE_BOLD, button_text);
//   al_draw_textf(font[FONT_SQUARE_BOLD].fnt, colours.base [COL_GREY] [SHADE_HIGH], editor.panel_x + sysmenu_button[i].x1 + 15, editor.panel_y + sysmenu_button[i].y1 + 20, 0, "%s", button_text);

 }

 draw_menu_buttons();
// mode buttons should be drawn in i_display.c

 display_log(editor.panel_x + EDIT_WINDOW_X, editor.panel_y + editor.mlog_window_y);
// al_set_target_bitmap(sysmstate.bmp);
 al_set_clipping_rectangle(editor.panel_x, editor.panel_y, editor.panel_w, editor.panel_h);

 draw_scrollbar(&mlog.scrollbar_v, 0, 0);

 if (ex_control.panel_drag_ready == 1
		|| ex_control.mouse_dragging_panel == 1)
		draw_panel_drag_ready_line();

// al_set_target_bitmap(al_get_backbuffer(display));
// al_draw_bitmap(sysmstate.bmp, sysmstate.panel_x, sysmstate.panel_y, 0); // TO DO!!!!: need to treat the sysmenu bitmap as a subbitmap of the display backbuffer, which would let us save this drawing operation

#endif


}

// returns 0 if game quit
// returns 1 if still playing
int run_sysmenu(void)
{

 return sysmenu_input();
}

int sysmenu_input(void)
{
#ifdef USING_SYSMENU
 sysmstate.button_highlight = -1;

 run_slider(&mlog.scrollbar_v, 0, 0);

 // check for the mouse pointer being in the game window:
 if (ex_control.mouse_x_pixels < settings.editor_x_split)
 {
  if (inter.panel_input_capture == PANEL_SYSTEM
   && ex_control.mb_press [0] == BUTTON_JUST_PRESSED)
  {
   initialise_control();
   settings.keyboard_capture = INPUT_WORLD;
  }
// needs work - see also equivalent in e_editor.c and t_template.c
  return 1;
 }

 int i;

 int mouse_x = ex_control.mouse_x_pixels - settings.editor_x_split;
 int mouse_y = ex_control.mouse_y_pixels;
 int just_pressed = (ex_control.mb_press [0] == BUTTON_JUST_PRESSED);
 int just_pressed_rmb = (ex_control.mb_press [1] == BUTTON_JUST_PRESSED);

 for (i = 0; i < SYSMENU_BUTTONS; i ++)
 {
  if (mouse_x >= sysmenu_button[i].x1
   && mouse_x <= sysmenu_button[i].x2
   && mouse_y >= sysmenu_button[i].y1
   && mouse_y <= sysmenu_button[i].y2)
  {
   sysmstate.button_highlight = i;
   if (just_pressed)
   {
    switch(i)
    {
     case SYSMENU_PAUSE:
      if (game.pause_soft == 0)
       game.pause_soft = 1;
        else
         game.pause_soft = 0;
      break;
     case SYSMENU_HALT:
      if (game.pause_hard == 0)
       game.pause_hard = 1;
        else
         game.pause_hard = 0;
      break;
/*     case SYSMENU_NEXT_TURN:
      switch(game.phase)
      {
       case GAME_PHASE_PREGAME:
        start_new_turn();
        break;
       case GAME_PHASE_TURN:
        start_new_turn();
        break;
      }
      break;*/
     case SYSMENU_QUIT:
      if (game.phase == GAME_PHASE_OVER)
       return 0; // don't need to ask for confirmation if game over
      sysmstate.quit_confirm ^= 1;
      break;
     case SYSMENU_QUIT_CONFIRM:
      if (sysmstate.quit_confirm == 1)
       return 0; // tells the main game loop to finish
      break;
     case SYSMENU_SAVE:
      save_game();
      return 1;
//     case SYSMENU_LOAD:
//      load_game();
//      return 1;
    }
   }
    else
				{
					if (just_pressed_rmb)
					{
      switch(i)
      {
       case SYSMENU_PAUSE:
   					print_help(HELP_SYSMENU_PAUSE);
        break;
       case SYSMENU_HALT:
   					print_help(HELP_SYSMENU_HALT);
        break;
       case SYSMENU_QUIT:
  	     if (game.phase == GAME_PHASE_OVER)
   					 print_help(HELP_SYSMENU_RETURN_TO_MAIN);
   					  else
   					   print_help(HELP_SYSMENU_QUIT);
        break;
       case SYSMENU_QUIT_CONFIRM:
   					print_help(HELP_SYSMENU_QUIT_CONFIRM);
        break;
       case SYSMENU_SAVE:
   					print_help(HELP_SYSMENU_SAVE);
        break;
      } // end switch for right mouse button
					} // end if just_pressed_rmb
				} // end else for if just_pressed
   break;
  }

 }
#endif
 return 1; // tells the main game loop that we're still playing (as quit_confirm hasn't been clicked on)

}

void close_sysmenu(void)
{

// settings.edit_window = EDIT_WINDOW_CLOSED;
// settings.keyboard_capture = INPUT_WORLD;

 sysmstate.quit_confirm = 0;


}
