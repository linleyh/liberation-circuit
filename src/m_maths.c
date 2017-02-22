#include <allegro5/allegro.h>

/*

Some of the code in this file is a reimplementation of parts of Allegro 5's fixed point maths code,
mostly to give greater precision (or at least smoother results) than the original code.

fixed point maths is used instead of floating point to avoid imprecision (as the game
needs to run in the same way on multiple computers, and I think using integer-based
maths improves the chances of that)


*/



#include "m_config.h"
//#include "allegro.h"

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "m_globvars.h"
#include "g_header.h"
#include "g_misc.h"
#include "m_maths.h"



int turn_towards_angle(int angle, int tangle, int turning);
int turn_towards_angle_forbid(int angle, int tangle, int turning, int forbid);
static al_fixed fixed_atan2(al_fixed y, al_fixed x);

al_fixed fix_abs(al_fixed num);


// 10 is 4x the precision of the built-in Allegro fixed tan
#define TAN_PRECISION 10
#define TAN_TABLE_SIZE (1<<TAN_PRECISION)
#define TAN_TABLE_MASK (TAN_TABLE_SIZE-1)

#define TAN_MULTIPLIER (1<<(TAN_PRECISION-8))
// 8 is there because al_fixed angles are 0-255
const al_fixed fixed_tan_tbl [TAN_TABLE_SIZE];

#define IC_TRIG_PRECISION 15
#define IC_FIXED_COS_TABLE_SIZE (1 << ((32-IC_TRIG_PRECISION) - 8))
#define IC_TRIG_MASK (IC_FIXED_COS_TABLE_SIZE - 1)
#define IC_SIN_ADJUSTMENT

const al_fixed fixed_cos_tbl [IC_FIXED_COS_TABLE_SIZE + 1];


