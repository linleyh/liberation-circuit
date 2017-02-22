
#ifndef H_G_COMMAND
#define H_G_COMMAND


#define SELECT_MAX 32

#define SELECT_TERMINATE -1
// SELECT_TERMINATE
#define SELECT_EMPTY -2

enum
{
BUILD_MODE_NONE,
BUILD_MODE_PLACE,
BUILD_MODE_ANGLE

};

enum
{
SELECT_MODE_NONE,
SELECT_MODE_SINGLE_CORE, // can assume that selected_core [0] is valid and not SELECT_EMPTY or SELECT_TERMINATE
SELECT_MODE_MULTI_CORE, // can't assume that any selected core is valid (may be SELECT_EMPTY)
SELECT_MODE_DATA_WELL,

SELECT_MODES
};

#define CONTROL_GROUPS 7
#define POWER_DATA_RECORDS 50

struct command_struct
{
	int select_mode;
	int selected_core [SELECT_MAX + 1]; // need to null-terminate
	int selected_member; // only relevant if just one core selected.
 int selected_data_well; // only relevant in SELECT_MODE_DATA_WELL
 int select_box;
 al_fixed mouse_drag_world_x, mouse_drag_world_y;


 int build_mode;
 int builder_core_index;
 int display_build_buttons; // details of build buttons are stored in view_struct
 int build_template_index;
 cart build_position;
 al_fixed build_angle;
 al_fixed default_build_angle; // set for each build.
 int build_member_collision [GROUP_MAX_MEMBERS];
 int build_fail_collision; // 1 if any member is in collision
 int build_fail_edge; // 1 if any member is off map
 int build_fail_static; // 1 if core is static and on data well
 int build_fail_range; // 1 if core is static and target out of range

// int stress_record [POWER_DATA_RECORDS];
// int stress_level_record [POWER_DATA_RECORDS];
 int power_use_record [POWER_DATA_RECORDS];
 int power_fail_record [POWER_DATA_RECORDS];
 int power_use_pos;

 int control_group_core [CONTROL_GROUPS] [SELECT_MAX+1];
 int control_group_core_timestamp [CONTROL_GROUPS] [SELECT_MAX+1]; // unlike selected_core, cores are not automatically removed from control groups when they are destroyed.
 int last_control_group_selected; // selecting a control group twice quickly focuses on the control group
 timestamp last_control_group_select_time;


};


void init_commands(void);
void run_commands(void);

void remove_core_from_selection(int core_index);
void build_button_pressed(int template_index);
int check_proc_visible_to_user(int proc_index);
void clear_selection(void);
void clear_control_groups(void);

int add_command(struct core_struct* core, int command_type, int x, int y, int core_index, int member_index, int queued, int control_pressed);
s16b add_to_build_queue(int player_index, int builder_core_index, int template_index, int build_x, int build_y, int angle, int back_or_front, int repeat, int queue_for_this_core, int failure_message);
void clear_build_queue_for_core(int player_index, int remove_core_index);
void requeue_repeat_build(int player_index);
void build_queue_next(int player_index, int reset_build_queue_buttons);
void rearrange_build_queue(void);
int work_out_queue_drag_position(void);

#endif
