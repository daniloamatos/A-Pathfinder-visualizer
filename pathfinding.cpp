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
    static int expanded = 0;
    static int pushed = 0;
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
    static std::vector<std::vector<bool>> closed;
    static NodeHeap* open = nullptr;
    

    static ListNode* node = nullptr;

    if (!state.initialized)
    {
        state.found = false;
        delete open;
        //nodes to be processed
        open = new NodeHeap(grid->cols, grid->rows);
        closed = std::vector<std::vector<bool>>(grid->cols,std::vector<bool>(grid->rows, false));
        node = new ListNode();
            node->val[0] = grid->start[0];
            node->val[1] = grid->start[1];
            node->gCost = 0.0f;
            node->hCost = hCostCalc(grid->start[0], grid->start[1]);
            open->push(node);

        state.initialized = true;
    }
    if(open->empty())
    {
        state.running = false;
        state.initialized = false;
        return {};
    }
        
    node = open->top();
    open->pop();
    if (closed[node->val[0]][node->val[1]])
        return {};
    SDL_Log(
        "(%d,%d) g=%.2f h=%.2f f=%.2f",
        node->val[0], node->val[1],
        node->gCost, node->hCost, node->fCost()
    );
    expanded++;
    for(int i = 0; i < 8; i++)
    {
        int x = node->val[0] + directions[i][0];
        int y = node->val[1] + directions[i][1];
        if (x < 0 || x >= height || y < 0 || y >= width)
            continue;
        else if(x == grid->end[0] && y == grid->end[1])
        {
            ListNode* endNode = new ListNode(x, y, hCostCalc(x,y), gCostCalc(node->gCost, i), node);
            node = endNode;
            while(node->parent != nullptr)
            {
                node = node->parent;
                x = node->val[0];
                y = node->val[1];
                if(!(x == grid->start[0] && y == grid->start[1]))
                    grid->cells[x][y].state = CellState::Path;
            }

            SDL_Log("expanded: %d | pushed: %d", expanded, pushed);

            state.found = true;
            state.running = false;
            state.initialized = false;
            return{};
        }
        else if (closed[x][y] || grid->cells[x][y].state == CellState::Obstacle)
            continue;
        Cell& thisCell = grid->cells[x][y];
        thisCell.state = CellState::Visited;
        float newGCost = gCostCalc(node->gCost, i);
        ListNode* existing = open->find(x, y);
        if (existing)
        {
            if (newGCost < existing->gCost)
                open->decreaseKey(x, y, newGCost, node);
        }
        open->push(new ListNode(x, y, hCostCalc(x,y), newGCost, node));
        pushed++;

        
    }
    closed[node->val[0]][node->val[1]] = true;
return {};
}

