#include "random.h"

int random_seed = 0;
// pseudo-random integer generator
int maxrand(int seed, int max)
{
	random_seed = random_seed + seed * 1103515245 + 12345;
	return (uint32)(random_seed / 65536) % (max + 1);
}
