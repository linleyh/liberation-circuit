
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"

#include "g_misc.h"

#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "m_globvars.h"
#include "e_log.h"
#include "e_editor.h"
#include "e_complete.h"
#include "i_header.h"
#include "m_input.h"
#include "e_inter.h"

#include "p_panels.h"

extern struct submenustruct submenu [SUBMENUS];
extern struct view_struct view;

extern struct fontstruct font [FONTS];
extern struct game_struct game;
extern ALLEGRO_DISPLAY* display;

/*

This file contains functions to display the editor interface.

*/


extern struct editorstruct editor; // defined in e_editor.c
extern struct log_struct mlog; // in e_log.c

static void map_source_col(int col_index, int r, int g, int b);
void fix_edit_panel_size_etc(void);
void change_edit_panel_width(void);
void reset_editor_slider_locations(void);

void update_editor_display(void);

void new_source_edit(int new_se);
//void change_source_edit(int new_se);

//#define SCROLLBAR_THICKNESS 12

ALLEGRO_COLOR source_col [STOKEN_TYPES] [2]; // colours are set in init_editor_display() in this file. The [2] is whether the editor is accepting input

void draw_source_line(struct source_edit_struct* se, int x, int y, int w_pixels, int src_line, int start_pos);
void draw_selection_line(struct source_edit_struct* se, int x, int y, int w_pixels, int src_line);
void draw_selection_rect(int x1, int y1, int x2, int y2, int x_offset);

// editor colours are for the background, menu, headers etc (source_col is for source text)
enum
{
EDIT_COL_BORDER,
EDIT_COL_BORDER_NO_FOCUS,
EDIT_COL_SOURCE_BACK,
EDIT_COL_SOURCE_BACK_DIM,
EDIT_COL_CURSOR,
EDIT_COL_MOUSE_CURSOR_INDICATOR,
EDIT_COL_MLOG_BACK,
EDIT_COL_TITLE, // "Code editor" title at top of screen
EDIT_COL_MENU_TITLE, // File, Edit, Build heading
EDIT_COL_MENU_HIGHLIGHT, // File, Edit, Build highlighted
EDIT_COL_MENU_BOX, // background to File etc box
EDIT_COL_MENU_BOX_BORDER,
EDIT_COL_MENU_BOX_TEXT,
EDIT_COL_MENU_BOX_HIGHLIGHT,
EDIT_COL_LINE_NUMBER,
/*EDIT_COL_TAB_HIGHLIGHT,
EDIT_COL_TAB_CURRENT,
EDIT_COL_TAB_BORDER,
EDIT_COL_TAB_NAME,
EDIT_COL_TAB_NAME_CURRENT,*/
EDIT_COL_MENU_BOX_SHORTCUT,
EDIT_COL_OVERWINDOW,
EDIT_COL_OVERWINDOW_BORDER,
EDIT_COL_OVERWINDOW_TEXT,
EDIT_COL_OVERWINDOW_BUTTON,
EDIT_COL_OVERWINDOW_BUTTON_BORDER,
EDIT_COL_SEARCH_BOX,
EDIT_COL_SELECTION, // text selection backgroup

EDIT_COLS
};

ALLEGRO_COLOR edit_col [EDIT_COLS];


