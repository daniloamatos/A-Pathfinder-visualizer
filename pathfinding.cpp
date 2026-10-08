#include <algorithm>
#include <vector>
#include "main.h"
#include <cmath>
#include <queue>
#include <iostream>
#include <SDL3/SDL.h>


std::vector<ListNode*> aStar(Grid* grid, PathfinderState& state)
{
    int height = grid->cols;
    int width = grid->rows;
    float diagonalCost = std::sqrt(2.0f);
    int& expanded = state.expanded;
    int& pushed = state.pushed;
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1},
                            { 0, -1},           { 0, 1},
                            { 1, -1}, { 1, 0},  { 1, 1}};

    //hCost calculator
    auto hCostCalc = [&](int thisX, int thisY)
    {
        int endX = grid->end[0];
        int endY = grid->end[1];
        int distanceX = abs(endX - thisX);
        int distanceY = abs(endY - thisY);
        //Calculate the cost using the most diagonal moves possible
        return std::min(distanceX, distanceY) * diagonalCost + (std::max(distanceX, distanceY) - std::min(distanceX, distanceY));
    };
    auto gCostCalc = [&](float nodeGCost, int i)
    {     
        //Return gCost adding the latest step to the previous GCost
        return nodeGCost + (i == 0 || i == 2 || i == 5 || i == 7 ? diagonalCost : 1.0f);
    };
    //nodes already processed
    auto& closed = state.closed;
    auto& open = state.open;

    if (!state.initialized)
    {
        expanded = 0;
        pushed = 0;
        state.found = false;
        state.noPath = false;
        //nodes to be processed
        open = std::make_unique<NodeHeap>(grid->cols, grid->rows);
        closed = std::vector<std::vector<bool>>(grid->cols,std::vector<bool>(grid->rows, false));
        ListNode* node = new ListNode();
            node->val[0] = grid->start[0];
            node->val[1] = grid->start[1];
            node->gCost = 0.0f;
            node->hCost = hCostCalc(grid->start[0], grid->start[1]);
            open->push(node);
            pushed++;

        state.initialized = true;
    }
    if(open->empty())
    {
        state.noPath = true;
        state.releaseSearch();
        return {};
    }
        
    ListNode* node = open->pop();
    if (closed[node->val[0]][node->val[1]])
        return {};
    SDL_Log(
        "(%d,%d) g=%.2f h=%.2f f=%.2f",
        node->val[0], node->val[1],
        node->gCost, node->hCost, node->fCost()
    );
    expanded++;

    // O destino só é confirmado quando chega ao topo da heap. Isso preserva
    // a garantia de optimalidade do A*.
    if (node->val[0] == grid->end[0] && node->val[1] == grid->end[1])
    {
        ListNode* pathNode = node;
        while (pathNode->parent != nullptr)
        {
            pathNode = pathNode->parent;
            int x = pathNode->val[0];
            int y = pathNode->val[1];
            if (!(x == grid->start[0] && y == grid->start[1]))
                grid->cells[x][y].state = CellState::Path;
        }

        SDL_Log("expanded: %d | pushed: %d", expanded, pushed);
        state.found = true;
        state.releaseSearch();
        return {};
    }

    for(int i = 0; i < 8; i++)
    {
        int x = node->val[0] + directions[i][0];
        int y = node->val[1] + directions[i][1];
        if (x < 0 || x >= height || y < 0 || y >= width)
            continue;
        if ((i == 0 || i == 2 || i == 5 || i == 7) &&
            (grid->cells[node->val[0]][y].state == CellState::Obstacle ||
             grid->cells[x][node->val[1]].state == CellState::Obstacle))
            continue;
        else if (closed[x][y] || grid->cells[x][y].state == CellState::Obstacle)
            continue;
        Cell& thisCell = grid->cells[x][y];
        if (!(x == grid->end[0] && y == grid->end[1]))
            thisCell.state = CellState::Visited;
        float newGCost = gCostCalc(node->gCost, i);
        ListNode* existing = open->find(x, y);
        if (existing)
        {
            if (newGCost < existing->gCost)
                open->decreaseKey(x, y, newGCost, node);
        }
        else
        {
            open->push(new ListNode(x, y, hCostCalc(x,y), newGCost, node));
            pushed++;
        }

        
    }
    closed[node->val[0]][node->val[1]] = true;
return {};
}

