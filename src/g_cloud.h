
#ifndef H_G_CLOUD
#define H_G_CLOUD

void init_clouds(void);
struct cloud_struct* new_cloud(int type, int cloud_lifetime, al_fixed x, al_fixed y);
//void run_clouds(void);
int create_fragment(cart position, cart speed, int fragment_size, int explode_time, int lifetime, int colour);
void run_fragments(void);

#endif
