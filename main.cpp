#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "main.h"
#include <chrono>


int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow(
        "Pathfinder Visualization",
        800,
        800,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
        return 1;
    }

    SDL_SetWindowAspectRatio(window, 1.0f, 1.0f);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
        return 1;
    }
    ImGui::CreateContext();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    Grid grid;


    PathfinderState pathfinder;

    const bool* keys = SDL_GetKeyboardState(nullptr);
    bool running = true;
    bool canStart = false;
    bool controlsHovered = false;
    bool showSpeedSlider = false;
    float speed = 0.0f;
    float accumulator = 0.0f;
    float buttonsWidth = 0.0f;
    float deltaTime;  
    float controlsHeight = 40.0f;

    std::chrono::steady_clock::time_point runStart;

    bool timerRunning = false;
    bool showResultPopup = false;

    double elapsedSeconds = 0.0;
    
    while (running)
    {
        controlsHovered = false;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        float menuBarHeight = 0.0f;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::MenuItem("New"))
            {
                for (auto& row : grid.cells)
                {
                    for (auto& cell : row)
                        cell.state = CellState::Empty;
                }
            }
            if (ImGui::MenuItem("Exit"))
            {
                running = false;
            }
            if (ImGui::MenuItem("Small"))
            {
                grid.resize(15, 15);
            }

            if (ImGui::MenuItem("Medium"))
            {
                grid.resize(30, 30);
            }

            if (ImGui::MenuItem("Large"))
            {
                grid.resize(50, 50);
            }
            if (ImGui::MenuItem("Run"))
            {
                if (canStart)
                {
                    if (!timerRunning)
                    {
                        runStart = std::chrono::steady_clock::now();
                        timerRunning = true;
                        pathfinder.found = false;
                    }

                    pathfinder.running = true;
                    accumulator = 0.0f;
                }
            }


            if (ImGui::MenuItem("Pause"))
            {
                pathfinder.running = false;
                accumulator = 0.0f;
            }


            if (ImGui::MenuItem("Stop && Clear"))
            {
                canStart = false;

                pathfinder.running = false;
                pathfinder.initialized = false;
                pathfinder.found = false;

                timerRunning = false;
                showResultPopup = false;

                for (auto& row : grid.cells)
                {
                    for (auto& cell : row)
                    {
                        if (
                            cell.state == CellState::Path ||
                            cell.state == CellState::Visited
                        )
                        {
                            cell.state = CellState::Empty;
                        }
                    }
                }
            }
            menuBarHeight = ImGui::GetFrameHeight();

            controlsHovered |= ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            
            ImGui::EndMainMenuBar();
        }
        if (pathfinder.running)
        {
            if (speed == 0.0f)
                {
                    double start = SDL_GetTicks();

                    while (pathfinder.running && SDL_GetTicks() - start < 8.0)
                    {
                        aStar(&grid, pathfinder);
                    }
                }
            else
            {
                accumulator += ImGui::GetIO().DeltaTime * speed;
                while (accumulator >= 1.0f && pathfinder.running)
                {
                    aStar(&grid, pathfinder);
                    accumulator -= 1.0f;
                }
            }

        }
        if (pathfinder.found && timerRunning)
        {
            auto runEnd = std::chrono::steady_clock::now();

            elapsedSeconds =
                std::chrono::duration<double>(
                    runEnd - runStart
                ).count();

            timerRunning = false;
            showResultPopup = true;

            // já capturamos o resultado
            pathfinder.found = false;
        }

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    float cellSize = std::min(
        static_cast<float>(windowWidth) / grid.cols,
        (static_cast<float>(windowHeight) - menuBarHeight) / grid.rows
    );

        const float speeds[] = {
            30.0f,
            60.0f,
            120.0f,
            150.5f,
            180.0f,
            240.0f,
            0.0f
        };

        const char* labels[] = {
            "0.25x",
            "0.5x",
            "1x",
            "1.25x",
            "1.5x",
            "2x",
            "instant"
        };

        ImVec2 display = ImGui::GetIO().DisplaySize;

        float menuHeight = showSpeedSlider ? 35.0f : 35.0f;
        float menuWidth  = showSpeedSlider ? 390.0f : 45.0f;

        // preso no canto inferior esquerdo
        ImGui::SetNextWindowPos(
            ImVec2(5.0f, display.y - menuHeight - 5.0f)
        );

        ImGui::SetNextWindowSize(
            ImVec2(menuWidth, menuHeight)
        );

        ImGui::SetNextWindowBgAlpha(0.85f);

        ImGui::Begin(
            "##speed-menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings
        );

        if (ImGui::Button(showSpeedSlider ? "v" : "^"))
            showSpeedSlider = !showSpeedSlider;

        controlsHovered |= ImGui::IsItemHovered();

        if (showSpeedSlider)
        {
            for (int i = 0; i < std::size(labels); i++)
            {
                ImGui::SameLine();

                bool selected = speed == speeds[i];

                if (selected)
                {
                    ImGui::PushStyleColor(
                        ImGuiCol_Button,
                        ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive)
                    );
                }

                if (ImGui::Button(labels[i]))
                    speed = speeds[i];

                controlsHovered |= ImGui::IsItemHovered();

                if (selected)
                    ImGui::PopStyleColor();
            }
        }
        controlsHovered |= ImGui::IsWindowHovered();
        ImGui::End();
