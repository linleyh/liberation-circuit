
#ifndef H_M_MATHS
#define H_M_MATHS



void init_trig(void);

int xpart(int angle, int length);
int ypart(int angle, int length);

float fxpart(float angle, float length);
float fypart(float angle, float length);

al_fixed symmetrical_sin(al_fixed angle);
al_fixed symmetrical_cos(al_fixed angle);

/*
inline float fxpart(float angle, float length);
inline float fypart(float angle, float length);

inline float fxpart(float angle, float length)
{
 return (cos(angle) * length);
}

inline float fypart(float angle, float length)
{
 return (sin(angle) * length);
}
*/


float angle_to_radians(int angle);
int radians_to_angle(double angle);

int angle_difference_int(int a1, int a2);
int angle_difference_signed_int(int a1, int a2);

int get_angle8(int y, int x);
int get_angle_coord(int x, int y, int x2, int y2);

int delta_turn_towards_angle(int angle, int tangle, int turning);

//void init_nearby_distance(void);
//int nearby_distance(int xa, int ya, int xb, int yb);

al_fixed fixed_xpart(al_fixed angle, al_fixed length);
al_fixed fixed_ypart(al_fixed angle, al_fixed length);

al_fixed distance(al_fixed y, al_fixed x);
al_fixed get_angle(al_fixed y, al_fixed x);
void fix_fixed_angle(al_fixed* fix_angle);
al_fixed get_fixed_fixed_angle(al_fixed fix_angle);
al_fixed angle_difference(al_fixed a1, al_fixed a2);
al_fixed angle_difference_signed(al_fixed a1, al_fixed a2);
float fixed_to_radians(al_fixed fix_angle);

al_fixed distance_oct(al_fixed y, al_fixed x);
al_fixed distance_oct_xyxy(al_fixed x1, al_fixed y1, al_fixed x2, al_fixed y2);
int distance_oct_int(int x, int y);

int get_angle_int(int y, int x);
int fixed_to_block(al_fixed fix_num);
al_fixed block_to_fixed(int block);
s16b fixed_angle_to_short(al_fixed fix_angle);
al_fixed short_angle_to_fixed(s16b short_angle);
al_fixed int_angle_to_fixed(int int_angle);
int fixed_angle_to_int(al_fixed fix_angle);

cart cart_plus_vector(cart base_position, al_fixed angle, al_fixed length);
void add_vector_to_cart(cart* base_position, al_fixed angle, al_fixed length);
cart cart_plus_xy(cart base_position, al_fixed x, al_fixed y);
polar xy_to_polar(al_fixed x, al_fixed y);
block_cart cart_to_block(cart position);
int verify_block_position(block_cart block_position);
al_fixed angle_from_cart_to_cart(cart cart1, cart cart2);
al_fixed distance_from_cart_to_cart(cart cart1, cart cart2);

al_fixed fixed_cos(al_fixed x);
al_fixed fixed_sin(al_fixed x);

#endif
