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


     add_button_triangle(xa,
																									ya + button_notch_1,
																									xa + button_notch_1,
																									ya,
																									xa,
																									yb - button_notch_2,
																									col);

     add_button_triangle(xa + button_notch_1,
																									ya,
																									xa,
																									yb - button_notch_2,
																									xa + button_notch_2,
																									yb,
																									col);

     add_button_triangle(xa + button_notch_1,
																									ya,
																									xa + button_notch_2,
																									yb,
																									xb - button_notch_2,
																									ya,
																									col);

     add_button_triangle(xa + button_notch_2,
																									yb,
																									xb - button_notch_2,
																									ya,
																									xb - button_notch_1,
																									yb,
																									col);

     add_button_triangle(xb - button_notch_2,
																									ya,
																									xb - button_notch_1,
																									yb,
																									xb,
																									ya + button_notch_2,
																									col);

     add_button_triangle(xb - button_notch_1,
																									yb,
																									xb,
																									ya + button_notch_2,
																									xb,
																									yb - button_notch_1,
																									col);


 check_button_buffer();

}

void add_menu_rectangle(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col)
{

	     add_button_triangle(xa,
																									ya,
																									xb,
																									ya,
																									xa,
																									yb,
																									col);

	     add_button_triangle(xb,
																									ya,
																									xa,
																									yb,
																									xb,
																									yb,
																									col);

 check_button_buffer();

}

void add_menu_quad(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col)
{

	     add_button_triangle(xa,
																									ya,
																									xb,
																									yb,
																									xc,
																									yc,
																									col);

	     add_button_triangle(xd,
																									yd,
																									xb,
																									yb,
																									xc,
																									yc,
																									col);

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

static void add_button_triangle(float xa, float ya, float xb, float yb, float xc, float yc, ALLEGRO_COLOR col)
{

//fprintf(stdout, "\ntriangle %f,%f %f,%f %f,%f",
//								xa,ya,xb,yb,xc,yc);

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xa;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = ya;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [0] [vbuf.index_pos_triangle [0]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xb;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yb;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [0] [vbuf.index_pos_triangle [0]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;

	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].x = xc;
	vbuf.buffer_triangle[vbuf.vertex_pos_triangle].y = yc;
 vbuf.buffer_triangle[vbuf.vertex_pos_triangle].color = col;
 vbuf.index_triangle [0] [vbuf.index_pos_triangle [0]++] = vbuf.vertex_pos_triangle;
 vbuf.vertex_pos_triangle++;


/*
  poly_buffer [poly_pos].x = xa;
  poly_buffer [poly_pos].y = ya;
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = col;
  poly_pos ++;

  poly_buffer [poly_pos].x = xb;
  poly_buffer [poly_pos].y = yb;
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = col;
  poly_pos ++;

  poly_buffer [poly_pos].x = xc;
  poly_buffer [poly_pos].y = yc;
  poly_buffer [poly_pos].z = 0;
  poly_buffer [poly_pos].color = col;
  poly_pos ++;
*/
// check_buffer_sizes();

}

















