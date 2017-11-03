#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>
#include <string.h>

#include "m_config.h"

#include "e_slider.h"
#include "g_header.h"
#include "m_globvars.h"
#include "m_input.h"
#include "m_maths.h"
#include "i_console.h"
#include "i_display.h"
#include "i_header.h"
#include "i_buttons.h"

#include "g_misc.h"


extern struct vbuf_struct vbuf;

struct menu_string_struct
{

 const char* str;// [MENU_STRING_LENGTH];
 int x;
 int y;
 ALLEGRO_COLOR* col;
 int align;
 int font_index;
 int type;

};

struct menu_string_struct menu_string [MENU_STRINGS];
int menu_string_pos;



extern struct fontstruct font [FONTS];

// defined in i_display.c:
//extern ALLEGRO_VERTEX poly_buffer [POLY_BUFFER]; // POLY_BUFFER #defined in i_header.h
//extern int poly_pos;


static void add_button_triangle(float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col);

static void check_button_buffer(void);
void draw_button_buffer(void);


void reset_i_buttons(void)
{

//	poly_pos = 0;
	menu_string_pos = 0;

}

void add_menu_string(int x, int y, ALLEGRO_COLOR* col, int align, int font_index, const char* str)
{

	if (menu_string_pos >= MENU_STRINGS)
	{
		fprintf(stdout, "\Error: i_buttons.c: add_menu_string(): menu_string_pos too high (string is [%s]).", str);
		error_call();
	}

// strcpy(menu_string [menu_string_pos].str, str);
 menu_string [menu_string_pos].str = str; // this just assigns the pointer!
 menu_string [menu_string_pos].x = x;
 menu_string [menu_string_pos].y = y;
 menu_string [menu_string_pos].align = align;
 menu_string [menu_string_pos].font_index = font_index;
 menu_string [menu_string_pos].col = col;

 menu_string_pos++;

}


void draw_menu_strings(void)
{
 int i;

 for (i = 0; i < menu_string_pos; i ++)
	{
		al_draw_textf(font[menu_string[i].font_index].fnt,
																*menu_string[i].col,
																menu_string[i].x,
																menu_string[i].y,
																menu_string[i].align,
																"%s",
																menu_string[i].str);
	}

	menu_string_pos = 0;

}


void add_menu_button(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col, int button_notch_1, int button_notch_2)
{

	int m = vbuf.vertex_pos_triangle, n = vbuf.index_pos_triangle[0];

	vbuf.buffer_triangle[m].x = xa;
	vbuf.buffer_triangle[m].y = ya + button_notch_1;
	vbuf.buffer_triangle[m].color = col;
	vbuf.buffer_triangle[m+1].x = xa + button_notch_1;
	vbuf.buffer_triangle[m+1].y = ya;
	vbuf.buffer_triangle[m+1].color = col;
	vbuf.buffer_triangle[m+2].x = xb - button_notch_2;
	vbuf.buffer_triangle[m+2].y = ya;
	vbuf.buffer_triangle[m+2].color = col;
	vbuf.buffer_triangle[m+3].x = xb;
	vbuf.buffer_triangle[m+3].y = ya + button_notch_2;
	vbuf.buffer_triangle[m+3].color = col;
	vbuf.buffer_triangle[m+4].x = xb;
	vbuf.buffer_triangle[m+4].y = yb - button_notch_1;
	vbuf.buffer_triangle[m+4].color = col;
	vbuf.buffer_triangle[m+5].x = xb - button_notch_1;
	vbuf.buffer_triangle[m+5].y = yb;
	vbuf.buffer_triangle[m+5].color = col;
	vbuf.buffer_triangle[m+6].x = xa + button_notch_2;
	vbuf.buffer_triangle[m+6].y = yb;
	vbuf.buffer_triangle[m+6].color = col;
	vbuf.buffer_triangle[m+7].x = xa;
	vbuf.buffer_triangle[m+7].y = yb - button_notch_2;
	vbuf.buffer_triangle[m+7].color = col;

	vbuf.index_triangle[0][n++] = m+0;
	vbuf.index_triangle[0][n++] = m+1;
	vbuf.index_triangle[0][n++] = m+7;
	vbuf.index_triangle[0][n++] = m+1;
	vbuf.index_triangle[0][n++] = m+2;
	vbuf.index_triangle[0][n++] = m+7;
	vbuf.index_triangle[0][n++] = m+2;
	vbuf.index_triangle[0][n++] = m+6;
	vbuf.index_triangle[0][n++] = m+7;
	vbuf.index_triangle[0][n++] = m+2;
	vbuf.index_triangle[0][n++] = m+3;
	vbuf.index_triangle[0][n++] = m+6;
	vbuf.index_triangle[0][n++] = m+3;
	vbuf.index_triangle[0][n++] = m+5;
	vbuf.index_triangle[0][n++] = m+6;
	vbuf.index_triangle[0][n++] = m+3;
	vbuf.index_triangle[0][n++] = m+4;
	vbuf.index_triangle[0][n++] = m+5;

	vbuf.vertex_pos_triangle += 8;
	vbuf.index_pos_triangle[0] += 18;

 check_button_buffer();

}

void add_menu_rectangle(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col)
{

	add_menu_quad(xa, ya, xb, ya, xb, yb, xa, yb, col);

}

void add_menu_quad(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col)
{

	int m = vbuf.vertex_pos_triangle, n = vbuf.index_pos_triangle[0];

	vbuf.buffer_triangle[m].x = xa;
	vbuf.buffer_triangle[m].y = ya;
	vbuf.buffer_triangle[m].color = col;
	vbuf.buffer_triangle[m+1].x = xb;
	vbuf.buffer_triangle[m+1].y = yb;
	vbuf.buffer_triangle[m+1].color = col;
	vbuf.buffer_triangle[m+2].x = xc;
	vbuf.buffer_triangle[m+2].y = yc;
	vbuf.buffer_triangle[m+2].color = col;
	vbuf.buffer_triangle[m+3].x = xd;
	vbuf.buffer_triangle[m+3].y = yd;
	vbuf.buffer_triangle[m+3].color = col;

	vbuf.index_triangle[0][n++] = m+0;
	vbuf.index_triangle[0][n++] = m+1;
	vbuf.index_triangle[0][n++] = m+2;
	vbuf.index_triangle[0][n++] = m+2;
	vbuf.index_triangle[0][n++] = m+3;
	vbuf.index_triangle[0][n++] = m+0;

	vbuf.vertex_pos_triangle += 4;
	vbuf.index_pos_triangle[0] += 6;

 check_button_buffer();

}



static void check_button_buffer(void)
{
 check_vbuf();
// if (poly_pos >= POLY_TRIGGER)
//			draw_button_buffer();

}

void draw_button_buffer(void)
{


 draw_vbuf();
//  al_draw_prim(poly_buffer, NULL, NULL, 0, poly_pos, ALLEGRO_PRIM_TRIANGLE_LIST);
//  poly_pos = 0;

}


void draw_menu_buttons(void)
{
	draw_button_buffer();
	draw_menu_strings();
 reset_i_buttons();
}

