/*

*/
void init_editor_display(int screen_w, int screen_h)
{

/* editor.sub_bmp = al_create_sub_bitmap(al_get_backbuffer(display), editor_panel_x, editor_panel_y, screen_w, screen_h);

 if (editor.sub_bmp == NULL)
 {
  fprintf(stdout, "\nError: e_inter.c: init_editor_display(): couldn't create editor sub_bitmap");
  error_call();
 }*/

 editor.text_width = font[FONT_BASIC].width; // must be before fix_edit_panel_size_etc

// set up the source_col array:
 map_source_col(STOKEN_TYPE_WORD, 120, 120, 220);
 map_source_col(STOKEN_TYPE_KEYWORD, 90, 90, 180);
 map_source_col(STOKEN_TYPE_NUMBER, 190, 80, 220);
 map_source_col(STOKEN_TYPE_OPERATOR, 190, 160, 80);
 map_source_col(STOKEN_TYPE_PUNCTUATION, 160, 160, 50);
 map_source_col(STOKEN_TYPE_STRING, 80, 220, 220);
 map_source_col(STOKEN_TYPE_COMMENT, 100, 100, 100);
 map_source_col(STOKEN_TYPE_PREPROCESSOR, 60, 140, 60);
 map_source_col(STOKEN_TYPE_ERROR, 250, 20, 20);



 edit_col [EDIT_COL_BORDER] = colours.base [COL_BLUE] [SHADE_MED];
 edit_col [EDIT_COL_BORDER_NO_FOCUS] = colours.base [COL_BLUE] [SHADE_MED];
 edit_col [EDIT_COL_SOURCE_BACK] = colours.base [COL_BLUE] [SHADE_LOW];
 edit_col [EDIT_COL_SOURCE_BACK_DIM] = al_map_rgb(40, 40, 60);//colours.base [COL_GREY] [SHADE_LOW];
 edit_col [EDIT_COL_CURSOR] = colours.base [COL_GREY] [SHADE_MAX];
 edit_col [EDIT_COL_MOUSE_CURSOR_INDICATOR] = colours.base_trans [COL_GREY] [SHADE_MED] [TRANS_MED];
 edit_col [EDIT_COL_MLOG_BACK] = colours.base [COL_BLUE] [SHADE_LOW];
 edit_col [EDIT_COL_TITLE] = colours.base [COL_GREY] [SHADE_MAX];
 edit_col [EDIT_COL_MENU_TITLE] = colours.base [COL_GREY] [SHADE_HIGH];
 edit_col [EDIT_COL_MENU_HIGHLIGHT] = colours.base [COL_BLUE] [SHADE_HIGH];
 edit_col [EDIT_COL_MENU_BOX] = colours.base [COL_BLUE] [SHADE_MIN];
 edit_col [EDIT_COL_MENU_BOX_BORDER] = colours.base [COL_BLUE] [SHADE_HIGH];
 edit_col [EDIT_COL_MENU_BOX_TEXT] = colours.base [COL_GREY] [SHADE_HIGH];
 edit_col [EDIT_COL_MENU_BOX_HIGHLIGHT] = colours.base [COL_BLUE] [SHADE_MED];
/* edit_col [EDIT_COL_TAB_HIGHLIGHT] = colours.base [COL_BLUE] [SHADE_MAX];
 edit_col [EDIT_COL_TAB_CURRENT] = colours.base [COL_BLUE] [SHADE_HIGH];
 edit_col [EDIT_COL_TAB_BORDER] = colours.base [COL_BLUE] [SHADE_HIGH];
 edit_col [EDIT_COL_TAB_NAME] = colours.base [COL_GREY] [SHADE_HIGH];
 edit_col [EDIT_COL_TAB_NAME_CURRENT] = colours.base [COL_GREY] [SHADE_MAX];*/
 edit_col [EDIT_COL_LINE_NUMBER] = colours.base [COL_GREY] [SHADE_LOW];
 edit_col [EDIT_COL_MENU_BOX_SHORTCUT] = colours.base [COL_GREY] [SHADE_MED];
 edit_col [EDIT_COL_OVERWINDOW] = colours.base [COL_BLUE] [SHADE_MIN];
 edit_col [EDIT_COL_OVERWINDOW_BORDER] = colours.base [COL_BLUE] [SHADE_MED];
 edit_col [EDIT_COL_OVERWINDOW_TEXT] = colours.base [COL_GREY] [SHADE_HIGH];
 edit_col [EDIT_COL_OVERWINDOW_BUTTON] = colours.base [COL_BLUE] [SHADE_MED];
 edit_col [EDIT_COL_OVERWINDOW_BUTTON_BORDER] = colours.base [COL_BLUE] [SHADE_HIGH];
 edit_col [EDIT_COL_SELECTION] = colours.base [COL_TURQUOISE] [SHADE_MED];
 edit_col [EDIT_COL_SEARCH_BOX] = colours.base [COL_GREY] [SHADE_MIN];


 fix_edit_panel_size_etc();


}

static void map_source_col(int col_index, int r, int g, int b)
{
	source_col [col_index] [1] = al_map_rgb(r, g, b);
	source_col [col_index] [0] = al_map_rgb(r * 0.9, g * 0.8, b * 0.7);
//	source_col [col_index] [0] = al_map_rgb(r - 20, g - 20, b - 20);

}

void fix_edit_panel_size_etc(void)
{


 editor.edit_window_x1 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X;;
//
 editor.edit_window_h = panel[PANEL_EDITOR].h - SLIDER_BUTTON_SIZE - EDIT_WINDOW_Y - 12;
 editor.edit_window_h /= EDIT_LINE_H;
 editor.edit_window_h *= EDIT_LINE_H;
 editor.edit_window_lines = editor.edit_window_h / EDIT_LINE_H;

// editor.mlog_window_y = EDIT_WINDOW_Y + editor.edit_window_h + SLIDER_BUTTON_SIZE + 10;

 change_edit_panel_width();


}

