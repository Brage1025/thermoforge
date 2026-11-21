
#include "thermoforge.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Remove whitespace from both ends of a string */
static char *trim(char *text)
{
    char *end;

    while (isspace((unsigned char)*text))
    {
        text++;
    }

    if (*text == '\0')
    {
        return text;
    }

    end = text + strlen(text) - 1;

    while (end > text &&
           isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }

    return text;
}


/* Initialize default simulation parameters */
void config_defaults(Config *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

    cfg->nx = 51;
    cfg->ny = 51;

    cfg->width = 1.0;
    cfg->height = 1.0;

    cfg->diffusivity = 0.01;

    cfg->initial_temperature = 20.0;
    cfg->boundary_temperature = 100.0;

    cfg->source = 0.0;

    cfg->final_time = 0.1;
    cfg->dt = 0.0;

    cfg->steps_per_snapshot = 10;

    snprintf(
        cfg->output_dir,
        sizeof(cfg->output_dir),
        "%s",
        "output"
    );
}


/* Convert a string into a double */
static int parse_double_value(
    const char *value,
    double *destination
)
{
    char *end = NULL;
    double parsed;

    errno = 0;

    parsed = strtod(value, &end);

    if (errno != 0 ||
        end == value ||
        *trim(end) != '\0')
    {
        return -1;
    }

    *destination = parsed;

    return 0;
}


/* Convert a string into a size_t */
static int parse_size_value(
    const char *value,
    size_t *destination
)
{
    char *end = NULL;
    unsigned long long parsed;

    if (value[0] == '-')
    {
        return -1;
    }

    errno = 0;

    parsed = strtoull(value, &end, 10);

    if (errno != 0 ||
        end == value ||
        *trim(end) != '\0')
    {
        return -1;
    }

    if (parsed > (unsigned long long)SIZE_MAX)
    {
        return -1;
    }

    *destination = (size_t)parsed;

    return 0;
}


/* Assign a parsed configuration value */
static int assign_value(
    Config *cfg,
    const char *key,
    const char *value
)
{
    if (strcmp(key, "nx") == 0)
        return parse_size_value(value, &cfg->nx);

    if (strcmp(key, "ny") == 0)
        return parse_size_value(value, &cfg->ny);

    if (strcmp(key, "steps_per_snapshot") == 0)
        return parse_size_value(
            value,
            &cfg->steps_per_snapshot
        );

    if (strcmp(key, "width") == 0)
        return parse_double_value(value, &cfg->width);

    if (strcmp(key, "height") == 0)
        return parse_double_value(value, &cfg->height);

    if (strcmp(key, "diffusivity") == 0)
        return parse_double_value(
            value,
            &cfg->diffusivity
        );

    if (strcmp(key, "initial_temperature") == 0)
        return parse_double_value(
            value,
            &cfg->initial_temperature
        );

    if (strcmp(key, "boundary_temperature") == 0)
        return parse_double_value(
            value,
            &cfg->boundary_temperature
        );

    if (strcmp(key, "source") == 0)
        return parse_double_value(value, &cfg->source);

    if (strcmp(key, "final_time") == 0)
        return parse_double_value(value, &cfg->final_time);

    if (strcmp(key, "dt") == 0)
        return parse_double_value(value, &cfg->dt);

    if (strcmp(key, "output_dir") == 0)
    {
        if (strlen(value) >= sizeof(cfg->output_dir))
        {
            return -1;
        }

        snprintf(
            cfg->output_dir,
            sizeof(cfg->output_dir),
            "%s",
            value
        );

        return 0;
    }

    return -2;
}


/* Load configuration from a text file */
int config_load(
    const char *path,
    Config *cfg,
    char *error,
    size_t error_size
)
{
    FILE *input;
    char line[512];
    size_t line_number = 0;

    if (path == NULL || cfg == NULL)
    {
        snprintf(error, error_size,
                 "Invalid configuration arguments");

        return -1;
    }

    input = fopen(path, "r");

    if (input == NULL)
    {
        snprintf(error, error_size,
                 "Cannot open configuration file: %s",
                 path);

        return -1;
    }

    while (fgets(line, sizeof(line), input) != NULL)
    {
        char *key;
        char *value;
        char *equals;
        int result;

        line_number++;

        key = trim(line);

        if (*key == '\0' || *key == '#')
        {
            continue;
        }

        equals = strchr(key, '=');

        if (equals == NULL)
        {
            snprintf(error, error_size,
                     "Line %zu: expected key = value",
                     line_number);

            fclose(input);
            return -1;
        }

        *equals = '\0';

        value = trim(equals + 1);
        key = trim(key);

        if (*key == '\0' || *value == '\0')
        {
            snprintf(error, error_size,
                     "Line %zu: empty key or value",
                     line_number);

            fclose(input);
            return -1;
        }

        result = assign_value(cfg, key, value);

        if (result == -2)
        {
            snprintf(error, error_size,
                     "Line %zu: unknown key '%s'",
                     line_number,
                     key);

            fclose(input);
            return -1;
        }

        if (result != 0)
        {
            snprintf(error, error_size,
                     "Line %zu: invalid value for '%s'",
                     line_number,
                     key);

            fclose(input);
            return -1;
        }
    }

    if (ferror(input))
    {
        snprintf(error, error_size,
                 "Error reading configuration file");

        fclose(input);
        return -1;
    }

    fclose(input);

    return config_validate(cfg, error, error_size);
}


/* Validate physical and numerical parameters */
int config_validate(
    Config *cfg,
    char *error,
    size_t error_size
)
{
    if (cfg == NULL)
    {
        snprintf(error, error_size,
                 "Configuration is NULL");

        return -1;
    }

    if (cfg->nx < 3 || cfg->ny < 3)
    {
        snprintf(error, error_size,
                 "Grid dimensions must be at least 3");

        return -1;
    }

    if (cfg->nx > 10000 ||
        cfg->ny > 10000 ||
        cfg->nx > SIZE_MAX / cfg->ny)
    {
        snprintf(error, error_size,
                 "Grid dimensions are too large");

        return -1;
    }

    if (!(cfg->width > 0.0) ||
        !(cfg->height > 0.0))
    {
        snprintf(error, error_size,
                 "Domain dimensions must be positive");

        return -1;
    }

    if (!(cfg->diffusivity > 0.0))
    {
        snprintf(error, error_size,
                 "Diffusivity must be positive");

        return -1;
    }

    if (!(cfg->final_time >= 0.0))
    {
        snprintf(error, error_size,
                 "Final time cannot be negative");

        return -1;
    }

    if (!(cfg->dt >= 0.0))
    {
        snprintf(error, error_size,
                 "Time step cannot be negative");

        return -1;
    }

    if (cfg->steps_per_snapshot == 0)
    {
        snprintf(error, error_size,
                 "Snapshot interval must be positive");

        return -1;
    }

    if (cfg->output_dir[0] == '\0')
    {
        snprintf(error, error_size,
                 "Output directory cannot be empty");

        return -1;
    }

    return 0;
}
