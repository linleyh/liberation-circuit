
#ifndef H_D_GEO
#define H_D_GEO

void update_design_member_position_recursively(struct template_struct* templ, int m);
int check_template_collisions(struct template_struct* templ);
int check_template_member_collision(struct template_struct* templ, int m);

int check_move_objects_obstruction(struct template_struct* templ);
int check_single_move_object_obstruction(struct template_struct* templ, int member_index, int object_index);

void update_design_member_positions(struct template_struct* templ);
void up_down_design_symmetry(void);

int get_link_dist_pixel(int base_dist, al_fixed link_angle_offset);

#endif
