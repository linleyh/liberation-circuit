
#ifndef H_G_PROC
#define H_G_PROC

/*
void apply_packet_damage_to_proc(struct proc_struct* pr, int damage, int cause_team);
void hurt_proc(int p, int damage, int cause_team);
void virtual_method_break(struct proc_struct* pr);
void proc_explodes(struct proc_struct* pr, int destroyer_team);
int destroy_proc(struct proc_struct* pr);
*/

void apply_packet_damage_to_proc(struct proc_struct* pr, int damage, int cause_team, int cause_core_index, timestamp cause_core_timestamp);
void set_group_object_properties(struct core_struct* core);
void reset_group_after_composition_change(struct core_struct* core);
void hurt_proc(int p, int damage, int cause_team);
void core_proc_explodes(struct proc_struct* core_pr, int destroyer_team);

#endif
