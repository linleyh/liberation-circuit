
#ifndef H_G_WORLD_BACK
#define H_G_WORLD_BACK


#define NODE_SPACING ((BLOCK_SIZE_PIXELS + 3) / 3)

void static_build_affects_block_nodes(al_fixed build_x, al_fixed build_y, int effect_size, int player_index);
void seed_mrand(unsigned int new_mrand_seed);
unsigned int mrand(unsigned int rand_max);

void fix_w_init_size(void);

#endif
