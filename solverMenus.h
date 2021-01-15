#ifndef SOLVERMENUS_H
#define SOLVERMENUS_H

#include <imgui/imgui.h>

#include <future>

// DEV NOTES:
// General Strucutre for Solver Menus:
//
// void doSolverTypeMenu
// {
//     setup static vars for solver
//     static std::future<void> future; // for updating solver properties
//     static bool isChanged = false; // if solver properties are changed in the menu

//     if (updateSolver && !isCalcFrame) // if solverMenu is telling us to change solver and no frame is being calculated
//     {
//         if (!isUpdating) // isUpdating tells us that we are waiting for a future to complete
//         {
//             lambdaFunc function to update solverNames
//             future = std::async(std::launch::async, lambdaFunc);
//             isUpdating = true;
//         }
//         if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) ) // future is complete
//         {
//             future.get();
//             isUpdating = false;
//             updateSolver = false;
//         }
//     }
//     if (updateSolver)
//     {
//         ImGui::TextUnformatted("Updating...");
//         return;
//     }

//     if (!isUpdating)
//     {
//         do menu stuff for solver
//         make sure to update isChanged if something is changed
    

//         if (isChanged)
//             if (ImGui::Button("Update Fluid Properties"))
//             {
//                 updateSolver = true;
//                 isChanged = false;
//                 return;
//             }
        
//     }
// }

void doJSSFMenu()
{
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    static float visc = JSSFSolver.visc, diff = JSSFSolver.diff, diss = JSSFSolver.diss;
    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolver.initialize(N,L,fluidBound,dt,visc,diff,diss);};
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
        isChanged |= ImGui::InputFloat("Viscosity", &(visc));
        isChanged |= ImGui::InputFloat("Diffusion", &(diff));
        isChanged |= ImGui::InputFloat("Dissipation", &(diss));
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
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;
    static float visc = JSSFSolverIter.visc, diff = JSSFSolverIter.diff, diss = JSSFSolverIter.diss;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolverIter.initialize(N,L,fluidBound,dt,visc,diff,diss);};
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

void doLBMMenu()
{
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;
    static float rho0 = LBMSolver.rho0, visc = LBMSolver.visc, us = LBMSolver.us;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){LBMSolver.initialize(N,L,1/dt,rho0,visc,us);};
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
        isChanged |= ImGui::InputFloat("Density", &(rho0));
        isChanged |= ImGui::InputFloat("Viscosity", &(visc),0,0,"%.0e");
        isChanged |= ImGui::InputFloat("Speed of Sound", &(us));
    

        if (isChanged)
            if (ImGui::Button("Update Fluid Properties"))
            {
                updateSolver = true;
                isChanged = false;
                return;
            }
    }
}

void doJSSF3DMenu()
{
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    static float visc = JSSFSolver3D.visc, diff = JSSFSolver3D.diff, diss = JSSFSolver3D.diss;
    static jfs::BOUND_TYPE fluidBound = jfs::ZERO;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolver3D.initialize(N,L,fluidBound,dt,visc,diff,diss);};
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
        isChanged |= ImGui::InputFloat("Viscosity", &(visc));
        isChanged |= ImGui::InputFloat("Diffusion", &(diff));
        isChanged |= ImGui::InputFloat("Dissipation", &(diss));
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

void minimizeMem()
{
    if (updateSolver)
    {
        JSSFSolver.initialize(1,L,jfs::ZERO,dt);
        JSSFSolverIter.initialize(1,L,jfs::ZERO,dt);
        LBMSolver.initialize(1,L,1/dt);
        JSSFSolver3D.initialize(1,L,jfs::ZERO,dt);
        updateSolver = true;
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
                    if (currentSolver != s)
                    {
                        currentSolverTmp = s;
                        isChanged = true;
                    }


                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }            
            }
            ImGui::EndCombo();
        }

        if (isChanged)
            if (ImGui::Button("Update Solver"))
            {
                ImGui::TextUnformatted("Updating...");
                isAnimating = false;
                nextFrame = false;
                updateSolver = true;
                return;
            }
    }

    // this section reduces the memory allocation
    // for unused solvers
    static std::future<void> future;
    if (updateSolver && isChanged)
    {
        if (!isUpdating)
        {
            future = std::async(std::launch::async, minimizeMem);
            isUpdating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            currentSolver = (SOLVER_TYPE) currentSolverTmp;
            updateRenderer();
            isUpdating = false;
            isChanged = false;
        }
    }
    if (isUpdating && isChanged)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }

    if (!isChanged)
        switch (currentSolver)
        {
        case EMPTY:
            updateSolver = false;
            break;

        case JSSF:
            doJSSFMenu();
            break;
        case JSSFIter:
            doJSSFIterMenu();
            break;
        case LBM:
            doLBMMenu();
            break;
        case JSSF3D:
            doJSSF3DMenu();
            break;
        }
}

#endif