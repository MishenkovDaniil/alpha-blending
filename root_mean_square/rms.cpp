#include <stdio.h>
#include <math.h>

#include "rms.h"

double rms (double *values, size_t val_num, double avg)
{
    double quadro_sum = 0;

    for (size_t iter = 0; iter < val_num; ++iter)
    {
        quadro_sum += (avg - values[iter]) * (avg - values[iter]);
    }

    double result = sqrt((quadro_sum / val_num));

    return result;
}