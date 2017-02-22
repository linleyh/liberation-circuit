

#ifndef H_S_MISSION
#define H_S_MISSION

#ifdef OLD_MISSIONS

void prepare_for_mission(void);
void mission_spawn_extra_p1_processes(void);
void set_player_w_init_spawn_angle(int player_index, int nearby_data_well);

//int use_mission_system_file(int mission);

void save_mission_status_file(void);
void load_mission_status_file(void);

enum
{
MISSION_STATUS_UNFINISHED,
MISSION_STATUS_FINISHED,
//MISSION_STATUS_2STARS,
//MISSION_STATUS_3STARS,

MISSION_STATUSES // used for bounds-checking loaded values
};

enum
{
MISSION_TUTORIAL1,
MISSION_TUTORIAL2,
MISSION_TUTORIAL3,
MISSION_TUTORIAL4,
MISSION_MISSION1,
MISSION_MISSION2,
MISSION_MISSION3,
MISSION_MISSION4,
MISSION_MISSION5,
MISSION_MISSION6,
MISSION_MISSION7,
MISSION_MISSION8,
MISSION_ADVANCED1, // currently it is assumed that all advanced missions, and nothing else, come after this one
MISSION_ADVANCED2,
MISSION_ADVANCED3,
MISSION_ADVANCED4,
MISSION_ADVANCED5,
MISSION_ADVANCED6,
MISSION_ADVANCED7,
MISSION_ADVANCED8,
MISSIONS
};

struct missionsstruct
{
	int status [MISSIONS]; // MISSION_STATUS_* enum
	int locked [MISSIONS]; // 0 or 1
};

struct missionsstruct missions;


struct mission_state_struct
{
// extact meaning of all of these values depends on which mission is being played
	int phase; // currently used only for tutorials - keeps track of where the player is up to
	int reveal_player1; // shows player 1 on the map if p1 has no static processes left

};

#endif

#endif