// This function is called whenever the editor panel width is changed (not counting opening and closing)
// It just resets x values, not y
void change_edit_panel_width(void)
{

 editor.edit_window_w = panel[PANEL_EDITOR].w - SLIDER_BUTTON_SIZE - EDIT_WINDOW_X - 12;
 editor.edit_window_chars = editor.edit_window_w / editor.text_width;

 reset_editor_slider_locations();

 reset_slider_length(SLIDER_EDITOR_SCROLLBAR_H, editor.edit_window_w, editor.edit_window_w / editor.text_width);

 if (panel[PANEL_EDITOR].open) // can this test be removed?
	{
//  struct source_edit_struct* se = get_current_source_edit();

//  if (se != NULL)
//   init_slider(&editor.scrollbar_h, &se->window_pos, SLIDEDIR_HORIZONTAL, 0, SOURCE_TEXT_LINE_LENGTH, editor.edit_window_w, 1, (editor.edit_window_w / editor.text_width) - 1, editor.edit_window_w / editor.text_width, EDIT_WINDOW_X + settings.editor_x_split - 1, EDIT_WINDOW_Y + editor.edit_window_h, SLIDER_BUTTON_SIZE, COL_BLUE, 0);
	}

}

void reset_editor_slider_locations(void)
{

	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x1 = EDIT_WINDOW_X + editor.edit_window_w - 10;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y1 = EDIT_WINDOW_Y - 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x1 + SLIDER_BUTTON_SIZE;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y1 + editor.edit_window_h + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].w = SLIDER_BUTTON_SIZE;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].h = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y2 - panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y1;

//	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x1 = EDIT_WINDOW_X + editor.edit_window_w + 1;

	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x1 = EDIT_WINDOW_X - 10;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y1 = EDIT_WINDOW_Y + editor.edit_window_h + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x2 = EDIT_WINDOW_X + editor.edit_window_w + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y1 + SLIDER_BUTTON_SIZE;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].w = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x2 - panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].h = SLIDER_BUTTON_SIZE;


/*
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x1 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + editor.edit_window_w + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y1 = panel[PANEL_EDITOR].y1 + EDIT_WINDOW_Y + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].x1 + SLIDER_BUTTON_SIZE;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].y1 + editor.edit_window_h + 1;
//	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_V].h =  shouldn't change

	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x1 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + editor.edit_window_w + 1;

	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x1 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y1 = panel[PANEL_EDITOR].y1 + EDIT_WINDOW_Y + editor.edit_window_h + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].x2 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + editor.edit_window_w + 1;
	panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y2 = panel[PANEL_EDITOR].element[FPE_EDITOR_WINDOW_SCROLLBAR_H].y1 + SLIDER_BUTTON_SIZE;
*/

}


void draw_edit_bmp(void)
{

 al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

 update_editor_display();

}


void draw_panel_drag_ready_line(void)
{

// al_draw_line(settings.editor_x_split + 3.5, 0, settings.editor_x_split + 3.5, settings.option [OPTION_WINDOW_H], colours.base [COL_AQUA] [SHADE_MED], 0);

}


void update_editor_display(void)
{

 int editor_panel_x = panel[PANEL_EDITOR].x1;
 int editor_panel_y = panel[PANEL_EDITOR].y1;

// al_set_target_bitmap(al_get_backbuffer(display));

// Should be able to move this to the panel display function:
 al_set_clipping_rectangle(editor_panel_x, editor_panel_y, panel[PANEL_EDITOR].w, panel[PANEL_EDITOR].h);
/*
 if (inter.panel_input_capture == INPUT_EDITOR)
  al_clear_to_color(edit_col [EDIT_COL_BORDER]); // Is this efficient when using a clipping rectangle?
   else
    al_clear_to_color(edit_col [EDIT_COL_BORDER_NO_FOCUS]);*/

 al_clear_to_color(panel [PANEL_EDITOR].background_colour);

 struct source_edit_struct* se = NULL;

 float x, y;
 int i;

 al_draw_textf(font[FONT_SQUARE].fnt, edit_col [EDIT_COL_TITLE], editor_panel_x + 5, editor_panel_y + 2, ALLEGRO_ALIGN_LEFT, "Code editor");


// now draw a box around the source edit window:
// al_draw_rectangle(EDIT_WINDOW_X - 1, EDIT_WINDOW_Y - 1, EDIT_WINDOW_X + editor.edit_window_w + 1, EDIT_WINDOW_Y + editor.edit_window_h + 1, base_col [COL_BLUE] [SHADE_MED], 1);

 if (control.editor_captures_input)
  al_draw_filled_rectangle(editor_panel_x + EDIT_WINDOW_X - 1, editor_panel_y + EDIT_WINDOW_Y - 1, editor_panel_x + EDIT_WINDOW_X + editor.edit_window_w + 1, editor_panel_y + EDIT_WINDOW_Y + editor.edit_window_h + 1, edit_col [EDIT_COL_SOURCE_BACK]);
   else
    al_draw_filled_rectangle(editor_panel_x + EDIT_WINDOW_X - 1, editor_panel_y + EDIT_WINDOW_Y - 1, editor_panel_x + EDIT_WINDOW_X + editor.edit_window_w + 1, editor_panel_y + EDIT_WINDOW_Y + editor.edit_window_h + 1, edit_col [EDIT_COL_SOURCE_BACK_DIM]);

// draw source edit window scrollbars:
// if (editor.current_source_edit_index != -1
//  && editor.tab_type [editor.current_tab] == TAB_TYPE_SOURCE)
// {
//  draw_scrollbar(&editor.scrollbar_v, 0, 0);
//  draw_scrollbar(&editor.scrollbar_h, 0, 0);
// }

 if (editor.current_source_edit_index != -1
  && editor.source_edit [editor.current_source_edit_index].active)
 {
 	if (!editor.source_edit [editor.current_source_edit_index].saved)
   al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_BOX_TEXT], (int) (editor_panel_x + scaleUI_x(FONT_SQUARE,103)), (int) (editor_panel_y + 3), ALLEGRO_ALIGN_RIGHT, "*");
  al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_BOX_TEXT], (int) (editor_panel_x + scaleUI_x(FONT_SQUARE,105)), (int) (editor_panel_y + 3), ALLEGRO_ALIGN_LEFT, "%s", editor.source_edit [editor.current_source_edit_index].src_file_path);
