#ifndef FLUIDMENUS_H
#define FLUIDMENUS_H

#include <imgui/imgui.h>
#include <jfs/JSSFSolver.h>

#include <future>


typedef enum {
    EMPTY,
    JSSF = 1,
    JSSFIter = 2
} SOLVER_TYPE;

static const int numSolvers = 3;
static const char* solverNames[numSolvers] = {"", "JSSF", "JSSF Iterative"};
SOLVER_TYPE currentSolver = EMPTY;
bool updateSolver = false;

// fluid solvers
jfs::JSSFSolver<> JSSFSolver(N,L,jfs::ZERO,dt);
jfs::JSSFSolver<jfs::iterativeSolver> JSSFSolverIter(N,L,jfs::ZERO,dt);

void doJSSFMenu()
{
    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolver.initialize(N,L,fluidBound,dt,JSSFSolver.visc,JSSFSolver.diff,JSSFSolver.diss);};
            future = std::async(std::launch::async, initLambda);
            isUpdating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            isUpdating = false;
            updateSolver = false;
        }
    }
    if (updateSolver)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }


    if (!isUpdating)
    {
        isChanged |= ImGui::InputFloat("Viscosity", &(JSSFSolver.visc));
        isChanged |= ImGui::InputFloat("Diffusion", &(JSSFSolver.diff));
        isChanged |= ImGui::InputFloat("Dissipation", &(JSSFSolver.diss));
        if (ImGui::BeginCombo("Boundary Type", bcTypes[currentBC]))
        {
            for (int bc=0; bc < 2; bc++)
            {
                const bool is_selected = (currentBC == bc);
                if (ImGui::Selectable(bcTypes[bc], is_selected))
                    {currentBC = bc; if (bc==0) fluidBound = jfs::ZERO; else fluidBound = jfs::PERIODIC;}

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                    isChanged = true;
                }            
            }
            ImGui::EndCombo();
        }
    

        if (isChanged)
            if (ImGui::Button("Update Fluid Properties"))
            {
                updateSolver = true;
                isChanged = false;
                return;
            }
    }
}

void doJSSFIterMenu()
{
    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolverIter.initialize(N,L,fluidBound,dt,JSSFSolver.visc,JSSFSolver.diff,JSSFSolver.diss);};
            future = std::async(std::launch::async, initLambda);
            isUpdating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            isUpdating = false;
            updateSolver = false;
        }
    }
    if (updateSolver)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }


    if (!isUpdating)
    {
        isChanged |= ImGui::InputFloat("Viscosity", &(JSSFSolver.visc));
        isChanged |= ImGui::InputFloat("Diffusion", &(JSSFSolver.diff));
        isChanged |= ImGui::InputFloat("Dissipation", &(JSSFSolver.diss));
        if (ImGui::BeginCombo("Boundary Type", bcTypes[currentBC]))
        {
            for (int bc=0; bc < 2; bc++)
            {
                const bool is_selected = (currentBC == bc);
                if (ImGui::Selectable(bcTypes[bc], is_selected))
                    {currentBC = bc; if (bc==0) fluidBound = jfs::ZERO; else fluidBound = jfs::PERIODIC;}

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                    isChanged = true;
                }            
            }
            ImGui::EndCombo();
        }
    

        if (isChanged)
            if (ImGui::Button("Update Fluid Properties"))
            {
                updateSolver = true;
                isChanged = false;
                return;
            }
    }
}

void doSolverMenu()
{
    static int currentSolverTmp = currentSolver;
    static bool isChanged = false;
    
    if (!isUpdating)
    {
        if (ImGui::BeginCombo("Solver Type", solverNames[currentSolverTmp]))
        {
            for (int s=0; s < numSolvers; s++)
            {
                const bool is_selected = (currentSolverTmp == s);
                if (ImGui::Selectable(solverNames[s], is_selected))
                    currentSolverTmp = s;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                    isChanged = true;
                }            
            }
            ImGui::EndCombo();
        }

        if (isChanged)
            if (ImGui::Button("Update Solver"))
            {
                ImGui::TextUnformatted("Updating...");
                updateSolver = true;
                isChanged = false;
                return;
            }
    }

    switch (currentSolverTmp)
    {
    case 0:
        if (!isChanged) currentSolver = EMPTY;
        break;
    case 1:
        if (!isChanged) 
        {
            doJSSFMenu();
            currentSolver = JSSF;
        }
        break;
    case 2:
        if (!isChanged) 
        {
            doJSSFIterMenu();
            currentSolver = JSSFIter;
        }
    }
}

#endif