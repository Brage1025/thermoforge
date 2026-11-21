# ThermoForge

### A C11 Scientific Heat Transfer Simulation Framework

![Language](https://img.shields.io/badge/language-C11-blue)
![Build](https://img.shields.io/badge/build-Make-green)
![License](https://img.shields.io/badge/license-MIT-orange)
![Status](https://img.shields.io/badge/status-educational%20research-lightgrey)

ThermoForge is a numerical simulation project written in C11 for
studying transient heat conduction in two-dimensional materials.

The project implements a finite-difference approximation of the
heat equation using an explicit Forward-Time Centered-Space
(FTCS) integration scheme.

It provides a modular simulation architecture, configurable
physical parameters, numerical stability analysis, temperature
diagnostics, and CSV-based scientific output.

---

## Overview

ThermoForge models the evolution of temperature over a rectangular
computational domain.

The governing equation is:

dT/dt = alpha \* (d2T/dx2 + d2T/dy2) + S

Where:

- T: Temperature field
- alpha: Thermal diffusivity
- S: Constant source term
- x, y: Spatial coordinates
- t: Simulation time

The numerical solver approximates the spatial derivatives using
central finite differences and advances the solution through
discrete time steps.

## Features

- Two-dimensional heat diffusion simulation
- Explicit FTCS numerical integration
- Fixed-temperature boundary conditions
- Configurable thermal diffusivity
- Configurable computational grid
- Automatic time-step selection
- Numerical stability validation
- Temperature field diagnostics
- CSV temperature export
- Simulation history tracking
- Modular C11 architecture
- Basic automated unit tests

## Requirements

- GCC or Clang
- C11-compatible compiler
- GNU Make
- Standard C library
- Math library

## Installation

Clone the repository:

```bash
git clone https://github.com/YOUR_USERNAME/thermoforge.git
cd thermoforge
```

Compile the project:

```bash
make
```

Run the test suite:

```bash
make test
```

Execute the example:

```bash
mkdir -p output
./thermoforge examples/heated_plate.cfg
```

Alternatively:

```bash
make run
```

## Configuration

ThermoForge uses a simple key-value configuration format.

Example:

```ini
nx = 81
ny = 81

width = 1.0
height = 1.0

diffusivity = 0.01

initial_temperature = 20.0
boundary_temperature = 100.0

source = 0.0

final_time = 0.20
dt = 0.0

steps_per_snapshot = 20

output_dir = output
```

A time step of zero enables automatic time-step selection.

## Numerical Stability

The explicit integration method requires a stable time step.

For a two-dimensional uniform grid, the condition is:

alpha _ dt _ (1/dx^2 + 1/dy^2) <= 1/2

ThermoForge calculates the maximum permitted time step
and rejects unstable user-specified values.

## Output

The simulation generates:

| File                   | Description                                 |
| ---------------------- | ------------------------------------------- |
| temperature_XXXXXX.csv | Final temperature matrix                    |
| history.csv            | Temperature statistics over time            |
| summary.txt            | Simulation parameters and final diagnostics |

CSV output can be analyzed using Python, MATLAB, GNU Octave,
R, or spreadsheet applications.

## Project Structure

```text
include/    Public declarations
src/        Simulation implementation
tests/      Unit tests
examples/   Example experiments
docs/       Technical documentation
```

## Scientific Limitations

ThermoForge is an educational numerical simulation framework.

The current implementation assumes:

- Constant thermal diffusivity
- Uniform rectangular computational grid
- Homogeneous isotropic material
- Fixed temperature boundaries
- Explicit numerical integration
- Constant internal source term

The software has not been validated for industrial engineering
or safety-critical applications.

## Future Development

Potential improvements include:

- Variable thermal conductivity
- Multiple boundary conditions
- Three-dimensional heat transfer
- Implicit numerical solvers
- Spatially varying heat sources
- Material property databases
- Parallel computation using OpenMP
- Python visualization tools
- Analytical benchmark comparisons

## License

This project is distributed under the MIT License.

See LICENSE for details.

---

ThermoForge - Exploring numerical heat transfer through C.
