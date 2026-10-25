# Walk Area and Obstacles

After intrinsic and extrinsic calibration, you can define a walkable area and obstacle polygons directly in the calibrated view.
This allows you to model walkable regions and export them as a wkt-string in world coordinates (meters). The string may be directly imported in [PedPy](https://pedpy.readthedocs.io/stable/).

All walk area data is stored in your `.pet` file.

:::{Important}
If you calculate a new intrinsic calibration, you must draw the polygons again.
:::

The resulting polygons are 2D polygons on the ground plane. If desired, polygons can be drawn on a non-zero height (e.g. for tracing the upper edge of a fence).
If you draw on an object which is not on the ground plane, remember to set the height correctly for the polygon. The vertices of such polygons are projected
onto the ground plane on export.

## Requirements

Before setting up walk area polygons, check that:

1. intrinsic calibration is active
2. extrinsic calibration is available and usable

The walk area feature is available in the calibration tab in the `walk area` section.

## PeTrack Workflow

### 1. Open the walk area controls

First, enable:

- `show/hide completely` to display polygons in the calibrated view
- `enable editing-mode` to start drawing

Disabling `enable editing-mode` turns off all drawing and editing, so clicks in the video view no longer affect the polygons.

You can switch between drawing modes using:

- `add walkarea`
- `add obstacles`

:::{Note}
Only one walkable polygon can exist at a time. After creating it, the drawing mode automatically switches to obstacles.
:::

### 2. Draw polygons in the calibrated view

Double click the left mouse button in the video view to add polygon vertices.
To close a polygon, double click close to the first point.

While editing:

- `Backspace` removes the last placed point
- `Escape` cancels the polygon currently being drawn

### 3. Move polygon vertices

In edit mode, every vertex (corner point) of the existing polygons is shown as a small filled dot in the polygon's color. These dots are the handles you can grab to move a vertex.
In the screenshot below, the arrows point at the handles of the walkable area (blue) and of two obstacles (red).

:::{image} images/walk_area_vertex_handles.png
:width: 60%
:::

Click on a vertex and drag it with the left mouse button held down to move it to a new position. This works for the walkable area as well as for obstacles, including polygons that were imported from a wkt-string.
Press `Escape` while dragging to cancel the move and put the vertex back where it was.

Only visible and enabled polygons can be edited this way, and only while no new polygon is being drawn.

The polygons are validated again after a vertex has been moved.

### 4. Manage polygons in the table

Each polygon is listed with:

- id
- type (`walkable` or `obstacle`)
- height
- enabled status

You can adjust the height of each polygon and temporarily disable polygons without deleting them.
Use the `delete` button to remove a polygon.

### 5. Validate geometry

PeTrack validates enabled polygons automatically.
Invalid obstacle rows are highlighted in red, and the status line above the table describes the problem.

Typical validation errors are:

- obstacle outside the walkable area
- overlapping enabled obstacles
- walkable area completely inside an obstacle

Export is only possible while validation is successful.

### 6. Import/Export WKT

Use the `walkarea input/output` section for exchanging polygons:

- `export wkt-string` exports walkable area and all enabled obstacles as world-coordinate WKT
- `import wkt-string` imports WKT and overwrites all current polygons

It is possible to import both `POLYGON` and `POLYGON Z` wkt-strings. If a z-value is parsed, it will be set as the height for the respective polygon. Make sure to always use meters as the unit here.

:::{Important}
Import replaces the complete current walk area setup (walkable polygon and all obstacles).
:::