//  al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_BOX_TEXT], editor_panel_x + 105, editor_panel_y + 3, ALLEGRO_ALIGN_LEFT, "%s", editor.source_edit [editor.current_source_edit_index].src_file_path);
		draw_scrollbar(SLIDER_EDITOR_SCROLLBAR_H);
		draw_scrollbar(SLIDER_EDITOR_SCROLLBAR_V);
 }


// al_set_target_bitmap(editor.edit_sub_bmp);
// al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(editor_panel_x + EDIT_WINDOW_X, editor_panel_y + EDIT_WINDOW_Y, editor.edit_window_w, editor.edit_window_h);

 x = 0;
 y = 0;

 if (editor.current_source_edit_index == -1
		|| editor.source_edit[editor.current_source_edit_index].active == 0)
 {
    x = editor_panel_x + EDIT_WINDOW_X + editor.edit_window_w / 2;
    y = editor.edit_window_h / 2;
    al_draw_textf(font[FONT_SQUARE].fnt, edit_col [EDIT_COL_TITLE], (int) x, (int) y, ALLEGRO_ALIGN_CENTRE, "No file loaded");
 }
  else
  {

   se = get_current_source_edit();
//   se = get_current_source_edit();

   if (se == NULL)
    return;

   int print_line;

//   switch(se->type)
   switch(se->active)
   {
    case 1: // SOURCE_EDIT_TYPE_SOURCE:
     x = editor_panel_x + EDIT_WINDOW_X + SOURCE_WINDOW_MARGIN;

     for (i = 0; i < editor.edit_window_lines; i ++)
     {
      print_line = se->window_line + i;
      if (print_line >= SOURCE_TEXT_LINES)
       break;
      y = editor_panel_y + EDIT_WINDOW_Y + i * EDIT_LINE_H + EDIT_LINE_OFFSET;
      draw_selection_line(se, x, y, editor.edit_window_w, print_line);
      draw_source_line(se, x, y, editor.edit_window_w, print_line, se->window_pos);
     }
/*
     if (editor.mouse_cursor_line != -1)
//						&& (editor.mouse_cursor_line != se->cursor_line
//						 || editor.mouse_cursor_pos != se->cursor_pos))
					{
      x = editor_panel_x + EDIT_WINDOW_X + (((editor.mouse_cursor_pos+1) - se->window_pos) * editor.text_width) - SOURCE_WINDOW_MARGIN;
      y = editor_panel_y + EDIT_WINDOW_Y + (editor.mouse_cursor_line - se->window_line) * EDIT_LINE_H + EDIT_LINE_OFFSET;
      al_draw_filled_rectangle(x - 1, y - 2, x + 1, y + EDIT_LINE_H - 4 + EDIT_LINE_OFFSET, edit_col [EDIT_COL_MOUSE_CURSOR_INDICATOR]);
					}
*/

// now draw the cursor
     if (!control.editor_captures_input
						||	editor.cursor_flash > CURSOR_FLASH_OFF)
     {
      x = editor_panel_x + EDIT_WINDOW_X + (((se->cursor_pos+1) - se->window_pos) * editor.text_width) - SOURCE_WINDOW_MARGIN;
      y = editor_panel_y + EDIT_WINDOW_Y + (se->cursor_line - se->window_line) * EDIT_LINE_H + EDIT_LINE_OFFSET;
      al_draw_filled_rectangle(x - scaleUI_y(FONT_BASIC,2), y - 2, x + scaleUI_y(FONT_BASIC,1), y + EDIT_LINE_H - 4 + EDIT_LINE_OFFSET, edit_col [EDIT_COL_CURSOR]);
//       al_draw_filled_rectangle(x - 1, y - 2, x + 1, y + EDIT_LINE_H - 4 + EDIT_LINE_OFFSET, edit_col [EDIT_COL_CURSOR]);
     }
     break; // end case SOURCE_EDIT_TYPE_SOURCE
    case 0: //SOURCE_EDIT_TYPE_BINARY:
     x = editor_panel_x + EDIT_WINDOW_X + editor.edit_window_w / 2;
     y = editor_panel_y + EDIT_WINDOW_Y + editor.edit_window_h / 2;
/*     if (se->bcode.static_length > 0)
      al_draw_textf(font[FONT_SQUARE].fnt, edit_col [EDIT_COL_TITLE], x, y, ALLEGRO_ALIGN_CENTRE, "This file is in bcode format (length: %i).", se->bcode.static_length);
       else
        al_draw_textf(font[FONT_SQUARE].fnt, edit_col [EDIT_COL_TITLE], x, y, ALLEGRO_ALIGN_CENTRE, "This file is in bcode format (length unknown).");*/
     al_draw_textf(font[FONT_SQUARE].fnt, edit_col [EDIT_COL_TITLE], (int) x, (int) (y+20), ALLEGRO_ALIGN_CENTRE, "Use \"convert bcode\" in build menu to get a text version.");
     break;
   }
  } // end else block that draws source or binary window


