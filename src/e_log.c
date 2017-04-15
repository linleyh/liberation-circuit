
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "g_header.h"
#include "c_header.h"
#include "e_slider.h"
#include "e_header.h"
#include "e_log.h"
#include "m_globvars.h"

#include "g_misc.h"
#include "i_header.h"
#include "m_input.h"

#include "p_panels.h"

#include "t_template.h"
#include "e_editor.h"


extern struct fontstruct font [FONTS];

extern struct editorstruct editor; // defined in e_editor.c

ALLEGRO_COLOR mlog_col [MLOG_COLS];

/*

This file contains functions for running the compiler mlog.
It's mostly based on i_console.c but with some differences

*/

struct log_struct mlog;


void write_to_log(const char* str);
void start_log_line(int mcol);
void finish_log_line(void);
void reset_log(void);

int log_visible_line_mouseover(void);
int log_visible_line_to_log_line(int visible_line);

// call this at startup
void init_log(void) // int w_pixels, int h_pixels)
{
 mlog.h_lines = panel[PANEL_LOG].h / LOG_LINE_HEIGHT;
 mlog.w_letters = panel[PANEL_LOG].w / LOG_LETTER_WIDTH;
// mlog.h_pixels = h_pixels;
// mlog.w_pixels = w_pixels;

 mlog.window_pos = LOG_LINES;// - mlog.h_lines;//27;//mlog.h_lines - 1;

// fprintf(stdout, "\nh_lines %i (pix %i def %i)", mlog.h_lines, h_pixels, LOG_LINE_LENGTH);
// error_call();

 reset_log();


// init_slider(&mlog.scrollbar_v, &mlog.window_pos, SLIDEDIR_VERTICAL, 0, LOG_LINES - mlog.h_lines, h_pixels, 1, mlog.h_lines - 1, mlog.h_lines, EDIT_WINDOW_X + game.editor_x_split + editor.edit_window_w, EDIT_WINDOW_Y + editor.edit_window_h + SLIDER_BUTTON_SIZE, SLIDER_BUTTON_SIZE);
// init_slider(&mlog.scrollbar_v, &mlog.window_pos, SLIDEDIR_VERTICAL, 0, LOG_LINES - mlog.h_lines, h_pixels, 1, mlog.h_lines - 1, mlog.h_lines, EDIT_WINDOW_X + settings.editor_x_split + editor.edit_window_w, editor.mlog_window_y, SLIDER_BUTTON_SIZE, COL_BLUE, 0);

 mlog_col [MLOG_COL_EDITOR] = al_map_rgb(120, 120, 220);
 mlog_col [MLOG_COL_TEMPLATE] = al_map_rgb(150, 100, 200);
 mlog_col [MLOG_COL_COMPILER] = al_map_rgb(100, 150, 200);
 mlog_col [MLOG_COL_FILE] = al_map_rgb(100, 200, 150);
 mlog_col [MLOG_COL_ERROR] = al_map_rgb(200, 120, 120);
 mlog_col [MLOG_COL_WARNING] = al_map_rgb(180, 130, 130);
 mlog_col [MLOG_COL_HELP] = al_map_rgb(100, 135, 180);
// mlog_col [MLOG_COL_HELP] = al_map_rgb(170, 185, 200);






}

void reset_log(void)
{

 mlog.lpos = 0;
 mlog.window_pos = LOG_LINES;// - mlog.h_lines;//27;//mlog.h_lines - 1;

 update_slider(SLIDER_LOG_SCROLLBAR_V);

 int i;

 for (i = 0; i < LOG_LINES; i ++)
 {
  mlog.log_line[i].used = 0;
  mlog.log_line[i].text [0] = 0;
  mlog.log_line[i].source_player_index = -1;
//  mlog.log_line[i].source_line = -1;
 }

}

// Call this when the log is resized but its contents shouldn't be cleared.
void log_resized(void)
{

 mlog.h_lines = panel[PANEL_LOG].h / LOG_LINE_HEIGHT;
 mlog.w_letters = panel[PANEL_LOG].w / LOG_LETTER_WIDTH;

 mlog.window_pos = LOG_LINES;// - mlog.h_lines;
 update_slider(SLIDER_LOG_SCROLLBAR_V);


}

