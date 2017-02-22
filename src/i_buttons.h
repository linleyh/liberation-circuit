


#ifndef H_I_BUTTONS
#define H_I_BUTTONS


enum
{
MBUTTON_TYPE_MENU,
MBUTTON_TYPE_TEMPLATE,
MBUTTON_TYPE_SMALL, // single line buttons, like clear button on template
MBUTTON_TYPE_MODE,
MBUTTON_TYPE_MODE_HIGHLIGHT,
MBUTTON_TYPE_MISSION_STATUS,

};


#define MENU_STRINGS 120
#define MENU_STRING_LENGTH 40

void draw_button_buffer(void);
void reset_i_buttons(void);
void add_menu_button(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col, int button_notch_1, int button_notch_2);
void add_menu_rectangle(float xa, float ya, float xb, float yb, ALLEGRO_COLOR col);
void add_menu_quad(float xa, float ya, float xb, float yb, float xc, float yc, float xd, float yd, ALLEGRO_COLOR col);
void add_menu_string(int x, int y, ALLEGRO_COLOR* col, int align, int font_index, const char* str);
void draw_menu_strings(void);
void draw_menu_buttons(void);

#endif
