
#include "thermoforge.h"

#include <float.h>

/*
 * Calculate statistical properties of the current
 * temperature distribution.
 */
Diagnostics diagnostics_compute(const Grid *grid)
{
    Diagnostics result = {
        0.0,
        0.0,
        0.0,
        0.0
    };

    size_t count;
    size_t k;

    double sum = 0.0;

    if (grid == NULL ||
        grid->current == NULL ||
        grid->nx == 0 ||
        grid->ny == 0)
    {
        return result;
    }

    count = grid->nx * grid->ny;

    result.minimum = DBL_MAX;
    result.maximum = -DBL_MAX;

    for (k = 0; k < count; k++)
    {
        double value = grid->current[k];

        if (value < result.minimum)
        {
            result.minimum = value;
        }

        if (value > result.maximum)
        {
            result.maximum = value;
        }

        sum += value;
    }

    result.mean = sum / (double)count;

    /*
     * Energy proxy:
     * Integrated temperature multiplied by cell area.
     *
     * This is not physical thermal energy because
     * density and heat capacity are not included.
     */
    result.energy_proxy =
        sum * grid->dx * grid->dy;

    return result;
}
