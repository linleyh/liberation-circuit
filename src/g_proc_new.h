
#ifndef H_PROC_NEW
#define H_PROC_NEW


void init_template(struct template_struct* tpl, int player_index, int templ_index);
int create_new_from_template(struct template_struct* templ, int player_index, cart core_position, al_fixed group_angle, struct core_struct** collided_core);

void set_basic_group_properties(struct core_struct* core);
void calculate_move_object_properties(struct core_struct* group_core, struct proc_struct* proc, int object_index);
int add_notional_member_recursively(struct template_struct* templ, int member_index, cart new_position, al_fixed new_angle, int allow_failure);
s16b restore_component(struct core_struct* core, int player_index, int template_index, int member_index);

// This struct is set up with basic physical properties of procs
//  to allow collision detection to be done before a group is properly created.
struct notional_proc_struct
{
	int index; // -1 if notional proc doesn't exist
	cart position; // this is position in world
	block_cart block_position; // remember that notional procs are not on the blocklist
	al_fixed angle; // this is angle in world
	int shape;
};


#endif
