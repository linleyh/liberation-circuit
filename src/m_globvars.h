
#ifndef H_M_GLOBVARS
#define H_M_GLOBVARS


// this file contains variables that need to be available in every file
// they should be declared in main.c (which is the only file that shouldn't include this file)

extern struct settingsstruct settings;
extern struct inter_struct inter;

extern struct world_struct w;

extern struct ex_control_struct ex_control;
extern struct control_struct control;

#endif
