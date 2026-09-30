#pragma once
#include <vector>
#include <list>
enum class CellState {
    Empty,
    Start,
    End,
    Obstacle,
    Visited,
    Path
};
struct Cell {
    CellState state = CellState::Empty;
};

struct Grid {
    int cols = 30;
    int rows = 30;
    int start[2] = {-1, -1};
    int end[2] = {-1, -1};
    std::vector<std::vector<Cell>> cells;

    Grid() : cells(rows, std::vector<Cell>(cols)) {}

    void resize(int newCols, int newRows)
    {
        std::vector<std::vector<Cell>> newCells(
            newCols,
            std::vector<Cell>(newRows)
        );

        int copyCols = std::min(cols, newCols);
        int copyRows = std::min(rows, newRows);

        for (int col = 0; col < copyCols; col++)
        {
            for (int row = 0; row < copyRows; row++)
            {
                CellState state = cells[col][row].state;

                if (
                    state == CellState::Obstacle ||
                    state == CellState::Start ||
                    state == CellState::End
                )
                {
                    newCells[col][row].state = state;
                }
            }
        }

        cells = std::move(newCells);

        cols = newCols;
        rows = newRows;

        // Start saiu do novo grid
        if (
            start[0] < 0 || start[0] >= cols ||
            start[1] < 0 || start[1] >= rows
        )
        {
            start[0] = -1;
            start[1] = -1;
        }

        // End saiu do novo grid
        if (
            end[0] < 0 || end[0] >= cols ||
            end[1] < 0 || end[1] >= rows
        )
        {
            end[0] = -1;
            end[1] = -1;
        }
    }
};

struct ListNode {
    int val[2];
    //path cost: start node -> this node
    float gCost;
    //heuristic cost: this node -> end
    float hCost;
    ListNode *parent;
    ListNode(): val{0, 0}, gCost(0.0f), hCost(0.0f),parent(nullptr) {};
    ListNode(int x, int y): val{x, y}, hCost(0.0f), gCost(0.0f), parent(nullptr) {};
    ListNode(int x, int y, ListNode* parent): val{x, y}, hCost(0.0f), gCost(0.0f) ,parent(parent){};
    ListNode(int x, int y, float z, float g, ListNode* parent): val{x, y}, hCost(z), gCost(g) ,parent(parent){};

    float fCost() const {
        return gCost + hCost;
    }
};

struct PathfinderState
{
    bool running = false;
    bool initialized = false;
    bool found = false;
};


struct NodeHeap
{
    std::vector<std::vector<int>> position;
    std::vector<ListNode*> nodeHeap;
    NodeHeap(int cols, int rows): position(cols, std::vector<int>(rows, -1)){};
    void swapNodes(int a, int b)
    {
        std::swap(nodeHeap[a], nodeHeap[b]);
        position[nodeHeap[a]->val[0]][nodeHeap[a]->val[1]] = a;
        position[nodeHeap[b]->val[0]][nodeHeap[b]->val[1]] = b;
    };

    bool highPriority(const ListNode* a, const ListNode* b)
    {
        float aCost = a->fCost();
        float bCost = b->fCost();

        return aCost == bCost
            ? a->hCost < b->hCost
            : aCost < bCost;
    };

    void siftUp(int i)
    {
        while (i > 0)
        {
            int parent = (i - 1) / 2;
            if (!highPriority(nodeHeap[i], nodeHeap[parent]))
                break;
            swapNodes(i, parent);
            i = parent;
        }
    };

    void siftDown(int i)
    {
        while (2 * i + 1 < nodeHeap.size())
        {
            int left  = 2 * i + 1;
            int right = 2 * i + 2;
            int winner = left;
            if (right < nodeHeap.size())
                winner = highPriority(nodeHeap[left], nodeHeap[right]) ? left : right;
            if (!highPriority(nodeHeap[winner], nodeHeap[i]))
                break;
            swapNodes(i, winner);
            i = winner;
        }
    };
    
    void push(ListNode* node)
    {
        int i = nodeHeap.size();
        nodeHeap.push_back(node);
        position[node->val[0]][node->val[1]] = i;
        siftUp(i);
    };
    ListNode* top()
    {
        return !empty() ? nodeHeap[0] : nullptr;
    };
    ListNode* pop()
    {
        ListNode* to_pop = top();
        if(to_pop){
            int last = nodeHeap.size() - 1;
            swapNodes(last, 0);
            position[to_pop->val[0]][to_pop->val[1]] = -1;
            nodeHeap.pop_back();
            if (!empty())
                siftDown(0);
            return to_pop;
        }
        return nullptr;
    };

    ListNode* find(int x, int y)
    {
        int i = position[x][y];
        return i != -1? nodeHeap[i] : nullptr;
    };
    void decreaseKey(int x, int y, float newG, ListNode* newParent)
    {
        int i = position[x][y];
        if (i != -1 && nodeHeap[i] ->gCost > newG)
        {
            nodeHeap[i] -> gCost = newG;
            nodeHeap[i]-> parent = newParent;
            siftUp(i);
        }
        return;
    };

    bool empty(){
        return nodeHeap.size() == 0 ? true : false;
    };
        
};

std::vector<ListNode*> aStar(Grid* grid, PathfinderState& state);