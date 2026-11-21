# Numerical Methods

## 1. Introduction

ThermoForge solves the transient two-dimensional heat equation
using an explicit finite-difference method.

The computational domain is represented by a uniform Cartesian grid.

## 2. Governing Equation

The heat equation is:

dT/dt = alpha \* (d2T/dx2 + d2T/dy2) + S

The model assumes constant material properties.

## 3. Spatial Discretization

The second-order central difference approximation is:

d2T/dx2 approximately:

(T[i+1,j] - 2T[i,j] + T[i-1,j]) / dx^2

Similarly:

d2T/dy2 approximately:

(T[i,j+1] - 2T[i,j] + T[i,j-1]) / dy^2

## 4. Temporal Discretization

Forward Euler integration is used:

T_new = T_old + dt \* (diffusion + source)

The method is explicit and conditionally stable.

## 5. Stability Condition

For the two-dimensional diffusion equation:

rx + ry <= 0.5

Where:

rx = alpha \* dt / dx^2

ry = alpha \* dt / dy^2

The maximum stable time step is:

dt_max = 0.5 / (alpha \* (1/dx^2 + 1/dy^2))

The implementation selects 95 percent of this limit
when automatic time stepping is enabled.

## 6. Grid Convention

The grid includes boundary nodes.

The physical spacing is:

dx = width / (nx - 1)
dy = height / (ny - 1)

The one-dimensional array index is:

index = j \* nx + i

The x coordinate increases from left to right.
The y coordinate increases from bottom to top.

## 7. Boundary Conditions

All four boundaries use fixed Dirichlet temperature values.

Boundary temperatures are applied at initialization
and preserved during every time step.

## 8. Diagnostics

The simulation calculates:

- Minimum temperature
- Maximum temperature
- Arithmetic mean temperature
- Integrated temperature proxy

The energy proxy is not thermodynamic energy.
It does not include material density or specific heat capacity.

## 9. Numerical Limitations

The explicit method is conditionally stable.

Stability does not guarantee accuracy.

Quantitative research should include:

- Grid refinement
- Time-step refinement
- Analytical benchmark comparisons
- Conservation and physical consistency checks

ThermoForge is not a validated engineering solver.
