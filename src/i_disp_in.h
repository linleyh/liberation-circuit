
#ifndef H_I_DISP_IN
#define H_I_DISP_IN

void initialise_display(void);

void set_game_colours(int background_col, // index in back_and_hex_colours array
																						int hex_col, // index in back_and_hex_colours array
																						int players, // players in game
																						int player_base_cols [PLAYERS], // index in base_proc_col array
																						int player_packet_cols [PLAYERS]); // index in base_packet_colours array and similar interface array




enum
{
TEAM_COL_BLUE,
TEAM_COL_YELLOW,
TEAM_COL_GREEN,
TEAM_COL_WHITE,

TEAM_COL_PURPLE,
TEAM_COL_ORANGE,
TEAM_COL_RED,

TEAM_COLS

};

enum
{
PACKET_COL_YELLOW_ORANGE,
PACKET_COL_WHITE_BLUE,
PACKET_COL_WHITE_YELLOW,
PACKET_COL_WHITE_PURPLE,

PACKET_COL_ORANGE_RED,
PACKET_COL_BLUE_PURPLE,
PACKET_COL_ULTRAVIOLET,

PACKET_COLS
};

enum
{
BACK_COLS_BLUE,
BACK_COLS_GREEN,
BACK_COLS_YELLOW,
BACK_COLS_ORANGE,
BACK_COLS_PURPLE,
BACK_COLS_BLUE_DARK,
BACK_COLS_RED,

BACK_COLS

};

#endif