if (showResultPopup)
{
    ImGui::OpenPopup("Path found");
    showResultPopup = false;
}

if (ImGui::BeginPopupModal(
    "Path found",
    nullptr,
    ImGuiWindowFlags_AlwaysAutoResize
))
{
    controlsHovered = true;

    ImGui::Text(
        "Path found in %.6f seconds",
        elapsedSeconds
    );

    if (ImGui::Button("Close"))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}
float mouseX, mouseY;

SDL_MouseButtonFlags buttons =
    SDL_GetMouseState(&mouseX, &mouseY);

if (
    !pathfinder.running &&
    (buttons & SDL_BUTTON_LMASK) &&
    !controlsHovered &&
    mouseY >= menuBarHeight
)
{
    int col = static_cast<int>(
        (mouseY - menuBarHeight) / cellSize
    );

    int row = static_cast<int>(
        mouseX / cellSize
    );

    if (
        row >= 0 &&
        row < grid.rows &&
        col >= 0 &&
        col < grid.cols
    )
    {
        if (SDL_GetModState() & SDL_KMOD_CTRL)
        {
            if (
                grid.end[0] != -1 &&
                grid.end[1] != -1
            )
            {
                grid.cells
                    [grid.end[0]]
                    [grid.end[1]]
                    .state = CellState::Empty;
            }

            grid.cells[col][row].state =
                CellState::End;

            grid.end[0] = col;
            grid.end[1] = row;
        }

        else if (
            SDL_GetKeyboardState(nullptr)
                [SDL_SCANCODE_SPACE]
        )
        {
            grid.cells[col][row].state =
                CellState::Empty;
        }

        else if (
            SDL_GetModState() & SDL_KMOD_SHIFT
        )
        {
            if (
                grid.start[0] != -1 &&
                grid.start[1] != -1
            )
            {
                grid.cells
                    [grid.start[0]]
                    [grid.start[1]]
                    .state = CellState::Empty;
            }

            grid.cells[col][row].state =
                CellState::Start;

            grid.start[0] = col;
            grid.start[1] = row;

            canStart = true;
        }

        else
        {
            grid.cells[col][row].state =
                CellState::Obstacle;
        }
    }
}

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int row = 0; row < grid.rows; row++)
        {
            for (int col = 0; col < grid.cols; col++)
            {
                SDL_FRect rect = {
                    col * cellSize,
                    menuBarHeight + row * cellSize,
                    cellSize,
                    cellSize
                };

                switch (grid.cells[row][col].state)
                {
                    case CellState::Empty:
                        SDL_SetRenderDrawColor(renderer, 155, 155, 155, 255);
                        break;

                    case CellState::Start:
                        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
                        break;

                    case CellState::End:
                        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
                        break;

                    case CellState::Obstacle:
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        break;

                    case CellState::Visited:
                        SDL_SetRenderDrawColor(renderer, 25, 50, 100, 255);
                        break;

                    case CellState::Path:
                        SDL_SetRenderDrawColor(renderer, 50, 100, 255, 255);
                        break;
                }

                // cor da célula
                SDL_RenderFillRect(renderer, &rect);

                // borda
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
                SDL_RenderRect(renderer, &rect);
            }
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(
            ImGui::GetDrawData(),
            renderer
        );

        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}