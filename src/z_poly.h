
// z_poly is my own process geometry editor.
// It's not supported for general use, as using to design new components requires substantial changes to the code.
//#define Z_POLY

#ifdef Z_POLY

void zshape_init(void);
void zshape_start(void);
void zshape_end(void);
void zshape_add_poly(int poly, int layer, int colour);
void zshape_add_vertex(int x, int y, int collision);
void zshape_add_link(int x, int y, int left_x, int left_y, int right_x, int right_y, int far_x, int far_y, int link_point_x, int link_point_y, int object_x, int object_y);
void zshape_add_fill_source(int x, int y);

#endif

