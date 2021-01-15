#ifndef GRIDMENU_H
#define GRIDMENU_H

#include <imgui/imgui.h>

void updateSolverGrid()
{
    float visc;
    float diff;
    float diss;
    float rho0;
    float us;
    switch (currentSolver)
    {
    case EMPTY:
        break;
    
    case JSSF:
        visc = JSSFSolver.visc;
        diff = JSSFSolver.diff;
        diss = JSSFSolver.diss;
        JSSFSolver.initialize(N,L,JSSFSolver.BOUND,dt,visc,diff,diss);
        break;
    
    case JSSFIter:
        visc = JSSFSolverIter.visc;
        diff = JSSFSolverIter.diff;
        diss = JSSFSolverIter.diss;
        JSSFSolverIter.initialize(N,L,JSSFSolverIter.BOUND,dt,visc,diff,diss);
        break;
    
    case LBM:
        rho0 = LBMSolver.rho0;
        visc = LBMSolver.visc;
        us = LBMSolver.us;
        LBMSolver.initialize(N,L,1/dt,rho0,visc,us);
        break;
    
    case JSSF3D:
        visc = JSSFSolver.visc;
        diff = JSSFSolver.diff;
        diss = JSSFSolver.diss;
        JSSFSolver3D.initialize(N,L,JSSFSolver.BOUND,dt,visc,diff,diss);
        break;
    }
}

void doGridMenu()
{
    static bool isChanged = false;
    static bool updateGrid = false;
    static float dtTemp = dt; //dtTemp is used instead of dt because dt is used outside of just the solvers
    static std::future<void> future;

    // update the grid
    if (updateGrid && !isCalcFrame)
    {
        if (!isUpdating)
        {
            dt = dtTemp;
            future = std::async(std::launch::async, updateSolverGrid);
            isUpdating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            updateRenderer();
            isUpdating = false;
            updateGrid = false;
        }
    }
    if (updateGrid)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }

    // we don't want to be able to change anything when any of the other
    // parts of the program are updating
    if (!isUpdating)
    {
        isChanged |= ImGui::InputInt("Grid Size (>0)", (int*) &N);
        isChanged |= ImGui::InputFloat("Grid Length (>0)", &L);
        isChanged |= ImGui::InputFloat("deltaT in Seconds (>0)", &dtTemp);

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

#endif