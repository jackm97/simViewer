#ifndef GRIDMENU_H
#define GRIDMENU_H

#include <imgui/imgui.h>
#include "fluidMenus.h"

void updateSolverGrid()
{
    float visc;
    float diff;
    float diss;
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
    }
}

void updateRenderer()
{
    Eigen::VectorXf img(N*N*3);
    img.setZero();
    renderer2D.deleteTexture("background");
    renderer2D.addTexture(N,N,"background");
    renderer2D.uploadPix2Tex("background", GL_RGB, GL_FLOAT, img.data());
}

void doGridMenu()
{
    static bool isChanged = false;
    static bool updateGrid = false;
    static float dtTemp = dt;
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

    // do fluid menu (eventually this will
    // be called do solver menu)
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