
#include "thermoforge.h"

#include <math.h>
#include <stdio.h>

/*
 * Calculate the maximum stable time step.
 *
 * For the two-dimensional explicit diffusion equation:
 *
 * alpha * dt * (1/dx^2 + 1/dy^2) <= 0.5
 */
double solver_stable_dt(const Config *cfg)
{
    double dx;
    double dy;
    double denominator;

    if (cfg == NULL ||
        cfg->diffusivity <= 0.0 ||
        cfg->nx < 3 ||
        cfg->ny < 3)
    {
        return 0.0;
    }

    dx = cfg->width / (double)(cfg->nx - 1);
    dy = cfg->height / (double)(cfg->ny - 1);

    denominator = cfg->diffusivity *
        (1.0 / (dx * dx) + 1.0 / (dy * dy));

    if (denominator <= 0.0)
    {
        return 0.0;
    }

    return 0.5 / denominator;
}


/*
 * Advance the temperature field by one time step.
 *
 * The old temperature field is never modified during
 * the update. This avoids order-dependent calculations.
 */
static void advance_one_step(
    Grid *grid,
    const Config *cfg,
    double dt
)
{
    size_t i;
    size_t j;

    double rx =
        cfg->diffusivity * dt /
        (grid->dx * grid->dx);

    double ry =
        cfg->diffusivity * dt /
        (grid->dy * grid->dy);

    /* Apply boundary conditions to destination buffer */
    for (i = 0; i < grid->nx; i++)
    {
        grid->next[i] = cfg->boundary_temperature;

        grid->next[
            (grid->ny - 1) * grid->nx + i
        ] = cfg->boundary_temperature;
    }

    for (j = 0; j < grid->ny; j++)
    {
        grid->next[j * grid->nx] =
            cfg->boundary_temperature;

        grid->next[
            j * grid->nx + grid->nx - 1
        ] = cfg->boundary_temperature;
    }

    /* Update interior grid nodes */
    for (j = 1; j + 1 < grid->ny; j++)
    {
        for (i = 1; i + 1 < grid->nx; i++)
        {
            size_t p = j * grid->nx + i;

            double center = grid->current[p];

            double d2x =
                grid->current[p + 1]
                - 2.0 * center
                + grid->current[p - 1];

            double d2y =
                grid->current[p + grid->nx]
                - 2.0 * center
                + grid->current[p - grid->nx];

            grid->next[p] =
                center
                + rx * d2x
                + ry * d2y
                + dt * cfg->source;
        }
    }

    /* Exchange old and new fields */
    grid_swap(grid);
}


/* Write one diagnostics record */
static int write_history_row(
    FILE *history,
    double time,
    const Diagnostics *diag
)
{
    if (history == NULL)
    {
        return 0;
    }

    return fprintf(
        history,
        "%.12g,%.12g,%.12g,%.12g,%.12g\n",
        time,
        diag->minimum,
        diag->maximum,
        diag->mean,
        diag->energy_proxy
    ) < 0 ? -1 : 0;
}


/*
 * Run the complete simulation.
 *
 * Returns:
 *  0  success
 * -1  general solver error
 * -2  unstable requested time step
 */
int solver_run(
    Grid *grid,
    const Config *cfg,
    FILE *history,
    size_t *steps_done
)
{
    double stable_dt;
    double dt;
    double time = 0.0;

    size_t step = 0;

    Diagnostics diag;

    if (grid == NULL ||
        cfg == NULL ||
        grid->current == NULL)
    {
        return -1;
    }

    stable_dt = solver_stable_dt(cfg);

    if (stable_dt <= 0.0)
    {
        return -1;
    }

    /* Automatic time-step selection */
    dt = cfg->dt == 0.0
        ? 0.95 * stable_dt
        : cfg->dt;

    if (dt > stable_dt * (1.0 + 1.0e-12))
    {
        return -2;
    }

    /* Record initial conditions */
    diag = diagnostics_compute(grid);

    if (write_history_row(history, time, &diag) != 0)
    {
        return -1;
    }

    /* Main simulation loop */
    while (time < cfg->final_time)
    {
        double step_dt = dt;

        /* Shorten final step to reach exact final time */
        if (cfg->final_time - time < step_dt)
        {
            step_dt = cfg->final_time - time;
        }

        if (step_dt <= 0.0)
        {
            break;
        }

        advance_one_step(grid, cfg, step_dt);

        time += step_dt;
        step++;

        /* Periodically record diagnostics */
        if (step % cfg->steps_per_snapshot == 0 ||
            time >= cfg->final_time)
        {
            diag = diagnostics_compute(grid);

            if (write_history_row(history, time, &diag) != 0)
            {
                return -1;
            }
        }
    }

    if (steps_done != NULL)
    {
        *steps_done = step;
    }

    return 0;
}