// al_set_target_bitmap(al_get_backbuffer(display));
// al_set_target_bitmap(editor.log_sub_bmp);

// display_log();

// al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(editor_panel_x, editor_panel_y, panel[PANEL_EDITOR].w, panel[PANEL_EDITOR].h);

// draw line numbers for source:
 if (editor.current_source_edit_index != -1
	 && editor.source_edit [editor.current_source_edit_index].active)
 {
   for (i = 0; i < editor.edit_window_lines; i ++)
   {
    if (se->window_line + i >= SOURCE_TEXT_LINES - 1)
     break;
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_LINE_NUMBER], editor_panel_x + EDIT_WINDOW_X - 2, EDIT_WINDOW_Y + (i * EDIT_LINE_H) + EDIT_LINE_OFFSET, ALLEGRO_ALIGN_RIGHT, "%i", se->window_line + i + 1);
   }
 }

// draw message log scrollbar
// draw_scrollbar(&mlog.scrollbar_v, 0, 0);



// now draw submenus

 y = EMENU_BAR_Y;

 if (editor.submenu_name_highlight != -1)
 {
  x = editor_panel_x + EMENU_BAR_X + (editor.submenu_name_highlight * EMENU_BAR_NAME_WIDTH);
  al_draw_filled_rectangle(x, y, x + EMENU_BAR_NAME_WIDTH, y + EMENU_BAR_H - 2, edit_col [EDIT_COL_MENU_HIGHLIGHT]);
 }

 y = EMENU_BAR_Y + 2;

 for (i = 0; i < SUBMENUS; i ++)
 {
  x = editor_panel_x + EMENU_BAR_X + (i * EMENU_BAR_NAME_WIDTH) + 3;
  switch(i)
  {
   case SUBMENU_FILE:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_TITLE], (int) x, (int) y, ALLEGRO_ALIGN_LEFT, "File"); break;
   case SUBMENU_EDIT:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_TITLE], (int) x, (int) y, ALLEGRO_ALIGN_LEFT, "Edit"); break;
   case SUBMENU_SEARCH:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_TITLE], (int) x, (int) y, ALLEGRO_ALIGN_LEFT, "Find"); break;
   case SUBMENU_COMPILE:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_TITLE], (int) x, (int) y, ALLEGRO_ALIGN_LEFT, "Compile"); break;
//    al_draw_textf(font, base_col [COL_GREY] [SHADE_HIGH], x, y, ALLEGRO_ALIGN_LEFT, "Compile %i %i", editor.undo_buffer_pos, editor.undo_buffer_base_pos); break;
  }
 }


