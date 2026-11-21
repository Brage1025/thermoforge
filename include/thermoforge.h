
#ifndef THERMOFORGE_H
#define THERMOFORGE_H

#include <stddef.h>
#include <stdio.h>

/*
 * ThermoForge
 * Scientific heat diffusion simulation library.
 *
 * All physical quantities use consistent user-selected units.
 */

#define TF_VERSION "1.0.0"
#define TF_MAX_PATH 256

/* Simulation configuration */
typedef struct
{
    size_t nx;
    size_t ny;

    double width;
    double height;

    double diffusivity;

    double initial_temperature;
    double boundary_temperature;

    double source;

    double final_time;
    double dt;

    size_t steps_per_snapshot;

    char output_dir[TF_MAX_PATH];

} Config;


/* Two-dimensional computational grid */
typedef struct
{
    size_t nx;
    size_t ny;

    double dx;
    double dy;

    double *current;
    double *next;

} Grid;


/* Statistical properties of the temperature field */
typedef struct
{
    double minimum;
    double maximum;
    double mean;

    double energy_proxy;

} Diagnostics;


/* Configuration functions */
void config_defaults(Config *cfg);

int config_load(
    const char *path,
    Config *cfg,
    char *error,
    size_t error_size
);

int config_validate(
    Config *cfg,
    char *error,
    size_t error_size
);


/* Grid management */
int grid_create(
    Grid *grid,
    size_t nx,
    size_t ny,
    double width,
    double height
);

void grid_destroy(Grid *grid);

void grid_initialize(
    Grid *grid,
    double interior,
    double boundary
);

void grid_apply_boundary(
    Grid *grid,
    double boundary
);

void grid_swap(Grid *grid);

double grid_get(
    const Grid *grid,
    size_t i,
    size_t j
);

void grid_set(
    Grid *grid,
    size_t i,
    size_t j,
    double value
);


/* Numerical solver */
double solver_stable_dt(const Config *cfg);

int solver_run(
    Grid *grid,
    const Config *cfg,
    FILE *history,
    size_t *steps_done
);


/* Diagnostics */
Diagnostics diagnostics_compute(const Grid *grid);


/* Output handling */
int output_write_field(
    const Grid *grid,
    const char *directory,
    size_t step
);

int output_write_summary(
    const Config *cfg,
    const Diagnostics *diag,
    size_t steps,
    double actual_time,
    const char *directory
);

int output_write_history_header(FILE *stream);

#endif
