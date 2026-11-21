#!/usr/bin/env python3
"""
Create a publication-quality heatmap from a ThermoForge temperature CSV.

Expected input: a CSV containing a rectangular matrix of numeric temperatures,
with one row per y-coordinate and one column per x-coordinate.

Dependencies:
    python -m pip install numpy matplotlib

Example:
    python thermoforge_plot.py
    python thermoforge_plot.py --input output/temperature_000054.csv
    python thermoforge_plot.py --cmap inferno --dpi 600 --output output/heatmap.png
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def read_temperature_csv(path: Path) -> np.ndarray:
    """Read a rectangular numeric CSV, tolerating a single header row/column."""
    if not path.is_file():
        raise FileNotFoundError(f"Input CSV does not exist: {path}")

    with path.open("r", newline="", encoding="utf-8-sig") as handle:
        rows = [
            row for row in csv.reader(handle)
            if row and any(cell.strip() for cell in row)
        ]

    if not rows:
        raise ValueError(f"The CSV is empty: {path}")

    # Convert cells to floats. If the first row is a header, discard it.
    def convert_row(row: list[str]) -> list[float]:
        return [float(cell.strip()) for cell in row]

    numeric_rows: list[list[float]] = []
    for row_index, row in enumerate(rows):
        try:
            numeric_rows.append(convert_row(row))
        except ValueError:
            if row_index == 0:
                continue
            # A non-numeric first column can be row labels; handle below.
            if all(len(r) == len(rows[0]) for r in rows):
                try:
                    numeric_rows = [
                        [float(cell.strip()) for cell in r[1:]]
                        for r in rows[1:]
                    ]
                    break
                except ValueError as exc:
                    raise ValueError(
                        "CSV must contain a rectangular numeric temperature "
                        "matrix, optionally with one header row or row-label column."
                    ) from exc
            raise ValueError(
                f"Non-numeric value found on CSV row {row_index + 1}."
            )

    if not numeric_rows:
        raise ValueError("No numeric temperature data was found in the CSV.")

    widths = {len(row) for row in numeric_rows}
    if len(widths) != 1:
        raise ValueError("CSV rows have different numbers of columns.")

    data = np.asarray(numeric_rows, dtype=float)
    if data.ndim != 2 or min(data.shape) < 2:
        raise ValueError(f"Expected a 2D grid, got shape {data.shape}.")
    if not np.isfinite(data).all():
        raise ValueError("Temperature data contains NaN or infinite values.")

    return data


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot a ThermoForge temperature field as a heatmap."
    )
    parser.add_argument(
        "--input", "-i",
        type=Path,
        default=Path("output/temperature_000054.csv"),
        help="Input temperature CSV (default: output/temperature_000054.csv)",
    )
    parser.add_argument(
        "--output", "-o",
        type=Path,
        default=Path("output/temperature_heatmap.png"),
        help="Output image path (default: output/temperature_heatmap.png)",
    )
    parser.add_argument(
        "--title",
        default="ThermoForge — Temperature Field",
        help="Plot title",
    )
    parser.add_argument(
        "--cmap",
        default="inferno",
        help="Matplotlib colormap (default: inferno; try viridis, magma, or plasma)",
    )
    parser.add_argument(
        "--dpi",
        type=int,
        default=300,
        help="Output resolution in DPI (default: 300)",
    )
    parser.add_argument(
        "--width",
        type=float,
        default=8.0,
        help="Figure width in inches (default: 8)",
    )
    parser.add_argument(
        "--height",
        type=float,
        default=6.5,
        help="Figure height in inches (default: 6.5)",
    )
    parser.add_argument(
        "--vmin",
        type=float,
        default=None,
        help="Optional lower bound for the color scale",
    )
    parser.add_argument(
        "--vmax",
        type=float,
        default=None,
        help="Optional upper bound for the color scale",
    )
    parser.add_argument(
        "--extent",
        type=float,
        nargs=4,
        metavar=("XMIN", "XMAX", "YMIN", "YMAX"),
        default=None,
        help="Physical plot bounds; defaults to grid indices",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        temperature = read_temperature_csv(args.input)
        if args.dpi <= 0 or args.width <= 0 or args.height <= 0:
            raise ValueError("DPI and figure dimensions must be positive.")
        if args.vmin is not None and args.vmax is not None:
            if args.vmin >= args.vmax:
                raise ValueError("--vmin must be smaller than --vmax.")

        rows, columns = temperature.shape
        x_label = "x position (grid index)"
        y_label = "y position (grid index)"
        extent = args.extent or (0, columns - 1, 0, rows - 1)

        # Use a clean, vector-friendly layout; save PNG at high resolution.
        fig, ax = plt.subplots(figsize=(args.width, args.height), constrained_layout=True)
        image = ax.imshow(
            temperature,
            origin="lower",
            extent=extent,
            cmap=args.cmap,
            interpolation="nearest",
            aspect="equal",
            vmin=args.vmin,
            vmax=args.vmax,
        )

        colorbar = fig.colorbar(image, ax=ax, pad=0.025, shrink=0.92)
        colorbar.set_label("Temperature", rotation=90, labelpad=14)

        ax.set_title(args.title, fontsize=15, weight="semibold", pad=14)
        ax.set_xlabel(x_label, labelpad=8)
        ax.set_ylabel(y_label, labelpad=8)
        ax.tick_params(direction="out", length=4, width=0.8)
        for spine in ax.spines.values():
            spine.set_linewidth(0.8)

        minimum = float(np.min(temperature))
        maximum = float(np.max(temperature))
        mean = float(np.mean(temperature))
        ax.text(
            0.02, 0.02,
            f"Grid: {columns} × {rows}    "
            f"Min: {minimum:.3f}    Max: {maximum:.3f}    Mean: {mean:.3f}",
            transform=ax.transAxes,
            ha="left",
            va="bottom",
            fontsize=8.5,
            color="white",
            bbox={"boxstyle": "round,pad=0.35", "facecolor": "black", "alpha": 0.55, "edgecolor": "none"},
        )

        args.output.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(args.output, dpi=args.dpi, bbox_inches="tight", facecolor="white")
        plt.close(fig)

        print(f"Loaded: {args.input}")
        print(f"Grid:   {columns} columns × {rows} rows")
        print(f"Range:  {minimum:.6g} to {maximum:.6g}")
        print(f"Mean:   {mean:.6g}")
        print(f"Saved:  {args.output} ({args.dpi} DPI)")
        return 0

    except (OSError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