// draw source file tabs:
// y = SOURCE_TAB_Y;
// int text_col;
/*
 for (i = 0; i < ESOURCES; i ++)
 {
  if (editor.tab_type [i] == TAB_TYPE_NONE)
   break; // shouldn't be any gaps between tabs in the tab_index array
  switch(editor.tab_type [i])
  {
   case TAB_TYPE_NONE:
    break; // nothing to do
   case TAB_TYPE_SOURCE:
   case TAB_TYPE_BINARY:
    x = editor_panel_x + SOURCE_TAB_X + (SOURCE_TAB_W * i);
    if (editor.tab_highlight == i)
     al_draw_filled_rectangle(x, y, x + SOURCE_TAB_W, y + SOURCE_TAB_H, edit_col [EDIT_COL_TAB_HIGHLIGHT]);
    if (editor.current_tab == i)
    {
     text_col = EDIT_COL_TAB_NAME_CURRENT;
     if (editor.tab_highlight != i)
      al_draw_filled_rectangle(x, y, x + SOURCE_TAB_W, y + SOURCE_TAB_H, edit_col [EDIT_COL_TAB_CURRENT]);
    }
      else
       text_col = EDIT_COL_TAB_NAME;
    if (editor.source_edit[editor.tab_index [i]].saved == 0)
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [text_col], x + 3, y + 2, ALLEGRO_ALIGN_LEFT, "%s", editor.tab_name_unsaved [i]);
      else
       al_draw_textf(font[FONT_BASIC].fnt, edit_col [text_col], x + 3, y + 2, ALLEGRO_ALIGN_LEFT, "%s", editor.tab_name [i]);
    al_draw_rectangle(x, y, x + SOURCE_TAB_W, y + SOURCE_TAB_H, edit_col [EDIT_COL_TAB_BORDER], 1);
    break;
  } // end tab type switch

 } // end ESOURCES tab loop
*/
 al_set_clipping_rectangle(0, 0, settings.option [OPTION_WINDOW_W], settings.option [OPTION_WINDOW_H]);

	if (panel[PANEL_EDITOR].element[FPE_EDITOR_PANEL_RESIZE].last_highlight == inter.running_time)
	{
  al_draw_filled_rectangle(editor_panel_x, 0, editor_panel_x + 5, settings.option [OPTION_WINDOW_H], colours.base [COL_BLUE] [SHADE_MED]);
	}
	 else
   al_draw_filled_rectangle(editor_panel_x, 0, editor_panel_x + 2, settings.option [OPTION_WINDOW_H], colours.base [COL_BLUE] [SHADE_LOW]);



 if (completion.list_size > 0
		&& se != NULL)
		draw_code_completion_box();

// draw open submenu, if any.
 if (editor.submenu_open != -1)
 {

// draw box
  al_draw_filled_rectangle(editor_panel_x + editor.submenu_x, editor.submenu_y, editor_panel_x + editor.submenu_x + editor.submenu_w, editor.submenu_y + editor.submenu_h, edit_col [EDIT_COL_MENU_BOX]);

// highlight a line if mouse is hovering over it (submenu_highlight is set in e_editor.c)
  if (editor.submenu_highlight != -1)
  {
   al_draw_filled_rectangle(editor_panel_x + editor.submenu_x, editor.submenu_y + (editor.submenu_highlight * SUBMENU_LINE_HEIGHT), editor_panel_x + editor.submenu_x + editor.submenu_w, editor.submenu_y + ((editor.submenu_highlight + 1) * SUBMENU_LINE_HEIGHT) - 2, edit_col [EDIT_COL_MENU_BOX_HIGHLIGHT]);
  }

// border around the box
  al_draw_rectangle(editor_panel_x + editor.submenu_x, editor.submenu_y, editor_panel_x + editor.submenu_x + editor.submenu_w, editor.submenu_y + editor.submenu_h, edit_col [EDIT_COL_MENU_BOX_BORDER], 1);


  for (i = 0; i < submenu[editor.submenu_open].lines; i ++)
  {
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_BOX_TEXT], editor_panel_x + editor.submenu_x + 5, editor.submenu_y + 2 + (SUBMENU_LINE_HEIGHT * i), ALLEGRO_ALIGN_LEFT, "%s", submenu[editor.submenu_open].line[i].name);
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_MENU_BOX_SHORTCUT], editor_panel_x + editor.submenu_x + editor.submenu_w - 5, editor.submenu_y + 2 + (SUBMENU_LINE_HEIGHT * i), ALLEGRO_ALIGN_RIGHT, "%s", submenu[editor.submenu_open].line[i].shortcut);

  }

 }


// draw overwindow, if any:
 if (editor.overwindow_type != OVERWINDOW_TYPE_NONE)
 {
  al_draw_filled_rectangle(editor_panel_x + editor.overwindow_x, editor.overwindow_y, editor_panel_x + editor.overwindow_x + editor.overwindow_w, editor.overwindow_y + editor.overwindow_h, edit_col [EDIT_COL_OVERWINDOW]);
  al_draw_rectangle(editor_panel_x + editor.overwindow_x, editor.overwindow_y, editor_panel_x + editor.overwindow_x + editor.overwindow_w, editor.overwindow_y + editor.overwindow_h, edit_col [EDIT_COL_OVERWINDOW_BORDER], 1);
  switch(editor.overwindow_type)
  {
   case OVERWINDOW_TYPE_CLOSE:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_x + (editor.overwindow_w / 2), editor.overwindow_y + 30, ALLEGRO_ALIGN_CENTRE, "Close file without saving?");
    break;
   case OVERWINDOW_TYPE_FIND:
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_x + (editor.overwindow_w / 2), editor.overwindow_y + 20, ALLEGRO_ALIGN_CENTRE, "Find");
#define SEARCH_BOX_X 10
#define SEARCH_BOX_W scaleUI_x(FONT_BASIC,120)
#define SEARCH_BOX_Y 35
#define SEARCH_BOX_H scaleUI_y(FONT_BASIC,15)
    al_draw_filled_rectangle(editor_panel_x + editor.overwindow_x + SEARCH_BOX_X, editor.overwindow_y + SEARCH_BOX_Y, editor_panel_x + editor.overwindow_x + editor.overwindow_w - SEARCH_BOX_X, editor.overwindow_y + SEARCH_BOX_Y + SEARCH_BOX_H, edit_col [EDIT_COL_SEARCH_BOX]);
