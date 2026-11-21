# Development Guide

## Overview

ThermoForge is written in standard C11.

The application separates numerical computation, memory
management, configuration parsing, diagnostics, and output.

## Coding Style

- Use descriptive function and variable names.
- Keep functions focused on individual tasks.
- Check allocation failures.
- Avoid unnecessary global variables.
- Document numerical assumptions.
- Prefer explicit error handling.
- Keep public declarations in thermoforge.h.

## Memory Management

The Grid structure owns two dynamically allocated arrays.

The arrays must be released using:

grid_destroy(&grid);

Do not manually free individual grid cells.

## Adding Features

When implementing a new feature:

1. Update public declarations if required.
2. Implement the functionality in the appropriate source file.
3. Add validation where necessary.
4. Add unit tests.
5. Update numerical documentation.
6. Update README documentation.

## Testing

Run:

make clean
make
make test

For debugging, compile with:

make clean
make CFLAGS="-std=c11 -g -O0 -Wall -Wextra -Wpedantic -fsanitize=address,undefined"

## Suggested Improvements

- Independent boundary temperatures.
- Spatial heat source distributions.
- Variable thermal diffusivity.
- Three-dimensional grids.
- Implicit integration methods.
- OpenMP parallelization.
- Analytical solution benchmarks.
- Python-based visualization.
