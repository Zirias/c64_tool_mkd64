
#include "random.h"

#include <stdlib.h>
#include <time.h>

static int _initialized = 0;

int
random_num(int min, int max)
{
    if (!_initialized)
    {
	srand(time(NULL));
	_initialized = 1;
    }

    return (int) (rand() / ((double)RAND_MAX+1) * (max-min+1) + min);
}