// str should be null terminated, although no more than LOG_LINE_LENGTH will be used anyway
void write_to_log(const char* str)
{

/*
// if the new text comes from a different source to the current line, make a new line even if we haven't been told to do so
 if (mlog.log_line[mlog.lpos].source != source
  || mlog.log_line[mlog.lpos].source_line != source_line)
 {
  finish_log_line();
  start_log_line(source, source_line);
 }*/

 int max_length = LOG_LINE_LENGTH; // used to be mlog.w_letters

 int current_length = strlen(mlog.log_line[mlog.lpos].text);

 int space_left = max_length - current_length;

 if (space_left <= 1) // leave 1 for null character at end
  return;

 int i = current_length;
 int j = 0;
 int counter = 160; // maximum number of characters printed at once

 while(str [j] != '\0'
    && counter > 0)
 {
  if (i >= max_length - 2)
   break;
/*
  if (str[j] == '\n' || str[j] == '\r' || i >= mlog.w_letters - 1)
  {
   mlog.log_line[mlog.lpos].text [i] = '\0';
   finish_log_line();
   start_log_line(source, source_line);
   i = 0;
   j ++;
   continue;
  }*/
  mlog.log_line[mlog.lpos].text [i] = str [j];
  j ++;
  i ++;
  counter --;
 };

 mlog.log_line[mlog.lpos].text [i] = '\0';
 mlog.log_line[mlog.lpos].text [max_length - 1] = '\0';

}



void write_number_to_log(int num) //, int source, int source_line)
{
/*
// if the new text comes from a different source to the current line, make a new line even if we haven't been told to do so
 if (mlog.log_line[mlog.lpos].source != source
  || mlog.log_line[mlog.lpos].source_line != source_line)
 {
  finish_log_line();
  start_log_line(source, source_line);
 }*/

//fpr("\n num %i", num);

 int current_length = strlen(mlog.log_line[mlog.lpos].text);

 int space_left = mlog.w_letters - current_length;

 char num_str [10];

 snprintf(num_str, 9, "%i", num);

 if (space_left <= strlen(num_str) + 1) // make sure number will fit on line
  return;

 strcat(mlog.log_line[mlog.lpos].text, num_str);


}


void start_log_line(int mcol)//int source, int source_line)
{

 mlog.log_line[mlog.lpos].text [0] = '\0';
 mlog.log_line[mlog.lpos].source_player_index = -1;

 mlog.log_line[mlog.lpos].used = 1;
// mlog.log_line[mlog.lpos].source = source;
// mlog.log_line[mlog.lpos].source_line = source_line;
 mlog.log_line[mlog.lpos].colour = mcol;

}

// sets a point to jump to if log line clicked on (for compiler errors/warnings)
void set_log_line_source_position(int player_index, int template_index, int src_line)
{
// if this function not called for a log line, source_player_index will have been set to -1
	mlog.log_line[mlog.lpos].source_player_index = player_index;
	mlog.log_line[mlog.lpos].source_template_index = template_index;
	mlog.log_line[mlog.lpos].source_line = src_line;

}


void finish_log_line(void)
{

 mlog.lpos ++;
 if (mlog.lpos >= LOG_LINES)
  mlog.lpos = 0;

}

// a wrapper around other log functions that writes a whole line at once
void write_line_to_log(char* str, int mcol)
{
 start_log_line(mcol);
 write_to_log(str);
 finish_log_line();
}

