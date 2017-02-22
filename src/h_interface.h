
#ifndef H_H_INTERFACE
#define H_H_INTERFACE


// called when entering story interface from map
void init_story_interface(void);
void open_story_interface(void);
void draw_story_interface(void);
void story_input(void);
void draw_story_cutscene(int area_index, int counter, int counter_max);

void init_ending_cutscene(void);
void draw_ending_cutscene(int counter, int counter_max);


#endif
