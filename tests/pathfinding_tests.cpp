#include "main.h"
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        std::abort();
    }
}

static Grid makeGrid()
{
    Grid grid;
    grid.resize(8, 8);
    grid.start[0] = grid.start[1] = 0;
    grid.end[0] = grid.end[1] = 7;
    grid.cells[0][0].state = CellState::Start;
    grid.cells[7][7].state = CellState::End;
    return grid;
}

static void finish(Grid& grid, PathfinderState& state)
{
    int steps = 0;
    while (state.running && steps++ < 1000)
    {
        aStar(&grid, state);
        check(grid.cells[7][7].state == CellState::End, "Destination lost its color");
    }
    check(!state.running && !state.initialized, "Search did not finish");
    check(!state.open && state.closed.empty(), "Finished search retained resources");
}

int main()
{
    // Use controlled timestamps so timing regressions need no sleeps.
    using namespace std::chrono_literals;
    const SearchTimer::Clock::time_point start{};
    SearchTimer timer;
    check(!timer.active, "Idle grid should be editable");
    timer.pause(start);
    check(!timer.active, "Pause on an idle grid started a session");
    timer.resume(start);
    check(timer.active, "Grid must lock before the first A* step");
    timer.resume(start + 1s); // Clicking Run while running is a no-op for time.
    timer.pause(start + 2s);
    check(timer.active, "Paused grid must remain locked");
    check(timer.elapsedSeconds() == 2.0, "Repeated Run reset the timer");
    timer.pause(start + 5s);
    check(timer.elapsedSeconds() == 2.0, "Repeated Pause counted paused time");
    timer.resume(start + 12s);
    timer.pause(start + 15s);
    timer.resume(start + 25s);
    timer.finish(start + 29s);
    check(timer.elapsedSeconds() == 9.0, "Result includes paused time");
    check(!timer.active, "Completion must unlock the grid");
    timer = SearchTimer{}; // Same reset as clearSearch, for a new run.
    timer.resume(start + 30s);
    timer.finish(start + 31s);
    check(timer.elapsedSeconds() == 1.0, "New run retained previous elapsed time");
    timer = SearchTimer{};
    timer.resume(start + 32s);
    timer.pause(start + 33s);
    timer = SearchTimer{}; // Cancel a paused session.
    check(!timer.active && timer.elapsedSeconds() == 0.0,
          "Cancellation must unlock the grid and reset time");

    Grid grid = makeGrid();
    PathfinderState state;
    state.running = true;
    aStar(&grid, state);
    check(state.pushed == 4, "Initial node missing from pushed count");
    check(state.expanded == 1, "First step did not expand one node");

    NodeHeap* heap = state.open.get();
    ListNode* next = heap->top();
    state.running = false; // Pause must retain the frontier and parents.
    check(state.initialized && state.open.get() == heap, "Pause discarded search");
    state.running = true; // Resume uses the same initialized search.
    aStar(&grid, state);
    check(state.open.get() == heap && state.expanded == 2, "Resume restarted search");
    check(state.closed[next->val[0]][next->val[1]], "Resume lost the next queued node");
    finish(grid, state);
    check(state.found && !state.noPath, "Reachable destination was not found");
    check(grid.cells[1][1].state == CellState::Path, "Path reconstruction failed");

    for (bool paused : {false, true})
    {
        grid = makeGrid();
        state = PathfinderState{};
        state.running = true;
        aStar(&grid, state);
        state.running = !paused;
        // The same reset used by New, Stop & Clear and grid resize.
        state = PathfinderState{};
        check(!state.open && state.closed.empty(), "Cancellation retained resources");
        check(!state.running && !state.initialized, "Cancellation retained active state");

        grid.resize(10, 10);
        state.running = true;
        aStar(&grid, state);
        check(state.open->position.size() == 10, "Restart retained old heap dimensions");
        finish(grid, state);
    }

    grid = makeGrid();
    grid.cells[0][1].state = CellState::Obstacle;
    grid.cells[1][0].state = CellState::Obstacle;
    grid.cells[1][1].state = CellState::Obstacle;
    state = PathfinderState{};
    state.running = true;
    finish(grid, state);
    check(state.noPath && !state.found, "Blocked search reported a path");
    check(state.pushed == 1, "Blocked search did not count the initial node");

    // Leaving the application mid-search also destroys the owned nodes.
    {
        Grid activeGrid = makeGrid();
        PathfinderState active;
        active.running = true;
        aStar(&activeGrid, active);
    }

    // Rejected duplicate allocations must also be freed (checked by ASan/LSan).
    {
        NodeHeap duplicates(2, 2);
        duplicates.push(new ListNode(0, 0));
        duplicates.push(new ListNode(0, 0));
        check(duplicates.nodeHeap.size() == 1, "Heap accepted duplicate coordinates");
    }
    std::cout << "Pathfinding lifecycle checks passed\n";
}
