
#ifndef H_H_MISSION
#define H_H_MISSION



enum
{
MISSION_STATUS_UNFINISHED,
MISSION_STATUS_FINISHED,
//MISSION_STATUS_2STARS,
//MISSION_STATUS_3STARS,

MISSION_STATUSES // used for bounds-checking loaded values
};


struct mission_state_struct
{
// extact meaning of all of these values depends on which mission is being played
	int phase; // currently used only for tutorials - keeps track of where the player is up to
	int reveal_player1; // shows player 1 on the map if p1 has no static processes left

	union
	{
		int union_value1;
		int tutorial1_base_chatter;
	};

	union
	{
		int union_value2;
	};

	union
	{
		int union_value3;
	};

	union
	{
		int union_value4;
	};

	union
	{
		int union_value5;
	};

};

void mission_spawn_extra_processes(void);
void add_extra_spawn(int player_index, int template_index, int spawn_x_block, int spawn_y_block, int spawn_angle);
void prepare_for_mission(void);
void set_game_colours_for_area(int area_index, int players);

#endif
