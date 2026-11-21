
#include "thermoforge.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Allocate the computational grid.
 *
 * The grid contains two buffers:
 * current = current simulation state
 * next    = next simulation state
 */
int grid_create(
    Grid *grid,
    size_t nx,
    size_t ny,
    double width,
    double height
)
{
    size_t count;

    if (grid == NULL ||
        nx < 3 ||
        ny < 3 ||
        width <= 0.0 ||
        height <= 0.0)
    {
        return -1;
    }

    if (nx > SIZE_MAX / ny)
    {
        return -1;
    }

    count = nx * ny;

    if (count > SIZE_MAX / sizeof(double))
    {
        return -1;
    }

    grid->current = calloc(count, sizeof(double));
    grid->next = calloc(count, sizeof(double));

    if (grid->current == NULL || grid->next == NULL)
    {
        free(grid->current);
        free(grid->next);

        grid->current = NULL;
        grid->next = NULL;

        return -1;
    }

    grid->nx = nx;
    grid->ny = ny;

    grid->dx = width / (double)(nx - 1);
    grid->dy = height / (double)(ny - 1);

    return 0;
}


/* Release allocated grid memory */
void grid_destroy(Grid *grid)
{
    if (grid == NULL)
    {
        return;
    }

    free(grid->current);
    free(grid->next);

    memset(grid, 0, sizeof(*grid));
}


/*
 * Apply fixed-temperature boundary conditions.
 *
 * All four edges receive the same temperature.
 */
void grid_apply_boundary(Grid *grid, double boundary)
{
    size_t i;
    size_t j;

    if (grid == NULL || grid->current == NULL)
    {
        return;
    }

    for (i = 0; i < grid->nx; i++)
    {
        grid->current[i] = boundary;

        grid->current[
            (grid->ny - 1) * grid->nx + i
        ] = boundary;
    }

    for (j = 0; j < grid->ny; j++)
    {
        grid->current[j * grid->nx] = boundary;

        grid->current[
            j * grid->nx + grid->nx - 1
        ] = boundary;
    }
}


/* Initialize all grid cells */
void grid_initialize(
    Grid *grid,
    double interior,
    double boundary
)
{
    size_t i;
    size_t j;

    if (grid == NULL || grid->current == NULL)
    {
        return;
    }

    for (j = 0; j < grid->ny; j++)
    {
        for (i = 0; i < grid->nx; i++)
        {
            grid_set(grid, i, j, interior);
        }
    }

    grid_apply_boundary(grid, boundary);

    memcpy(
        grid->next,
        grid->current,
        grid->nx * grid->ny * sizeof(double)
    );
}


/* Exchange the current and next simulation buffers */
void grid_swap(Grid *grid)
{
    double *temporary;

    if (grid == NULL)
    {
        return;
    }

    temporary = grid->current;
    grid->current = grid->next;
    grid->next = temporary;
}


/* Retrieve a temperature value */
double grid_get(
    const Grid *grid,
    size_t i,
    size_t j
)
{
    if (grid == NULL ||
        grid->current == NULL ||
        i >= grid->nx ||
        j >= grid->ny)
    {
        return 0.0;
    }

    return grid->current[j * grid->nx + i];
}


/* Change a temperature value */
void grid_set(
    Grid *grid,
    size_t i,
    size_t j,
    double value
)
{
    if (grid == NULL ||
        grid->current == NULL ||
        i >= grid->nx ||
        j >= grid->ny)
    {
        return;
    }

    grid->current[j * grid->nx + i] = value;
}
