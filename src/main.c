
#include "thermoforge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Display application usage */
static void print_usage(const char *program)
{
    printf("\nThermoForge v%s\n", TF_VERSION);
    printf("Scientific Heat Diffusion Simulator\n\n");

    printf("Usage:\n");
    printf("  %s [configuration-file]\n\n", program);

    printf("If no configuration is provided, default parameters are used.\n");
}


/* Open the simulation history file */
static int open_history(
    const char *directory,
    FILE **stream
)
{
    char path[512];

    int written = snprintf(
        path,
        sizeof(path),
        "%s/history.csv",
        directory
    );

    if (written < 0 || (size_t)written >= sizeof(path))
    {
        return -1;
    }

    *stream = fopen(path, "w");

    return *stream == NULL ? -1 : 0;
}


int main(int argc, char **argv)
{
    Config cfg;

    Grid grid = {0};

    Diagnostics final_diag;

    char error[256] = "";

    FILE *history = NULL;

    size_t steps = 0;

    double stable_dt;

    int status = EXIT_FAILURE;

    /* Validate command-line arguments */
    if (argc > 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("\n");
    printf("====================================\n");
    printf("         THERMOFORGE v%s\n", TF_VERSION);
    printf("====================================\n\n");

    /* Load default parameters */
    config_defaults(&cfg);

    /* Load external configuration if provided */
    if (argc == 2)
    {
        printf("Loading configuration: %s\n", argv[1]);

        if (config_load(
            argv[1],
            &cfg,
            error,
            sizeof(error)
        ) != 0)
        {
            fprintf(stderr,
                    "Configuration error: %s\n",
                    error);

            return EXIT_FAILURE;
        }
    }

    /* Validate configuration */
    if (config_validate(
        &cfg,
        error,
        sizeof(error)
    ) != 0)
    {
        fprintf(stderr,
                "Invalid configuration: %s\n",
                error);

        return EXIT_FAILURE;
    }

    printf("Grid dimensions: %zu x %zu\n",
           cfg.nx,
           cfg.ny);

    printf("Diffusivity: %.8g\n",
           cfg.diffusivity);

    printf("Simulation duration: %.8g\n",
           cfg.final_time);

    /* Allocate computational grid */
    printf("\nInitializing computational grid...\n");

    if (grid_create(
        &grid,
        cfg.nx,
        cfg.ny,
        cfg.width,
        cfg.height
    ) != 0)
    {
        fprintf(stderr,
                "Failed to allocate computational grid.\n");

        goto cleanup;
    }

    /* Initialize temperature distribution */
    grid_initialize(
        &grid,
        cfg.initial_temperature,
        cfg.boundary_temperature
    );

    /* Prepare history output */
    if (open_history(cfg.output_dir, &history) != 0)
    {
        fprintf(stderr,
                "Cannot open output directory '%s'. "
                "Make sure it exists.\n",
                cfg.output_dir);

        goto cleanup;
    }

    if (output_write_history_header(history) != 0)
    {
        fprintf(stderr,
                "Failed to write history header.\n");

        goto cleanup;
    }

    /* Calculate numerical stability limit */
    stable_dt = solver_stable_dt(&cfg);

    printf("\nNumerical stability analysis\n");
    printf("----------------------------\n");

    printf("Maximum stable dt: %.10g\n", stable_dt);

    printf("Selected dt: %.10g\n",
           cfg.dt == 0.0
               ? stable_dt * 0.95
               : cfg.dt);

    /* Execute simulation */
    printf("\nStarting simulation...\n");

    {
        int solver_status = solver_run(
            &grid,
            &cfg,
            history,
            &steps
        );

        if (solver_status == -2)
        {
            fprintf(stderr,
                    "Requested time step exceeds stability limit.\n");

            goto cleanup;
        }

        if (solver_status != 0)
        {
            fprintf(stderr,
                    "Simulation solver encountered an error.\n");

            goto cleanup;
        }
    }

    /* Close history before finalizing output */
    if (fclose(history) != 0)
    {
        history = NULL;

        fprintf(stderr,
                "Failed to close history file.\n");

        goto cleanup;
    }

    history = NULL;

    /* Calculate final diagnostics */
    final_diag = diagnostics_compute(&grid);

    /* Export final temperature field */
    if (output_write_field(
        &grid,
        cfg.output_dir,
        steps
    ) != 0)
    {
        fprintf(stderr,
                "Failed to export temperature field.\n");

        goto cleanup;
    }

    /* Write simulation summary */
    if (output_write_summary(
        &cfg,
        &final_diag,
        steps,
        cfg.final_time,
        cfg.output_dir
    ) != 0)
    {
        fprintf(stderr,
                "Failed to write simulation summary.\n");

        goto cleanup;
    }

    /* Display final results */
    printf("\nSimulation completed successfully.\n");

    printf("------------------------------------\n");

    printf("Total time steps: %zu\n", steps);

    printf("Minimum temperature: %.6f\n",
           final_diag.minimum);

    printf("Maximum temperature: %.6f\n",
           final_diag.maximum);

    printf("Mean temperature: %.6f\n",
           final_diag.mean);

    printf("\nOutput directory: %s\n",
           cfg.output_dir);

    status = EXIT_SUCCESS;

cleanup:

    if (history != NULL)
    {
        fclose(history);
    }

    grid_destroy(&grid);

    return status;
}
