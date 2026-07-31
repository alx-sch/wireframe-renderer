# Wireframe Renderer

<p align="center">
    <img src=".assets/fdfe.png" alt="fdf badge" width="400" />
</p>

A 3D wireframe terrain renderer written in C. Reads heightmap files (`.fdf`) and renders them as isometric wireframe projections using MiniLibX. Supports per-vertex color, automatic scaling and centering, and runs on both Linux and macOS.

- **Isometric projection** at a configurable angle (default 30°)
- **Bresenham's line algorithm** for efficient integer-only rasterization
- **Per-vertex hex color** support (`0xRRGGBB`) with a configurable default wireframe color
- **Auto-scaling and centering** — fits any map to the window regardless of dimensions
- **Cross-platform** — builds on Linux (X11) and macOS

---

## Table of Contents

- [Getting Started](#-getting-started)
- [Codespaces / Devcontainer](#-codespaces--devcontainer)
- [The `.fdf` File Format](#the-fdf-file-format)
- [How It Works](#how-it-works)
  - [Parsing](#parsing)
  - [Isometric Projection](#isometric-projection)
  - [Scaling and Centering](#scaling-and-centering)
  - [Bresenham's Line Algorithm](#bresenhams-line-algorithm)
  - [Rendering Pipeline](#rendering-pipeline)
- [Screenshot](#screenshot)
- [Controls](#controls)
- [Acknowledgements](#acknowledgements)
- [References](#references)

---

## 🚀 Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/fdf.git
   cd fdf
   ```

2. Build the project (MiniLibX is cloned and compiled automatically):
   ```bash
   make
   ```

3. Run with a map file:
   ```bash
   ./fdf maps/mars.fdf
   ```

> **Requirements:** `cc`, `make`, `git`, X11 development libraries (`libxext-dev`, `libx11-dev` on Debian/Ubuntu).

---

## 🐳 Codespaces / Devcontainer

This repo includes a devcontainer configuration — open it in GitHub Codespaces (or any devcontainer-compatible editor) and it will build automatically with all dependencies.

Since fdf renders to an X11 window, the devcontainer provides a virtual display via **noVNC**:

1. Open the repo in Codespaces (or `Dev Containers: Reopen in Container` locally).
2. After the container builds, start the virtual display:
   ```bash
   start-vnc.sh
   ```
3. Open the forwarded port **6080** (noVNC web client) — Codespaces will prompt you automatically.
4. Run fdf:
   ```bash
   ./fdf maps/mars.fdf
   ```
5. The wireframe renders in the noVNC browser tab.

---

## The `.fdf` File Format

An `.fdf` file encodes a terrain as a rectangular grid of space-separated integer heights. Each line represents one row of the grid, and every value in that line represents the elevation (z-value) at that column:

```
0  0  0  0  0  0  0  0  0  0
0 10 10 10 10 10 10 10 10  0
0 10 20 15 12 15 17 20 10  0
0 10 10 10 10 10 10 10 10  0
0  0  0  0  0  0  0  0  0  0
```

Optionally, a color can be appended to any value using a comma and a hexadecimal code:

```
0 10,0xFF0000 20,0x00FF00 10,0x0000FF 0
```

If no color is specified, the configurable default wireframe color is used (sky blue by default).

**Constraints:**
- The grid must be rectangular (same number of entries per line)
- At least one row and one column
- Colors must be in the format `,0xRRGGBB`

---

## How It Works

### Parsing

The program reads the `.fdf` file in three passes:

1. **Dimensions** — counts words per line to get the grid width (`x_max`) and counts lines for the height (`y_max`). Validates that all rows have equal width.
2. **Z-values** — tokenizes each line and converts entries to integers with `atoi`, populating a 2D integer matrix `z[row][col]`.
3. **Colors** — scans each token for a comma separator; if present, parses the trailing hex string. Otherwise assigns the default color.

### Isometric Projection

The core of fdf is projecting 3D grid coordinates $(x, y, z)$ onto a 2D image plane using an isometric transformation. For a given angle $\theta$ (default 30°) and a z-scale factor $s$:

$$x' = (x - y) \cdot \cos(\theta)$$

$$y' = (x + y) \cdot \sin(\theta) - z \cdot s$$

This creates the characteristic "diamond" view where both the x- and y-axes recede at equal angles from the horizontal. The z-scale factor ($s = 0.08$ by default) controls how exaggerated the terrain height appears — without it, peaks on high-relief maps would dominate the projection.

<p align="center">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/f/f7/Axonometric_projection.svg/250px-Axonometric_projection.svg.png" alt="Isometric projection axes" width="200" />
    <br><span>Isometric projection maps three axes onto a 2D plane at equal angles.</span>
</p>

**Why subtract in $x'$ and add in $y'$?** The $(x - y)$ term separates the x and y axes horizontally (one goes left, one goes right), while $(x + y)$ combines them vertically (both recede "into" the screen). The $-z \cdot s$ lifts elevated points upward on screen.

### Scaling and Centering

After projection, the wireframe must fit within the window (1440×900 by default). The algorithm:

1. **Find extrema** — iterate over all projected coordinates to find the minimum and maximum $x'$ and $y'$ values.
2. **Compute range** — $\Delta x = x'_{max} - x'_{min}$, $\Delta y = y'_{max} - y'_{min}$.
3. **Determine scale** — calculate independent scale factors for each axis, then take the smaller one so the entire map fits:

$$\text{scale}_x = \frac{W}{\Delta x} \cdot u \qquad \text{scale}_y = \frac{H}{\Delta y} \cdot u$$

$$\text{scale} = \min(\text{scale}_x,\ \text{scale}_y)$$

   where $W$ and $H$ are window dimensions and $u$ is the screen utilization factor (0.9 = 90% of available space).

4. **Compute offset** — center the projection by calculating the remaining margin on each axis and halving it:

$$\text{offset}_x = \frac{W - (\Delta x \cdot \text{scale})}{2}$$

5. **Apply** — the final pixel coordinate for each point becomes:

$$(x' - x'_{min}) \cdot \text{scale} + \text{offset}_x$$

### Bresenham's Line Algorithm

Connecting adjacent grid points with straight lines requires a rasterization algorithm. Bresenham's algorithm is ideal because it uses only integer arithmetic (additions and comparisons), making it fast on systems without hardware floating-point or when pixel-level precision matters.

The idea: for a line from $(x_0, y_0)$ to $(x_1, y_1)$, maintain an **error term** that tracks the accumulated deviation from the true line. At each step, advance along the dominant axis by one pixel and use the error to decide whether to also step along the secondary axis:

```
err = dx - dy
while (x0, y0) != (x1, y1):
    plot(x0, y0)
    e2 = 2 * err
    if e2 > -dy:
        err -= dy
        x0 += sx
    if e2 < dx:
        err += dx
        y0 += sy
```

Here `sx` and `sy` encode direction (+1 or -1), and `dx`/`dy` are absolute differences. The algorithm generalizes to all octants without special-casing slope ranges.

### Rendering Pipeline

The complete rendering sequence:

1. **Fill background** — write the background color to every pixel in the image buffer.
2. **Traverse the grid** — for each point `(row, col)`:
   - If a right neighbor exists (`col + 1 < x_max`), draw a horizontal wireframe edge.
   - If a bottom neighbor exists (`row + 1 < y_max`), draw a vertical wireframe edge.
3. **Display** — push the completed image buffer to the window with a single `mlx_put_image_to_window` call.

Pixel writes go directly into the image buffer via pointer arithmetic rather than calling `mlx_pixel_put` per pixel, which avoids the overhead of repeated X11 draw calls.

---

## Screenshot

<p align="center">
    <img src=".assets/fdf_example.png" alt="fdf rendering of a colored terrain map" width="600" />
    <br><span>Isometric wireframe render of a terrain heightmap with per-vertex color encoding.</span>
</p>

---

## Controls

| Key | Action |
| :--- | :--- |
| `Esc` | Close window and exit |
| Window `X` button | Close window and exit |

---

## Acknowledgements

- Project badge by [Ali Ogun](https://github.com/ayogun/42-project-badges)
- [MiniLibX](https://github.com/42Paris/minilibx-linux) — lightweight X11 graphics library

---

## References

<a name="footnote1">[1]</a> Bresenham, J.E. (1965). *Algorithm for Computer Control of a Digital Plotter*. IBM Systems Journal, 4(1), 25–30.<br>
<a name="footnote2">[2]</a> Angel, E. & Shreiner, D. (2014). *Interactive Computer Graphics: A Top-Down Approach with WebGL*. Pearson. Chapter on projections and viewing transformations.<br>
<a name="footnote3">[3]</a> MiniLibX documentation: <a href="https://harm-smits.github.io/42docs/libs/minilibx" target="_blank">https://harm-smits.github.io/42docs/libs/minilibx</a><br>
<a name="footnote4">[4]</a> Isometric projection — Wikipedia: <a href="https://en.wikipedia.org/wiki/Isometric_projection" target="_blank">https://en.wikipedia.org/wiki/Isometric_projection</a>

<div align="right"><b><a href="#fdf">back to top</a></b></div>
