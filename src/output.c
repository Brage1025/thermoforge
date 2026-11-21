
#include "thermoforge.h"

#include <stdio.h>
#include <string.h>

/* Construct a safe output path */
static int make_path(
    char *destination,
    size_t capacity,
    const char *directory,
    const char *filename
)
{
    int written = snprintf(
        destination,
        capacity,
        "%s/%s",
        directory,
        filename
    );

    if (written < 0 || (size_t)written >= capacity)
    {
        return -1;
    }

    return 0;
}


/* Export the complete temperature field as CSV */
int output_write_field(
    const Grid *grid,
    const char *directory,
    size_t step
)
{
    char filename[64];
    char path[512];

    FILE *output;

    size_t i;
    size_t j;

    if (grid == NULL || directory == NULL)
    {
        return -1;
    }

    snprintf(
        filename,
        sizeof(filename),
        "temperature_%06zu.csv",
        step
    );

    if (make_path(
        path,
        sizeof(path),
        directory,
        filename
    ) != 0)
    {
        return -1;
    }

    output = fopen(path, "w");

    if (output == NULL)
    {
        return -1;
    }

    /* Write x-coordinate header */
    for (i = 0; i < grid->nx; i++)
    {
        if (fprintf(
            output,
            "%s%.12g",
            i == 0 ? "" : ",",
            (double)i * grid->dx
        ) < 0)
        {
            fclose(output);
            return -1;
        }
    }

    fputc('\n', output);

    /* Write temperature values */
    for (j = 0; j < grid->ny; j++)
    {
        for (i = 0; i < grid->nx; i++)
        {
            if (fprintf(
                output,
                "%s%.12g",
                i == 0 ? "" : ",",
                grid_get(grid, i, j)
            ) < 0)
            {
                fclose(output);
                return -1;
            }
        }

        fputc('\n', output);
    }

    return fclose(output) == 0 ? 0 : -1;
}


/* Write simulation metadata and final diagnostics */
int output_write_summary(
    const Config *cfg,
    const Diagnostics *diag,
    size_t steps,
    double actual_time,
    const char *directory
)
{
    char path[512];

    FILE *output;

    if (make_path(
        path,
        sizeof(path),
        directory,
        "summary.txt"
    ) != 0)
    {
        return -1;
    }

    output = fopen(path, "w");

    if (output == NULL)
    {
        return -1;
    }

    fprintf(output, "ThermoForge Run Summary\n");
    fprintf(output, "=======================\n\n");

    fprintf(output, "Version:               %s\n", TF_VERSION);

    fprintf(output, "Grid:                  %zu x %zu\n",
            cfg->nx, cfg->ny);

    fprintf(output, "Domain:                %.8g x %.8g\n",
            cfg->width, cfg->height);

    fprintf(output, "Diffusivity:           %.12g\n",
            cfg->diffusivity);

    fprintf(output, "Requested final time:  %.12g\n",
            cfg->final_time);

    fprintf(output, "Actual final time:     %.12g\n",
            actual_time);

    fprintf(output, "Simulation steps:      %zu\n", steps);

    fprintf(output, "\nFinal Diagnostics\n");
    fprintf(output, "-----------------\n");

    fprintf(output, "Minimum temperature:   %.12g\n",
            diag->minimum);

    fprintf(output, "Maximum temperature:   %.12g\n",
            diag->maximum);

    fprintf(output, "Mean temperature:      %.12g\n",
            diag->mean);

    fprintf(output, "Energy proxy:          %.12g\n",
            diag->energy_proxy);

    fprintf(output,
            "\nNote: energy_proxy is not thermodynamic energy.\n");

    return fclose(output) == 0 ? 0 : -1;
}


/* Create the history CSV header */
int output_write_history_header(FILE *stream)
{
    if (stream == NULL)
    {
        return -1;
    }

    return fprintf(
        stream,
        "time,minimum,maximum,mean,energy_proxy\n"
    ) < 0 ? -1 : 0;
}