al_fixed fixed_cos(al_fixed x)
{

 al_fixed range_between_table_entries = (fixed_cos_tbl[(((x + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK) + 1]
          -  fixed_cos_tbl[((x + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK]);

 al_fixed interpolation_proportion = al_fixdiv((x + 0x4000) & ((1<<IC_TRIG_PRECISION)-1), ((1<<IC_TRIG_PRECISION)-1));

 al_fixed interpolation_amount = al_fixmul(range_between_table_entries, interpolation_proportion);


 return fixed_cos_tbl[((x + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK]
   + interpolation_amount;

}

al_fixed fixed_sin(al_fixed x)
{

 al_fixed range_between_table_entries = (fixed_cos_tbl[(((x - 0x400000 + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK) + 1]
          -  fixed_cos_tbl[((x - 0x400000 + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK]);

 al_fixed interpolation_proportion = al_fixdiv((x - 0x400000 + 0x4000) & ((1<<IC_TRIG_PRECISION)-1), ((1<<IC_TRIG_PRECISION)-1));

 al_fixed interpolation_amount = al_fixmul(range_between_table_entries, interpolation_proportion);

 return fixed_cos_tbl[((x - 0x400000 + 0x4000) >> IC_TRIG_PRECISION) & IC_TRIG_MASK]
   + interpolation_amount;

}

void init_trig(void)
{


//#define SAVE_TAN_TBL

#ifdef SAVE_TAN_TBL
// This code was used to generate the text for the definition of fixed_tan_tbl below.

#define TANFILE_SIZE 4096
#define TANFILE_WORD_LENGTH 20

 FILE *initfile;
 char buffer [20];

 initfile = fopen("tantbl.txt", "wt");

 if (!initfile)
 {
  fprintf(stdout, "\nFailed to open tantbl.txt. Starting with default settings.");
  error_call();
 }

 for (i = 0; i < TAN_TABLE_SIZE; i ++)
	{
		if (i % 8 == 0)
		 sprintf(buffer, "\n%iL, ", fixed_tan_tbl [i]);
		  else
		   sprintf(buffer, "%iL, ", fixed_tan_tbl [i]);

		fwrite(buffer, sizeof(char), strlen(buffer), initfile);

	}


 fclose(initfile);



#endif





}

// Because this function uses floating point, it can't be used for anything that affects gameplay
// It's just for clouds and display functions (although it may not actually be used at all).
// Use fixed_xpart instead.
int xpart(int angle, int length)
{
 return cos(angle_to_radians(angle)) * length;


}

int ypart(int angle, int length)
{
 return sin(angle_to_radians(angle)) * length;

}


float fxpart(float angle, float length)
{
 return (cos(angle) * length);
}

float fypart(float angle, float length)
{
 return (sin(angle) * length);
}

// because this function uses only fixed point, it can be used for gameplay
al_fixed fixed_xpart(al_fixed angle, al_fixed length)
{
 return al_fixmul(fixed_cos(angle), length);
}

al_fixed fixed_ypart(al_fixed angle, al_fixed length)
{
 return al_fixmul(fixed_sin(angle), length);
}



// use this function for working out sin when it needs to be mirrored properly for opposite angles
// e.g. torque exerted by objects on either side of a process
//  and mirrored object angles
// without this, fixed_sin returns asymmetrical rounded values.
al_fixed symmetrical_sin(al_fixed angle)
{
	angle &= AFX_MASK;

	if (angle < AFX_ANGLE_2)
		return fixed_sin(angle);

	angle = AFX_ANGLE_1 - angle;

	return fixed_sin(angle) * -1;

}

al_fixed symmetrical_cos(al_fixed angle)
{
	angle &= AFX_MASK;

	if (angle < AFX_ANGLE_2)
		return fixed_cos(angle);

	angle = AFX_ANGLE_1 - angle;

	return fixed_cos(angle);

}


float angle_to_radians(int angle)
{
 return ((float) angle * PI * 2) / ANGLE_1;
}

int radians_to_angle(double angle)
{
 return angle * (ANGLE_2/PI);
}

// returns smaller difference between two angles
// is run through abs so is always +ve (see angle_difference_signed)
al_fixed angle_difference(al_fixed a1, al_fixed a2)
{
 al_fixed d1, d2;

 a1 &= AFX_MASK;
 a2 &= AFX_MASK;

 d1 = (a1 - a2 + AFX_ANGLE_1);
 if (d1 >= AFX_ANGLE_1)
  d1 -= AFX_ANGLE_1;
 d2 = (a2 - a1 + AFX_ANGLE_1);
 if (d2 >= AFX_ANGLE_1)
  d2 -= AFX_ANGLE_1;

 if (d1 < d2)
  return abs(d1);

  return abs(d2);

}


// returns smaller difference between two angles
// is run through abs so is always +ve (see angle_difference_signed)
al_fixed angle_difference_signed(al_fixed a1, al_fixed a2)
{

// to do: optimise!!!

 a1 &= AFX_MASK;
 a2 &= AFX_MASK;

 if (a2 > a1)
	{
		if (a2 > a1 + AFX_ANGLE_2)
		{
		 return	a2 - (AFX_ANGLE_1 + a1);
		}
		 else
			{
				return a2 - a1;
			}
	}

		if (a1 > a2 + AFX_ANGLE_2)
		{
   return (AFX_ANGLE_1 - a1) + a2;
		}
		 else
			{
				return a2 - a1;
			}




 al_fixed d1, d2;


 d1 = (a1 - a2 + AFX_ANGLE_1);
 if (d1 > AFX_ANGLE_1)
  d1 -= AFX_ANGLE_1;
 d2 = (a2 - a1 + AFX_ANGLE_1);
 if (d2 > AFX_ANGLE_1)
  d2 -= AFX_ANGLE_1;

 if (d1 < d2)
  return d1;

  return d2;
}


al_fixed fix_abs(al_fixed num)
{
 if (num < al_fixtoi(-1))
  num *= -1;
 return num;
}

// returns smaller difference between two angles
// is run through abs so is always +ve (see angle_difference_signed)
int angle_difference_int(int a1, int a2)
{
 int d1, d2;

 a1 &= AFX_MASK;
 a2 &= AFX_MASK;

 d1 = (a1 - a2 + ANGLE_1) & ANGLE_MASK;
 d2 = (a2 - a1 + ANGLE_1) & ANGLE_MASK;

 if (d1 < d2)
  return abs(d1) & ANGLE_MASK;

  return abs(d2) & ANGLE_MASK;
}

int angle_difference_signed_int(int a1, int a2)
{

 unsigned int d1;

 a1 &= AFX_MASK;
 a2 &= AFX_MASK;

 d1 = (a2 - a1) & ANGLE_MASK;

 if (d1 > ANGLE_2)
  return -ANGLE_1 + d1;

 return d1;

}

al_fixed fixed_sqrt(al_fixed number);
int integer_sqrt(uint64_t x);

// works out an octagonal distance. Reasonably accurate and much faster than hypot.
al_fixed distance_oct(al_fixed y, al_fixed x)
{
 y = abs(y);
 x = abs(x);

 if (x > y)
		return al_fixmul(y, 26870) + al_fixmul(x, 61685);

	return al_fixmul(y, 61685) + al_fixmul(x, 26870);

// 26870 is al_ftofix(0.41)
// 61685 is al_ftofix(0.94124)

}

al_fixed distance_oct_xyxy(al_fixed x1, al_fixed y1, al_fixed x2, al_fixed y2)
{

 al_fixed dist_x = abs(x2 - x1);
 al_fixed dist_y = abs(y2 - y1);

 if (dist_x > dist_y)
		return al_fixmul(dist_y, 26870) + al_fixmul(dist_x, 61685);

	return al_fixmul(dist_y, 61685) + al_fixmul(dist_x, 26870);

}

int distance_oct_int(int x, int y)
{
	return al_fixtoi(distance_oct(al_itofix(y), al_itofix(x)));

}

al_fixed distance(al_fixed y, al_fixed x)
{

	if (y == 0
		&& x == 0)
			return 0;

	if (al_fixtoi(y) * al_fixtoi(y) + al_fixtoi(x) * al_fixtoi(x) > 32000)
  return al_itofix(integer_sqrt(al_fixtoi(y) * al_fixtoi(y) + al_fixtoi(x) * al_fixtoi(x)));

 return fixed_sqrt(al_fixmul(y, y) + al_fixmul(x, x));
}



al_fixed fixed_sqrt(al_fixed number)
{
    uint32_t t, q, b, r;

    r = number;
    b = 0x40000000;
    q = 0;

    while (b > 0x40)
    {
      t = q + b;

      if (r >= t)
      {
        r -= t;
        q = t + b; // equivalent to q += 2*b
      }

      r <<= 1;
      b >>= 1;
    }

    q >>= 8;

    return q;
}

int integer_sqrt(uint64_t x)
{

 uint64_t op, res, one;

 op = x;
 res = 0;

 one = 1 << 30;

 while (one > op)
 {
		one >>= 2;
 }

 while (one != 0)
	{
   if (op >= res + one)
			{
     op -= res + one;
     res += one << 1;
   }
   res >>= 1;
   one >>= 2;
 }

 return res;
}


int get_angle_int(int y, int x)
{

 return fixed_angle_to_short(fixed_atan2(al_itofix(y), al_itofix(x))) & ANGLE_MASK;

}

al_fixed get_angle(al_fixed y, al_fixed x)
{
 al_fixed return_value = fixed_atan2(y, x) & AFX_MASK;
 return return_value;
}

void fix_fixed_angle(al_fixed* fix_angle)
{
 *fix_angle &= AFX_MASK;
 return;
}

al_fixed get_fixed_fixed_angle(al_fixed fix_angle)
{
 return fix_angle;// & AFX_MASK;
}


float fixed_to_radians(al_fixed fix_angle)
{
 return al_fixtof(al_fixmul(fix_angle, al_fixtorad_r));
}

// takes an x or y fixed coordinate and finds block value
// Doesn't bounds-check!
int fixed_to_block(al_fixed fix_num)
{
 int temp_int = al_fixtoi(fix_num);
 return temp_int / BLOCK_SIZE_PIXELS;

}

cart cart_plus_vector(cart base_position, al_fixed angle, al_fixed length)
{
	cart temp_cart;

	temp_cart.x = base_position.x + fixed_xpart(angle, length);
	temp_cart.y = base_position.y + fixed_ypart(angle, length);

	return temp_cart;

}

void add_vector_to_cart(cart* base_position, al_fixed angle, al_fixed length)
{
	base_position->x += fixed_xpart(angle, length);
	base_position->y += fixed_ypart(angle, length);
}

cart cart_plus_xy(cart base_position, al_fixed x, al_fixed y)
{
	cart temp_cart; // could I just use base_position instead of temp_cart? Not sure.
	temp_cart.x = base_position.x + x;
	temp_cart.y = base_position.y + y;
	return temp_cart;
}


block_cart cart_to_block(cart position)
{

 block_cart temp_block_cart;

 temp_block_cart.x = al_fixtoi(position.x) / BLOCK_SIZE_PIXELS;
 temp_block_cart.y = al_fixtoi(position.y) / BLOCK_SIZE_PIXELS;

 return temp_block_cart;

}


int verify_block_position(block_cart block_position)
{
	if (block_position.x < 2 || block_position.x >= w.blocks.x - 2
		|| block_position.y < 2 || block_position.y >= w.blocks.y - 2)
			return 0;
 return 1;
}

al_fixed angle_from_cart_to_cart(cart cart1, cart cart2)
{
	return get_angle(cart2.y - cart1.y, cart2.x - cart1.x);
}


al_fixed distance_from_cart_to_cart(cart cart1, cart cart2)
{
 return distance(cart2.y - cart1.y, cart2.x - cart1.x);
}


al_fixed block_to_fixed(int block)
{
 return al_itofix(block * BLOCK_SIZE_PIXELS);
}


polar xy_to_polar(al_fixed x, al_fixed y)
{
	polar temp_polar;

	temp_polar.angle = get_angle(y, x);
	temp_polar.magnitude = distance(y, x);

	return temp_polar;
}



#define AFX_ANGLE_TO_ANGLE (ANGLE_1 / 256)
#define FIXED_AFX_ANGLE_TO_ANGLE al_itofix(ANGLE_1/256)

s16b fixed_angle_to_short(al_fixed fix_angle)
{
 return al_fixtoi(al_fixmul(fix_angle, FIXED_AFX_ANGLE_TO_ANGLE));

}

int fixed_angle_to_int(al_fixed fix_angle)
{
 return al_fixtoi(fix_angle * AFX_ANGLE_TO_ANGLE);
}

al_fixed short_angle_to_fixed(s16b short_angle)
{
 return al_fixdiv(al_itofix(short_angle), al_itofix(AFX_ANGLE_TO_ANGLE)) & AFX_MASK;
}

al_fixed int_angle_to_fixed(int int_angle)
{
 return al_fixdiv(al_itofix(int_angle), al_itofix(AFX_ANGLE_TO_ANGLE)) & AFX_MASK;
}


int delta_turn_towards_angle(int angle, int tangle, int turning)
{

 if (angle == tangle)
  return 0;

 if ((angle < tangle && tangle > angle + ANGLE_2)
     || (angle > tangle && tangle > angle - ANGLE_2))
 {
  return turning * -1;
 }

 return turning;

}

const al_fixed fixed_cos_tbl [IC_FIXED_COS_TABLE_SIZE + 1] =
{
65536L, 65531L, 65516L, 65492L, 65457L, 65413L, 65358L, 65294L,
65220L, 65137L, 65043L, 64940L, 64827L, 64704L, 64571L, 64429L,
64277L, 64115L, 63944L, 63763L, 63572L, 63372L, 63162L, 62943L,
62714L, 62476L, 62228L, 61971L, 61705L, 61429L, 61145L, 60851L,
60547L, 60235L, 59914L, 59583L, 59244L, 58896L, 58538L, 58172L,
57798L, 57414L, 57022L, 56621L, 56212L, 55794L, 55368L, 54934L,
54491L, 54040L, 53581L, 53114L, 52639L, 52156L, 51665L, 51166L,
50660L, 50146L, 49624L, 49095L, 48559L, 48015L, 47464L, 46906L,
46341L, 45769L, 45190L, 44604L, 44011L, 43412L, 42806L, 42194L,
41576L, 40951L, 40320L, 39683L, 39040L, 38391L, 37736L, 37076L,
36410L, 35738L, 35062L, 34380L, 33692L, 33000L, 32303L, 31600L,
30893L, 30182L, 29466L, 28745L, 28020L, 27291L, 26558L, 25821L,
25080L, 24335L, 23586L, 22834L, 22078L, 21320L, 20557L, 19792L,
19024L, 18253L, 17479L, 16703L, 15924L, 15143L, 14359L, 13573L,
12785L, 11996L, 11204L, 10411L, 9616L, 8820L, 8022L, 7224L,
6424L, 5623L, 4821L, 4019L, 3216L, 2412L, 1608L, 804L,
0L, -804L, -1608L, -2412L, -3216L, -4019L, -4821L, -5623L,
-6424L, -7224L, -8022L, -8820L, -9616L, -10411L, -11204L, -11996L,
-12785L, -13573L, -14359L, -15143L, -15924L, -16703L, -17479L, -18253L,
-19024L, -19792L, -20557L, -21320L, -22078L, -22834L, -23586L, -24335L,
-25080L, -25821L, -26558L, -27291L, -28020L, -28745L, -29466L, -30182L,
-30893L, -31600L, -32303L, -33000L, -33692L, -34380L, -35062L, -35738L,
-36410L, -37076L, -37736L, -38391L, -39040L, -39683L, -40320L, -40951L,
-41576L, -42194L, -42806L, -43412L, -44011L, -44604L, -45190L, -45769L,
-46341L, -46906L, -47464L, -48015L, -48559L, -49095L, -49624L, -50146L,
-50660L, -51166L, -51665L, -52156L, -52639L, -53114L, -53581L, -54040L,
-54491L, -54934L, -55368L, -55794L, -56212L, -56621L, -57022L, -57414L,
-57798L, -58172L, -58538L, -58896L, -59244L, -59583L, -59914L, -60235L,
-60547L, -60851L, -61145L, -61429L, -61705L, -61971L, -62228L, -62476L,
-62714L, -62943L, -63162L, -63372L, -63572L, -63763L, -63944L, -64115L,
-64277L, -64429L, -64571L, -64704L, -64827L, -64940L, -65043L, -65137L,
-65220L, -65294L, -65358L, -65413L, -65457L, -65492L, -65516L, -65531L,
-65536L, -65531L, -65516L, -65492L, -65457L, -65413L, -65358L, -65294L,
-65220L, -65137L, -65043L, -64940L, -64827L, -64704L, -64571L, -64429L,
-64277L, -64115L, -63944L, -63763L, -63572L, -63372L, -63162L, -62943L,
-62714L, -62476L, -62228L, -61971L, -61705L, -61429L, -61145L, -60851L,
-60547L, -60235L, -59914L, -59583L, -59244L, -58896L, -58538L, -58172L,
-57798L, -57414L, -57022L, -56621L, -56212L, -55794L, -55368L, -54934L,
-54491L, -54040L, -53581L, -53114L, -52639L, -52156L, -51665L, -51166L,
-50660L, -50146L, -49624L, -49095L, -48559L, -48015L, -47464L, -46906L,
-46341L, -45769L, -45190L, -44604L, -44011L, -43412L, -42806L, -42194L,
-41576L, -40951L, -40320L, -39683L, -39040L, -38391L, -37736L, -37076L,
-36410L, -35738L, -35062L, -34380L, -33692L, -33000L, -32303L, -31600L,
-30893L, -30182L, -29466L, -28745L, -28020L, -27291L, -26558L, -25821L,
-25080L, -24335L, -23586L, -22834L, -22078L, -21320L, -20557L, -19792L,
-19024L, -18253L, -17479L, -16703L, -15924L, -15143L, -14359L, -13573L,
-12785L, -11996L, -11204L, -10411L, -9616L, -8820L, -8022L, -7224L,
-6424L, -5623L, -4821L, -4019L, -3216L, -2412L, -1608L, -804L,
0L, 804L, 1608L, 2412L, 3216L, 4019L, 4821L, 5623L,
6424L, 7224L, 8022L, 8820L, 9616L, 10411L, 11204L, 11996L,
12785L, 13573L, 14359L, 15143L, 15924L, 16703L, 17479L, 18253L,
19024L, 19792L, 20557L, 21320L, 22078L, 22834L, 23586L, 24335L,
25080L, 25821L, 26558L, 27291L, 28020L, 28745L, 29466L, 30182L,
30893L, 31600L, 32303L, 33000L, 33692L, 34380L, 35062L, 35738L,
36410L, 37076L, 37736L, 38391L, 39040L, 39683L, 40320L, 40951L,
41576L, 42194L, 42806L, 43412L, 44011L, 44604L, 45190L, 45769L,
46341L, 46906L, 47464L, 48015L, 48559L, 49095L, 49624L, 50146L,
50660L, 51166L, 51665L, 52156L, 52639L, 53114L, 53581L, 54040L,
54491L, 54934L, 55368L, 55794L, 56212L, 56621L, 57022L, 57414L,
57798L, 58172L, 58538L, 58896L, 59244L, 59583L, 59914L, 60235L,
60547L, 60851L, 61145L, 61429L, 61705L, 61971L, 62228L, 62476L,
62714L, 62943L, 63162L, 63372L, 63572L, 63763L, 63944L, 64115L,
64277L, 64429L, 64571L, 64704L, 64827L, 64940L, 65043L, 65137L,
65220L, 65294L, 65358L, 65413L, 65457L, 65492L, 65516L, 65531L,
65536L,
};




const al_fixed fixed_tan_tbl [TAN_TABLE_SIZE] =
{
0L, 201L, 402L, 603L, 804L, 1005L, 1207L, 1408L,
1609L, 1810L, 2011L, 2213L, 2414L, 2615L, 2817L, 3018L,
3220L, 3421L, 3623L, 3825L, 4026L, 4228L, 4430L, 4632L,
4834L, 5036L, 5239L, 5441L, 5644L, 5846L, 6049L, 6252L,
6455L, 6658L, 6861L, 7064L, 7268L, 7471L, 7675L, 7879L,
8083L, 8287L, 8492L, 8696L, 8901L, 9106L, 9311L, 9516L,
9721L, 9927L, 10133L, 10339L, 10545L, 10751L, 10958L, 11165L,
11372L, 11579L, 11786L, 11994L, 12202L, 12410L, 12618L, 12827L,
13036L, 13245L, 13454L, 13664L, 13874L, 14084L, 14295L, 14506L,
14717L, 14928L, 15140L, 15352L, 15564L, 15776L, 15989L, 16202L,
16416L, 16630L, 16844L, 17058L, 17273L, 17489L, 17704L, 17920L,
18136L, 18353L, 18570L, 18787L, 19005L, 19223L, 19442L, 19661L,
19880L, 20100L, 20320L, 20541L, 20762L, 20983L, 21205L, 21427L,
21650L, 21873L, 22097L, 22321L, 22546L, 22771L, 22997L, 23223L,
23449L, 23676L, 23904L, 24132L, 24360L, 24590L, 24819L, 25049L,
25280L, 25511L, 25743L, 25975L, 26208L, 26442L, 26676L, 26911L,
27146L, 27382L, 27618L, 27855L, 28093L, 28331L, 28570L, 28810L,
29050L, 29291L, 29533L, 29775L, 30018L, 30261L, 30506L, 30751L,
30996L, 31243L, 31490L, 31738L, 31986L, 32235L, 32486L, 32736L,
32988L, 33240L, 33494L, 33748L, 34002L, 34258L, 34514L, 34772L,
35030L, 35289L, 35548L, 35809L, 36071L, 36333L, 36596L, 36861L,
37126L, 37392L, 37659L, 37927L, 38196L, 38465L, 38736L, 39008L,
39281L, 39555L, 39829L, 40105L, 40382L, 40660L, 40939L, 41219L,
41500L, 41782L, 42066L, 42350L, 42636L, 42923L, 43210L, 43500L,
43790L, 44081L, 44374L, 44668L, 44963L, 45259L, 45557L, 45856L,
46156L, 46457L, 46760L, 47064L, 47369L, 47676L, 47984L, 48294L,
48605L, 48917L, 49231L, 49546L, 49863L, 50181L, 50501L, 50822L,
51145L, 51469L, 51795L, 52122L, 52451L, 52782L, 53114L, 53448L,
53784L, 54121L, 54460L, 54801L, 55144L, 55488L, 55834L, 56182L,
56532L, 56883L, 57237L, 57592L, 57949L, 58309L, 58670L, 59033L,
59398L, 59766L, 60135L, 60506L, 60880L, 61255L, 61633L, 62013L,
62395L, 62780L, 63167L, 63556L, 63947L, 64341L, 64737L, 65135L,
65536L, 65939L, 66345L, 66754L, 67165L, 67578L, 67994L, 68413L,
68835L, 69259L, 69686L, 70116L, 70548L, 70984L, 71422L, 71863L,
72308L, 72755L, 73206L, 73659L, 74116L, 74575L, 75039L, 75505L,
75974L, 76447L, 76924L, 77404L, 77887L, 78374L, 78864L, 79358L,
79856L, 80357L, 80863L, 81372L, 81885L, 82402L, 82922L, 83447L,
83977L, 84510L, 85047L, 85589L, 86135L, 86686L, 87241L, 87801L,
88365L, 88934L, 89508L, 90086L, 90670L, 91258L, 91852L, 92450L,
93054L, 93663L, 94277L, 94897L, 95523L, 96154L, 96791L, 97433L,
98082L, 98736L, 99396L, 100063L, 100736L, 101415L, 102101L, 102794L,
103493L, 104198L, 104911L, 105631L, 106358L, 107092L, 107834L, 108583L,
109340L, 110105L, 110877L, 111658L, 112447L, 113244L, 114050L, 114864L,
115687L, 116519L, 117360L, 118211L, 119071L, 119941L, 120820L, 121710L,
122609L, 123519L, 124440L, 125371L, 126314L, 127267L, 128232L, 129209L,
130198L, 131198L, 132211L, 133237L, 134276L, 135327L, 136393L, 137471L,
138564L, 139671L, 140793L, 141929L, 143081L, 144248L, 145431L, 146631L,
147847L, 149080L, 150330L, 151598L, 152884L, 154188L, 155512L, 156855L,
158218L, 159601L, 161005L, 162430L, 163878L, 165347L, 166839L, 168355L,
169896L, 171460L, 173050L, 174666L, 176309L, 177979L, 179677L, 181404L,
183161L, 184948L, 186766L, 188616L, 190499L, 192416L, 194368L, 196356L,
198380L, 200443L, 202544L, 204686L, 206869L, 209095L, 211366L, 213681L,
216043L, 218453L, 220914L, 223425L, 225990L, 228610L, 231286L, 234021L,
236816L, 239675L, 242598L, 245588L, 248647L, 251779L, 254986L, 258270L,
261634L, 265082L, 268616L, 272241L, 275959L, 279774L, 283691L, 287713L,
291845L, 296091L, 300457L, 304947L, 309567L, 314324L, 319222L, 324269L,
329471L, 334837L, 340373L, 346089L, 351993L, 358095L, 364404L, 370933L,
377693L, 384695L, 391955L, 399486L, 407305L, 415427L, 423871L, 432658L,
441807L, 451343L, 461290L, 471677L, 482533L, 493891L, 505786L, 518258L,
531351L, 545111L, 559592L, 574852L, 590956L, 607977L, 625995L, 645100L,
665396L, 686996L, 710033L, 734653L, 761027L, 789350L, 819846L, 852777L,
888446L, 927212L, 969495L, 1015798L, 1066724L, 1123004L, 1185531L, 1255406L,
1334007L, 1423079L, 1524866L, 1642302L, 1779299L, 1941193L, 2135452L, 2372864L,
2669612L, 3051108L, 3559763L, 4271848L, 5339934L, 7120008L, 10679997L, 21359102L,
2147483647L, -21363473L, -10681090L, -7120493L, -5340207L, -4272023L, -3559907L, -3051214L,
-2669680L, -2372918L, -2135495L, -1941229L, -1779329L, -1642328L, -1524888L, -1423098L,
-1334024L, -1255421L, -1185547L, -1123019L, -1066737L, -1015808L, -969504L, -927220L,
-888454L, -852784L, -819853L, -789356L, -761033L, -734658L, -710038L, -687002L,
-665401L, -645105L, -625999L, -607981L, -590960L, -574855L, -559595L, -545114L,
-531353L, -518261L, -505788L, -493893L, -482536L, -471680L, -461293L, -451345L,
-441809L, -432659L, -423873L, -415429L, -407306L, -399488L, -391957L, -384697L,
-377694L, -370935L, -364406L, -358096L, -351994L, -346090L, -340374L, -334838L,
-329472L, -324270L, -319223L, -314325L, -309568L, -304948L, -300458L, -296092L,
-291846L, -287714L, -283692L, -279775L, -275960L, -272242L, -268617L, -265083L,
-261635L, -258270L, -254986L, -251780L, -248648L, -245588L, -242598L, -239675L,
-236817L, -234022L, -231287L, -228610L, -225991L, -223426L, -220914L, -218454L,
-216044L, -213681L, -211366L, -209096L, -206870L, -204687L, -202545L, -200443L,
-198381L, -196356L, -194368L, -192416L, -190499L, -188616L, -186766L, -184948L,
-183161L, -181405L, -179678L, -177980L, -176309L, -174667L, -173051L, -171461L,
-169896L, -168356L, -166840L, -165347L, -163878L, -162431L, -161005L, -159601L,
-158218L, -156855L, -155512L, -154189L, -152884L, -151598L, -150330L, -149080L,
-147847L, -146631L, -145432L, -144249L, -143081L, -141930L, -140793L, -139671L,
-138564L, -137472L, -136393L, -135328L, -134276L, -133237L, -132212L, -131199L,
-130198L, -129209L, -128232L, -127267L, -126314L, -125371L, -124440L, -123519L,
-122609L, -121710L, -120820L, -119941L, -119071L, -118211L, -117361L, -116519L,
-115687L, -114864L, -114050L, -113244L, -112447L, -111658L, -110878L, -110105L,
-109340L, -108583L, -107834L, -107093L, -106358L, -105631L, -104911L, -104199L,
-103493L, -102794L, -102101L, -101415L, -100736L, -100063L, -99397L, -98736L,
-98082L, -97433L, -96791L, -96154L, -95523L, -94897L, -94278L, -93663L,
-93054L, -92450L, -91852L, -91258L, -90670L, -90086L, -89508L, -88934L,
-88365L, -87801L, -87241L, -86686L, -86136L, -85589L, -85048L, -84510L,
-83977L, -83448L, -82923L, -82402L, -81885L, -81372L, -80863L, -80357L,
-79856L, -79358L, -78864L, -78374L, -77887L, -77404L, -76924L, -76447L,
-75975L, -75505L, -75039L, -74576L, -74116L, -73659L, -73206L, -72755L,
-72308L, -71864L, -71422L, -70984L, -70548L, -70116L, -69686L, -69259L,
-68835L, -68413L, -67994L, -67578L, -67165L, -66754L, -66345L, -65939L,
-65536L, -65135L, -64737L, -64341L, -63947L, -63556L, -63167L, -62780L,
-62396L, -62013L, -61633L, -61256L, -60880L, -60506L, -60135L, -59766L,
-59398L, -59033L, -58670L, -58309L, -57950L, -57592L, -57237L, -56883L,
-56532L, -56182L, -55834L, -55488L, -55144L, -54801L, -54460L, -54121L,
-53784L, -53448L, -53114L, -52782L, -52451L, -52122L, -51795L, -51469L,
-51145L, -50822L, -50501L, -50181L, -49863L, -49546L, -49231L, -48917L,
-48605L, -48294L, -47984L, -47676L, -47369L, -47064L, -46760L, -46457L,
-46156L, -45856L, -45557L, -45259L, -44963L, -44668L, -44374L, -44081L,
-43790L, -43500L, -43211L, -42923L, -42636L, -42350L, -42066L, -41783L,
-41500L, -41219L, -40939L, -40660L, -40382L, -40105L, -39829L, -39555L,
-39281L, -39008L, -38736L, -38465L, -38196L, -37927L, -37659L, -37392L,
-37126L, -36861L, -36596L, -36333L, -36071L, -35809L, -35548L, -35289L,
-35030L, -34772L, -34514L, -34258L, -34002L, -33748L, -33494L, -33240L,
-32988L, -32736L, -32486L, -32236L, -31986L, -31738L, -31490L, -31243L,
-30996L, -30751L, -30506L, -30261L, -30018L, -29775L, -29533L, -29291L,
-29050L, -28810L, -28570L, -28331L, -28093L, -27855L, -27618L, -27382L,
-27146L, -26911L, -26676L, -26442L, -26208L, -25975L, -25743L, -25511L,
-25280L, -25049L, -24819L, -24590L, -24360L, -24132L, -23904L, -23676L,
-23449L, -23223L, -22997L, -22771L, -22546L, -22321L, -22097L, -21873L,
-21650L, -21427L, -21205L, -20983L, -20762L, -20541L, -20320L, -20100L,
-19880L, -19661L, -19442L, -19223L, -19005L, -18787L, -18570L, -18353L,
-18136L, -17920L, -17704L, -17489L, -17273L, -17058L, -16844L, -16630L,
-16416L, -16202L, -15989L, -15776L, -15564L, -15352L, -15140L, -14928L,
-14717L, -14506L, -14295L, -14084L, -13874L, -13664L, -13455L, -13245L,
-13036L, -12827L, -12618L, -12410L, -12202L, -11994L, -11786L, -11579L,
-11372L, -11165L, -10958L, -10751L, -10545L, -10339L, -10133L, -9927L,
-9721L, -9516L, -9311L, -9106L, -8901L, -8696L, -8492L, -8287L,
-8083L, -7879L, -7675L, -7471L, -7268L, -7064L, -6861L, -6658L,
-6455L, -6252L, -6049L, -5846L, -5644L, -5441L, -5239L, -5036L,
-4834L, -4632L, -4430L, -4228L, -4026L, -3825L, -3623L, -3421L,
-3220L, -3018L, -2817L, -2615L, -2414L, -2213L, -2011L, -1810L,
-1609L, -1408L, -1207L, -1005L, -804L, -603L, -402L, -201L
};



static al_fixed fixed_atan2(al_fixed y, al_fixed x)
{
   al_fixed ratio_yx;

   int a, b, c;

   al_fixed d;

   if (x == 0)
			{
    if (y == 0)
    {
	    return 0;
    }
     else
					{
						if (y < 0)
							return -0x00400000L;
						  else
									return 0x00400000L;
					}
   }

   al_set_errno(0);
   ratio_yx = al_fixdiv(y, x);

   if (al_get_errno())
			{
				if (y < 0)
					return -0x00400000L;
				  else
							return 0x00400000L;
   }

   if (ratio_yx >= 0)
			{
     a = 0;
     b = (TAN_TABLE_SIZE>>1)-1;
   }
    else
    {
      a = (TAN_TABLE_SIZE>>1);
      b = TAN_TABLE_SIZE-1;
    }

   do
   {
      c = (a + b) >> 1;
      d = ratio_yx - fixed_tan_tbl [c];

      if (d > 0)
	      a = c + 1;
        else
        {
	        if (d < 0)
	         b = c - 1;
        }

   } while ((a <= b) && (d));

   al_fixed remainder_angle = 0;
   al_fixed range_between_table_entries = 0;
   al_fixed interpolation_amount = 0;

   if (d != 0)
   {
    if (ratio_yx > fixed_tan_tbl [c])
    {
     remainder_angle = ratio_yx - fixed_tan_tbl [c];
     range_between_table_entries = fixed_tan_tbl [c] - fixed_tan_tbl [(c+1) & TAN_TABLE_MASK];
     interpolation_amount = al_fixdiv(remainder_angle, range_between_table_entries);
    }
     else
     {
      remainder_angle = ratio_yx - fixed_tan_tbl [c];
      range_between_table_entries = fixed_tan_tbl [c] - fixed_tan_tbl [(c-1) & TAN_TABLE_MASK];
      interpolation_amount = al_fixdiv(remainder_angle, range_between_table_entries);
     }
   }

   if (ratio_yx >= 0)
    ratio_yx = (((long)c) << (15+8-TAN_PRECISION)) - interpolation_amount;
     else
      ratio_yx = ((-0x00800000L + (((long)c) << (15+8-TAN_PRECISION)))) - interpolation_amount;

   if (x >= 0)
    return ratio_yx;

   if (y >= 0)
    return 0x00800000L + ratio_yx;

   return ratio_yx - 0x00800000L;

}