//    al_draw_rectangle(editor_panel_x + editor.overwindow_x + SEARCH_BOX_X, editor.overwindow_y + SEARCH_BOX_Y, editor_panel_x + editor.overwindow_x + editor.overwindow_w - SEARCH_BOX_X, editor.overwindow_y + SEARCH_BOX_Y + SEARCH_BOX_H, edit_col [EDIT_COL_OVERWINDOW_BORDER], 1);
    al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_x + SEARCH_BOX_X + 3, editor.overwindow_y + SEARCH_BOX_Y + 3, ALLEGRO_ALIGN_LEFT, "%s", editor.search_string);
// use the same cursor flash value as the editor:
    if (editor.cursor_flash > CURSOR_FLASH_OFF)
    {
      x = editor_panel_x + editor.overwindow_x + SEARCH_BOX_X + 1 + (strlen(editor.search_string) * editor.text_width);
      y = editor.overwindow_y + SEARCH_BOX_Y + 1;
      al_draw_filled_rectangle(x, y, x + 2, y + EDIT_LINE_H - 3 + EDIT_LINE_OFFSET, edit_col [EDIT_COL_CURSOR]);
    }
    break;
  }
  for (i = 0; i < editor.overwindow_buttons; i ++)
  {
   al_draw_filled_rectangle(editor_panel_x + editor.overwindow_button_x [i], editor.overwindow_button_y [i], editor_panel_x + editor.overwindow_button_x [i] + OVERWINDOW_BUTTON_W, editor.overwindow_button_y [i] + OVERWINDOW_BUTTON_H, edit_col [EDIT_COL_OVERWINDOW_BUTTON]);
   al_draw_rectangle(editor_panel_x + editor.overwindow_button_x [i], editor.overwindow_button_y [i], editor_panel_x + editor.overwindow_button_x [i] + OVERWINDOW_BUTTON_W, editor.overwindow_button_y [i] + OVERWINDOW_BUTTON_H, edit_col [EDIT_COL_OVERWINDOW_BUTTON_BORDER], 1);
   switch(editor.overwindow_button_type [i])
   {
    case OVERWINDOW_BUTTON_TYPE_CLOSE_TAB:
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_button_x [i] + (OVERWINDOW_BUTTON_W / 2), editor.overwindow_button_y [i] + 5, ALLEGRO_ALIGN_CENTRE, "Yes");
     break;
    case OVERWINDOW_BUTTON_TYPE_NO:
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_button_x [i] + (OVERWINDOW_BUTTON_W / 2), editor.overwindow_button_y [i] + 5, ALLEGRO_ALIGN_CENTRE, "No");
     break;
    case OVERWINDOW_BUTTON_TYPE_FIND:
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_button_x [i] + (OVERWINDOW_BUTTON_W / 2), editor.overwindow_button_y [i] + 5, ALLEGRO_ALIGN_CENTRE, "Find");
     break;
    case OVERWINDOW_BUTTON_TYPE_CANCEL_FIND:
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_button_x [i] + (OVERWINDOW_BUTTON_W / 2), editor.overwindow_button_y [i] + 5, ALLEGRO_ALIGN_CENTRE, "Cancel");
     break;
    default:
     al_draw_textf(font[FONT_BASIC].fnt, edit_col [EDIT_COL_OVERWINDOW_TEXT], editor_panel_x + editor.overwindow_button_x [i] + (OVERWINDOW_BUTTON_W / 2), editor.overwindow_button_y [i] + 5, ALLEGRO_ALIGN_CENTRE, "Error??");
     break;
   }

  }
 }



// anything after here will overwrite an open submenu




}