// this function assumes target bitmap has been set
void display_log(void)
{

// al_set_target_bitmap(al_get_backbuffer(display));
 al_set_clipping_rectangle(panel[PANEL_LOG].x1, panel[PANEL_LOG].y1, panel[PANEL_LOG].w, LOG_WINDOW_H);
// editor.log_sub_bmp = al_create_sub_bitmap(editor.sub_bmp, EDIT_WINDOW_X, editor.mlog_window_y, editor.edit_window_w, LOG_WINDOW_H);

 int x1 = panel[PANEL_LOG].x1;

// If player has several panels open, the log may go off the screen:
 if (x1 < 0)
		x1 = 0;

 int y1 = panel[PANEL_LOG].y1;
 int x2 = x1 + panel[PANEL_LOG].w + 10;
 int y2 = y1 + panel[PANEL_LOG].h + 20;
 int x, y;

 int lines_printed = mlog.h_lines;

 al_draw_filled_rectangle(x1, y1, x2, y2, colours.base [COL_BLUE] [SHADE_MIN]);
// al_draw_rectangle(editor.panel_x + EDIT_WINDOW_X + 1, editor.panel_y + editor.mlog_window_y + 1, editor.edit_window_w - 1, LOG_WINDOW_H - 1, colours.base [COL_BLUE] [SHADE_MED], 1);
// al_draw_rectangle(x1, y1, x2 - 1, y2 - 20, colours.base [COL_BLUE] [SHADE_MED], 1);
 al_draw_line(x1, y1-0.5, x2-1, y1-0.5, colours.base [COL_BLUE] [SHADE_MED], 1);

 int log_line_pos = mlog.window_pos + mlog.lpos;// + mlog.h_lines;

// log_line_pos %= LOG_LINES;

 while (log_line_pos >= LOG_LINES)
 {
  log_line_pos -= LOG_LINES;
 }
 while (log_line_pos < 0)
 {
  log_line_pos += LOG_LINES;
 }

 int display_line_pos = lines_printed;

 int highlight_line = -100;

 if (control.mouse_panel == PANEL_LOG
		&& ex_control.mouse_x_pixels	< x2 - 25)
		highlight_line = log_visible_line_mouseover();

 while (display_line_pos > 0)
 {
  x = x1 + 10;
  y = y1 + (display_line_pos * LOG_LINE_HEIGHT);
/*
  al_draw_textf(font[FONT_BASIC].fnt, mlog_col [mlog.log_line[log_line_pos].colour], x, y, 0, "wp %i lp %i hl %i %i: %s",
																mlog.window_pos, mlog.lpos, mlog.h_lines,
																log_line_pos, mlog.log_line[log_line_pos].text); // must use " %s"; can't use text string directly as it may contain % characters*/
		if (display_line_pos == highlight_line
			&& mlog.log_line[log_line_pos].source_player_index != -1) // should be -1 if not associated with a specific source_edit
			al_draw_filled_rectangle(panel[PANEL_LOG].x1, y - 2, x2, y + 9, colours.base [COL_BLUE] [SHADE_LOW]);
  al_draw_textf(font[FONT_BASIC].fnt, mlog_col [mlog.log_line[log_line_pos].colour], x, y, 0, "%s", mlog.log_line[log_line_pos].text); // must use " %s"; can't use text string directly as it may contain % characters
  display_line_pos --;
  log_line_pos --;
  if (log_line_pos == -1)
   log_line_pos = LOG_LINES - 1;
 };


 draw_scrollbar(SLIDER_LOG_SCROLLBAR_V);

}


void mouse_click_on_log_window(void)
{

 if (ex_control.mouse_x_pixels >= panel[PANEL_LOG].x2 - SLIDER_BUTTON_SIZE)
		return; // this is dealt with elsewhere

 int visible_line = log_visible_line_mouseover();

//fpr("\n wp %i lpos %i vis %i ", mlog.window_pos, mlog.lpos, visible_line);


 if (visible_line == -1)
		return;

	int log_line_index = log_visible_line_to_log_line(visible_line);

	if (log_line_index < 0
		|| log_line_index >= LOG_LINES)
		return;
//fpr(" click %i [%s]", log_line_index, mlog.log_line[log_line_index].text);

 if (mlog.log_line[log_line_index].source_player_index == -1
		|| mlog.log_line[log_line_index].source_player_index >= w.players)
		return;

// should be able to assume that this line is from the compiler, about a particular template.

 open_template(mlog.log_line[log_line_index].source_player_index, mlog.log_line[log_line_index].source_template_index);

 struct source_edit_struct* se = get_current_source_edit();

 if (se == NULL
  || se->type != SOURCE_EDIT_TYPE_SOURCE)
  return;

 int new_source_line = mlog.log_line[log_line_index].source_line;// - 1;
 if (new_source_line < 0)
		new_source_line = 0;

 se->cursor_line = new_source_line;
 se->cursor_pos = 0;
 se->cursor_base = se->cursor_pos;
 se->selected = 0;

 window_find_cursor(se);



}

// returns the visible line that the mouse is over (starts at 0 at the top of the visible log panel.)
// or -1 if the mouse isn't over a line
int log_visible_line_mouseover(void)
{

	int visible_line;

	visible_line = (ex_control.mouse_y_pixels - panel[PANEL_LOG].y1 - 2) / LOG_LINE_HEIGHT;

	if (visible_line < 0)
		return -1;

	if (visible_line >= mlog.h_lines)
		return -1;

	return visible_line;

}

int log_visible_line_to_log_line(int visible_line)
{

// This is terrible:

// int base_log_line_pos = mlog.window_pos + mlog.lpos;// + mlog.h_lines;
 int base_log_line_pos = mlog.window_pos - LOG_LINES + mlog.lpos;
/*
 while (base_log_line_pos >= LOG_LINES)
 {
  base_log_line_pos -= LOG_LINES;
 }
 while (base_log_line_pos < 0)
 {
  base_log_line_pos += LOG_LINES;
 }
*/
 int log_line_pos = base_log_line_pos - mlog.h_lines + visible_line;

 while (log_line_pos < 0)
 {
  log_line_pos += LOG_LINES;
 }
 while (log_line_pos >= LOG_LINES)
 {
  log_line_pos -= LOG_LINES;
 }

 return log_line_pos;


}





