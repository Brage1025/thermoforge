
#include "thermoforge.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition)                         \
    do                                           \
    {                                            \
        if (!(condition))                        \
        {                                        \
            fprintf(stderr,                      \
                    "FAIL %s:%d: %s\n",           \
                    __FILE__,                    \
                    __LINE__,                    \
                    #condition);                 \
            return 1;                            \
        }                                        \
    } while (0)


/* Test computational grid allocation */
static int test_grid_allocation(void)
{
    Grid grid = {0};

    CHECK(grid_create(
        &grid,
        5,
        4,
        2.0,
        1.5
    ) == 0);

    CHECK(fabs(grid.dx - 0.5) < 1e-12);
    CHECK(fabs(grid.dy - 0.5) < 1e-12);

    grid_destroy(&grid);

    CHECK(grid.current == NULL);
    CHECK(grid.next == NULL);

    return 0;
}


/* Test fixed-temperature boundaries */
static int test_boundary_conditions(void)
{
    Grid grid = {0};

    CHECK(grid_create(
        &grid,
        5,
        5,
        1.0,
        1.0
    ) == 0);

    grid_initialize(
        &grid,
        20.0,
        100.0
    );

    CHECK(grid_get(&grid, 0, 0) == 100.0);
    CHECK(grid_get(&grid, 4, 0) == 100.0);
    CHECK(grid_get(&grid, 0, 4) == 100.0);
    CHECK(grid_get(&grid, 4, 4) == 100.0);

    CHECK(grid_get(&grid, 2, 2) == 20.0);

    grid_destroy(&grid);

    return 0;
}


/* Test statistical calculations */
static int test_diagnostics(void)
{
    Grid grid = {0};

    Diagnostics diag;

    CHECK(grid_create(
        &grid,
        3,
        3,
        2.0,
        2.0
    ) == 0);

    grid_initialize(&grid, 5.0, 5.0);

    diag = diagnostics_compute(&grid);

    CHECK(diag.minimum == 5.0);
    CHECK(diag.maximum == 5.0);
    CHECK(diag.mean == 5.0);

    CHECK(fabs(
        diag.energy_proxy - 45.0
    ) < 1e-12);

    grid_destroy(&grid);

    return 0;
}


/* Test theoretical stability limit */
static int test_stability_limit(void)
{
    Config cfg;

    config_defaults(&cfg);

    cfg.nx = 11;
    cfg.ny = 11;

    cfg.width = 1.0;
    cfg.height = 1.0;

    cfg.diffusivity = 0.1;

    /*
     * dx = dy = 0.1
     *
     * dt_max = 0.5 / (0.1 * (100 + 100))
     *        = 0.025
     */

    CHECK(fabs(
        solver_stable_dt(&cfg) - 0.025
    ) < 1e-12);

    return 0;
}


/* Test stationary uniform temperature distribution */
static int test_uniform_field(void)
{
    Config cfg;

    Grid grid = {0};

    size_t steps = 0;

    size_t i;
    size_t j;

    config_defaults(&cfg);

    cfg.nx = 7;
    cfg.ny = 7;

    cfg.width = 1.0;
    cfg.height = 1.0;

    cfg.initial_temperature = 42.0;
    cfg.boundary_temperature = 42.0;

    cfg.final_time = 0.01;

    cfg.steps_per_snapshot = 100;

    CHECK(grid_create(
        &grid,
        cfg.nx,
        cfg.ny,
        cfg.width,
        cfg.height
    ) == 0);

    grid_initialize(
        &grid,
        cfg.initial_temperature,
        cfg.boundary_temperature
    );

    CHECK(solver_run(
        &grid,
        &cfg,
        NULL,
        &steps
    ) == 0);

    CHECK(steps > 0);

    for (j = 0; j < grid.ny; j++)
    {
        for (i = 0; i < grid.nx; i++)
        {
            CHECK(fabs(
                grid_get(&grid, i, j) - 42.0
            ) < 1e-10);
        }
    }

    grid_destroy(&grid);

    return 0;
}


/* Test invalid grid dimensions */
static int test_invalid_grid(void)
{
    Grid grid = {0};

    CHECK(grid_create(
        &grid,
        2,
        2,
        1.0,
        1.0
    ) != 0);

    return 0;
}


/* Test invalid time step rejection */
static int test_unstable_timestep(void)
{
    Config cfg;

    Grid grid = {0};

    size_t steps = 0;

    config_defaults(&cfg);

    cfg.nx = 11;
    cfg.ny = 11;

    cfg.width = 1.0;
    cfg.height = 1.0;

    cfg.diffusivity = 0.1;

    cfg.dt = 0.1;
    cfg.final_time = 0.1;

    CHECK(grid_create(
        &grid,
        cfg.nx,
        cfg.ny,
        cfg.width,
        cfg.height
    ) == 0);

    grid_initialize(
        &grid,
        20.0,
        100.0
    );

    CHECK(solver_run(
        &grid,
        &cfg,
        NULL,
        &steps
    ) == -2);

    grid_destroy(&grid);

    return 0;
}


/* Execute complete test suite */
int main(void)
{
    printf("\nThermoForge Test Suite\n");
    printf("======================\n\n");

    CHECK(test_grid_allocation() == 0);
    printf("[PASS] Grid allocation\n");

    CHECK(test_boundary_conditions() == 0);
    printf("[PASS] Boundary conditions\n");

    CHECK(test_diagnostics() == 0);
    printf("[PASS] Diagnostics\n");

    CHECK(test_stability_limit() == 0);
    printf("[PASS] Stability calculation\n");

    CHECK(test_uniform_field() == 0);
    printf("[PASS] Uniform temperature field\n");

    CHECK(test_invalid_grid() == 0);
    printf("[PASS] Invalid grid rejection\n");

    CHECK(test_unstable_timestep() == 0);
    printf("[PASS] Unstable time-step rejection\n");

    printf("\nAll tests passed successfully.\n");

    return 0;
}
