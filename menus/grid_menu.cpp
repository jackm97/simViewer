#include "grid_menu.h"
#include "../global_vars.h"

#include "solver_menus.h"
#include "../render_funcs.h"

#include <future>

void doGridMenu()
{
    static bool isChanged = false;
    static bool updateGrid = false;
    static int fps_tmp = (int) max_fps; 
    static std::future<void> future;

    // update the grid
    if (updateGrid && !isCalcFrame)
    {
        max_fps = (float) fps_tmp;
        updateSolver = true;
        updateGrid = false;
        updateRenderer = true;
    }
    // if (updateGrid)
    // {
    //     ImGui::TextUnformatted("Updating...");
    //     return;
    // }

    // we don't want to be able to change anything when any of the other
    // parts of the program are updating
    if (!isUpdating)
    {
        isChanged |= ImGui::InputInt("Grid Size (>0)", (int*) &N);
        isChanged |= ImGui::InputFloat("Grid Length (>0)", &L);
        isChanged |= ImGui::InputInt("Maximum Simulation FPS(0 is uncapped)", &fps_tmp);

        // if something is changed
        // add button to update grid
        if (isChanged)
            if (ImGui::Button("Update Grid"))
            {
                updateGrid = true;
                isChanged = false;
                return;
            }
    }

    // do multi-threading menu
    #ifdef USE_OPENMP
    static int num_threads = Eigen::nbThreads();
    static bool changeThreads = false;
    if (!isUpdating && !isChanged && ImGui::TreeNode("Multi-threading"))
    {
        if(ImGui::InputInt("Number of Threads", &num_threads))
            changeThreads = true;
        if(!isUpdating && !isAnimating && !nextFrame)
        {
            if (changeThreads && ImGui::Button("Update Threads"))
            {
                Eigen::setNbThreads(num_threads);
                changeThreads = false;
            }
        }
        std::string thread_info = "Current Eigen Threads: ";
        thread_info +=  std::to_string(Eigen::nbThreads());
        ImGui::TextUnformatted(thread_info.c_str());
        ImGui::TreePop();
    }
    #endif

    // do solver menu
    if (!isChanged && ImGui::TreeNode("Solver"))
    {
        doSolverMenu();
        ImGui::TreePop();
    }

}

void doMainWindow()
{
    if (ImGui::Begin("Main"))
        doGridMenu();
    ImGui::End();
}