// w_pixels is currently unused (but should be used - I think this function tries to draw the whole line after start_pos, even when it's being clipped at the right
void draw_source_line(struct source_edit_struct* se, int x, int y, int w_pixels, int src_line, int start_pos)
{

// al_draw_textf(font, source_col [0], x, y, ALLEGRO_ALIGN_LEFT, "%s", se->source.text [src_line]);
// return;


// int w_letters = (w_pixels / editor.text_width) + 1; // assumes font is fixed-width. +1 to deal with rounding (extra characters will just be clipped)

 int src_pos = 0;
 char* src = se->text [se->line_index [src_line]];

// al_draw_textf(font, source_col [STOKEN_TYPE_WORD], x + 30, y, ALLEGRO_ALIGN_LEFT, "%i, %i", src_line, se->line_index [src_line]);

// now set the start of the part of the string to be drawn, based on start_pos (which will be non-zero if the window is scrolled to the right):
 while (src_pos < start_pos)
 {
  if (src [src_pos] == '\0')
   return; // nothing to display (maybe line is empty or window is scrolled to right)
  src_pos ++;
 };

 char temp_str [SOURCE_TEXT_LINE_LENGTH];
 int str_pos = 0;
 int current_src_colour = 0;

// now go through src, reading blocks of code that has the same syntax highlighting and printing them:
 while(TRUE)
 {
// read the next character and determine its colour:
  if (src [src_pos] == '\0')
   return; // finished
  str_pos = 0;
  temp_str [str_pos] = src [src_pos];
  current_src_colour = se->source_colour [se->line_index [src_line]] [src_pos];

// now read more characters until we find a non-space character of a different colour (or the end of the string):
  while(TRUE)
  {
   src_pos ++;
   str_pos ++;
   if (src [src_pos] == '\0' // end of line
    || (se->source_colour [se->line_index [src_line]] [src_pos] != current_src_colour // change in colour
     && src [src_pos] != ' ')) // ignore colour changes if a space
   {
// finished reading the token - so print it:
    temp_str [str_pos] = '\0';
    al_draw_textf(font[FONT_BASIC].fnt, source_col [current_src_colour] [control.editor_captures_input], x, y, ALLEGRO_ALIGN_LEFT, "%s", temp_str);
    x += strlen(temp_str) * editor.text_width;
// and leave this loop to find the next token.
    break;
   }
   temp_str [str_pos] = src [src_pos];
  };


 };


}

#define SELECT_Y_ABOVE_LINE 4
// higher values for SELECT_Y_BELOW_LINE actually move the bottom of the selection rectangle upwards
#define SELECT_Y_BELOW_LINE 3

// assumes target bitmap set
// src_line is line_index index, not text array index
void draw_selection_line(struct source_edit_struct* se, int x, int y, int w_pixels, int src_line)
{

 int x_offset = se->window_pos * editor.text_width * -1;

 if (!se->selected
  || (src_line < se->select_fix_line
   && src_line < se->select_free_line)
  || (src_line > se->select_fix_line
   && src_line > se->select_free_line))
    return;

 if (src_line != se->select_fix_line
  && src_line != se->select_free_line)
 {
  draw_selection_rect(x, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + ((strlen(se->text [se->line_index [src_line]]) + 1) * editor.text_width), y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
  return;
 }

 if (src_line == se->select_fix_line
  && src_line == se->select_free_line)
 {
  if (se->select_fix_pos < se->select_free_pos)
  {
   draw_selection_rect(x + se->select_fix_pos * editor.text_width, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + se->select_free_pos * editor.text_width, y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
   return;
  }
  draw_selection_rect(x + se->select_free_pos * editor.text_width, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + se->select_fix_pos * editor.text_width, y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
  return;
 }


 if (se->select_fix_line < se->select_free_line)
 {
  if (src_line == se->select_fix_line)
  {
   draw_selection_rect(x + se->select_fix_pos * editor.text_width, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + ((strlen(se->text [se->line_index [src_line]]) + 1) * editor.text_width), y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
   return;
  }
  draw_selection_rect(x, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + (se->select_free_pos * editor.text_width), y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
  return;
 }

// free point must be before fix point:
 if (src_line == se->select_free_line)
 {
  draw_selection_rect(x + se->select_free_pos * editor.text_width, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + ((strlen(se->text [se->line_index [src_line]]) + 1) * editor.text_width), y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
  return;
 }
 draw_selection_rect(x, y - SELECT_Y_ABOVE_LINE + EDIT_LINE_OFFSET, x + (se->select_fix_pos * editor.text_width), y + EDIT_LINE_H - SELECT_Y_BELOW_LINE + EDIT_LINE_OFFSET, x_offset);
 return;



}

void draw_selection_rect(int x1, int y1, int x2, int y2, int x_offset)
{

 ALLEGRO_COLOR scol = edit_col [EDIT_COL_SELECTION];

 x1 += x_offset;
 x2 += x_offset;

 if (x1 < panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X)
  x1 = panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X;
 if (x2 < panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X)
  return;
 if (x1 > panel[PANEL_EDITOR].x1 + EDIT_WINDOW_X + editor.edit_window_w)
  return;

 al_draw_filled_rectangle(x1, y1, x2, y2, scol);

}




