# Pathfinder Visualization

An interactive A* pathfinding visualizer built with C++, SDL3, and Dear ImGui. Set a start and destination, draw obstacles, and watch the search examine cells before it displays the resulting path.

## Build and run

Requirements:

- CMake 3.30 or newer and a compiler that supports C++26.
- SDL3 and `pkg-config`; CMake looks up the `sdl3` package through `pkg-config`.
- Dear ImGui, included as a Git submodule.
- A graphical environment capable of opening an SDL window.

```sh
git clone --recurse-submodules https://github.com/daniloamatos/A-Pathfinder-visualizer.git pathfinding
cd pathfinding
cmake -S . -B build
cmake --build build
./build/main
```

For an existing clone without the submodule, run `git submodule update --init --recursive` before building. `make run` provides a shortcut for configuring, building, and launching the executable.

## Use the visualizer

Click or drag with the left mouse button to edit the grid. Hold a modifier to choose what to draw:

| Input | Action |
| --- | --- |
| Left mouse button | Place obstacles |
| Shift + left mouse button | Set the start |
| Ctrl + left mouse button | Set the destination |
| Space + left mouse button | Erase cells |

Select **Run** to start the search. Select it again to pause, and once more to resume from the same point. **Pause** also pauses the search. **Stop && Clear** discards search progress and clears the examined cells and path while retaining the start, destination, and obstacles. **New** clears the entire grid.

**Small**, **Medium**, and **Large** select 15 × 15, 30 × 30, and 50 × 50 grids. Start, destination, and obstacle cells that fit within the new dimensions are retained. The `^` button in the lower-left corner opens the speed controls; **instant** is selected by default.

| Color | Meaning |
| --- | --- |
| Green | Start |
| Red | Destination |
| Black | Obstacle |
| Dark blue | Cell examined as a neighbor during the search |
| Light blue | Final path |
| Gray | Empty cell |

## A* search

The search runs on an eight-connected grid. Orthogonal moves cost `1`, diagonal moves cost `√2`, and a diagonal move is blocked if either adjacent orthogonal cell is an obstacle. The estimated remaining cost is the octile-distance heuristic:

```text
h = √2 × min(dx, dy) + |dx - dy|
f = g + h
```

Here, `g` is the cost from the start, and `dx` and `dy` are coordinate distances to the destination. The octile heuristic is consistent with these movement costs, so the destination is accepted when it is removed from the priority queue, rather than when it first appears among a node's neighbors. The path is then reconstructed through parent pointers.

The dark-blue `Visited` state is a display marker for cells examined as eligible neighbors. It is separate from the algorithm's open and closed data structures; a dark-blue cell is not necessarily a processed node.

## Custom indexed heap

`NodeHeap` in `main.h` is a binary min-heap used for the A* open set. Nodes are ordered by `f = g + h`; equal `f` values favor the smaller `h`.

- A two-dimensional `position` table maps each grid coordinate to its heap index. This provides constant-time lookup of nodes already in the open set.
- `siftUp` and `siftDown` maintain the heap after insertion, removal, and `decreaseKey`. Heap updates take `O(log n)` time.
- When a cheaper route reaches an open node, `decreaseKey` updates its `g` cost and parent instead of inserting another copy.
- Removed nodes remain owned by the heap until the search ends, keeping parent pointers valid for path reconstruction.

`PathfinderState` holds the heap and a separate two-dimensional closed set. Each call to `aStar` advances one search step, which lets the UI pause and resume without rebuilding the frontier. Finishing or cancelling a search releases these structures.

## Speed and timing

Animated modes advance at 30, 60, 120, 150, 180, or 240 A* steps per second, corresponding to the **0.25x** through **2x** controls. **instant** executes steps in a frame-time budget of about 8 ms per frame.

The result dialog reports the sum of time spent inside `aStar` calls. Rendering, animation delays, and time spent paused are excluded. SDL vertical synchronization is used when available; otherwise, the main loop targets a frame duration of about 16 ms.

## Source layout

| File | Responsibility |
| --- | --- |
| `main.cpp` | SDL event loop, ImGui controls, grid editing and rendering, and search scheduling |
| `main.h` | Grid and cell types, indexed heap, search state, and timing state |
| `pathfinding.cpp` | Incremental A* step, neighbor checks, and path reconstruction |
| `CMakeLists.txt` | Build configuration and SDL3/ImGui integration |
| `Makefile` | Convenience commands for building, running, and cleaning |
| `Dockerfile` | Arch Linux image with the build and SDL3 dependencies |
