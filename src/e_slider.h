
#ifndef H_E_SLIDER
#define H_E_SLIDER


enum
{
SLIDEDIR_VERTICAL,
SLIDEDIR_HORIZONTAL
};

#define SLIDER_BUTTON_SIZE 18
#define SLIDER_MIN_SLIDER_SIZE 16

enum
{
SLIDER_HOLD_NONE,
SLIDER_HOLD_BUTTON,
SLIDER_HOLD_TRACK,
SLIDER_HOLD_SLIDER
};

struct slider_struct
{
// the following values should change only if the value being determined changes or something similar happens requiring the slider to be reinitialised
 int panel; // panel in which slider appears
 int subpanel; // subpanel
 int element; // slider's element

 int dir; // SLIDEDIR_VERTICAL or HORIZONTAL

 int value_min;
 int value_max;
 int slider_size; // size of slider in pixels

 float value_per_pixel; // can be less or more than 1

 int track_length; // length of track in pixels
 int total_length; // length of track + buttons

 int button_increment; // number of units increased or decreased if buttons at either end pressed (usually 1)
 int track_increment; // number of units increased or decreased if track clicked above or below slider

// the following values can change anytime the slider is used
 int value; // value being determined by slider (e.g. row position for a scrollbar)
 int* value_pointer; // pointer to location where actual value stored (this is updated along with value)
 float slider_pos; // position of the left/upper edge of the slider. Is float to avoid rounding errors

// the following values are updated by run_slider, which should be called each frame when a slider is in use
 int hold_type;
 int hold_delay; // used to put a delay between actions when button is being held on something
 int hold_offset;  // if slider grabbed, this is the point where it was grabbed
 int hold_base_value; // if slider grabbed, this is the value it had just beforehand (so it can be returned if the pointer is moved away)
// int hold_offset_fine; // fine-tunes the hold offset to make sure value doesn't snap to nearest whole value for slider_pos (complicated)

// the following are offsets from the subpanel - no longer used (use panel element values instead)
// int x1, y1, x2, y2;

 int colour; // meaning of this depends on slider type
 int highlighted; // uses a POINTER_* enum (including POINTER_OUTSIDE for no highlighting)
 int hidden_if_unused; // 1 or 0 - some slider types are hidden unless moused over or being dragged
 timestamp highlighted_time;
 int hidden; // 1 if currently hidden

};

//void init_slider(struct slider_struct* sl, int dir, int value_min, int value_max, int total_length, int button_increment, int slider_represents_size, int value, int x, int y, int thickness);
//void init_slider(struct slider_struct* sl, int* value_pointer, int dir, int value_min, int value_max, int total_length, int button_increment, int track_increment, int slider_represents_size, int x, int y, int thickness);
/*void init_slider(struct slider_struct* sl,
                 int* value_pointer,
                 int dir,
                 int value_min,
                 int value_max,
                 int total_length,
                 int button_increment,
                 int track_increment,
                 int slider_represents_size,
                 int x,
                 int y,
                 int thickness,
                 int colour,
                 int hidden_if_unused);*/


enum
{
SLIDER_LOG_SCROLLBAR_V,
SLIDER_DESIGN_SCROLLBAR_V,
SLIDER_DESIGN_SCROLLBAR_H,
SLIDER_EDITOR_SCROLLBAR_V,
SLIDER_EDITOR_SCROLLBAR_H,
SLIDER_BCODE_BCODE_SCROLLBAR_V,
SLIDER_BCODE_MEMORY_SCROLLBAR_V,
SLIDER_BCODE_TARGET_SCROLLBAR_V,
SLIDER_BCODE_STACK_SCROLLBAR_V,
SLIDERS
};

void init_slider(int s,
																	int pan,
																	int subpan,
																	int element,
																	int* value_pointer,
																	int dir,
																	int value_min,
																	int value_max,
																	int total_length,
																	int button_increment,
																	int track_increment,
																	int slider_represents_size,
//																	int x,
//																	int y,
																	int thickness,
																	int colour,
																	int hidden_if_unused);

enum
{
POINTER_OUTSIDE,
POINTER_BUTTON_LESS,
POINTER_BUTTON_MORE,
POINTER_TRACK_LESS,
POINTER_TRACK_MORE,
POINTER_SLIDER

};



void slider_button(struct slider_struct* sl, int sign);
//void check_slider_buttons(struct slider_struct* sl);
void run_slider(int s);
void slider_moved_to_value(struct slider_struct* sl, int new_value);
void reset_slider_length(int s, int new_total_length, int slider_represents_size);
void update_slider(int slider_index);

void draw_scrollbar(int s);
void draw_choosebar(struct slider_struct* sl, int x_offset, int y_offset);

#endif
