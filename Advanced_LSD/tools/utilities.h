/* Some other utilities */

/***************************************************
CUR_PRNG
Access the current prng from LSD to use it in other stl, etc, stuff
***************************************************/

extern minstd_rand lc;						// linear congruential generator
extern mt19937 mt32;						// Mersenne-Twister 32 bits generator
extern mt19937_64 mt64;					// Mersenne-Twister 64 bits generator
extern ranlux24 lf24;						// lagged fibonacci 24 bits generator
extern ranlux48 lf48;						// lagged fibonacci 48 bits generator

namespace utils
{
  //There is currently no need for external utilities.
}



