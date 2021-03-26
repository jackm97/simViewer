#ifndef SOLVERMENUS_H
#define SOLVERMENUS_H

#include "solver_menus/jssf_menu.h"
#include "solver_menus/jssf_iter_menu.h"
#include "solver_menus/lbm_menu.h"
#include "solver_menus/jssf3d_menu.h"

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

// frees currentSolver from heap
void releaseMem();

// must be called before newsolver==currentSolver
// allocates newsolver on heap
// frees currentsolver on heap
enum SOLVER_TYPE : unsigned int;
void handleMem(SOLVER_TYPE newsolver);

void doSolverMenu();

#endif