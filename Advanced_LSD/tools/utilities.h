/* Some other utilities */

#include "../extern/beta_distribution/beta_distribution.hpp"
namespace utils
{
//create a random number generator that can be used alongside with the LSD one or instead.
auto util_prng = std::mt19937 {}; //alternative prng which can be used with all the c++11 stuff
sftrabbit::beta_distribution<> beta(2, 2); //default

}
#define INIT_UTILS_RND \
  utils::util_prng.seed(seed-1); \
  utils::util_prng.discard(800000);

#define INIT_UTILS_RNDX(new_seed) \
  utils::util_prng.seed(new_seed); \
  utils::util_prng.discard(800000);

#define SET_BETA(a, b) \
  utils::beta = sftrabbit::beta_distribution<>(a,b);

#define RND_BETA \
  utils::beta(utils::util_prng)


