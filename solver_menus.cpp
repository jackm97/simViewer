#include "solver_menus.h"
#include "global_vars.h"

#include "render_funcs.h"

#include <future>


// frees currentSolver from heap
void releaseMem()
{
    switch (currentSolver)
    {
    case JSSF:
        delete JSSFSolver;
        break;
    case JSSFIter:
        delete JSSFSolverIter;
        break;
    case LBM:
        delete LBMSolver;
        break;
    case JSSF3D:
        delete JSSFSolver3D;
        break;
    }  
}

// must be called before newsolver==currentSolver
// allocates newsolver on heap
// frees currentsolver on heap
void handleMem(SOLVER_TYPE newsolver)
{
    if (currentSolver == newsolver)
    {
        std::cerr << "ERROR: Memory handling of solver types" << std::endl;
        exit(-1);
    }
    releaseMem();

    switch (newsolver)
    {
    case JSSF:
        JSSFSolver = new jfs::JSSFSolver<>(1, L, jfs::ZERO, dt);
        break;
    case JSSFIter:
        JSSFSolverIter = new jfs::JSSFSolver<jfs::iterativeSolver>(1, L, jfs::ZERO, dt);
        break;
    case LBM:
        LBMSolver = new jfs::LBMSolver (1, L, jfs::ZERO, 1/dt);
        break;
    case JSSF3D:
        JSSFSolver3D = new jfs::JSSFSolver3D<jfs::iterativeSolver>(1, L, jfs::ZERO, dt);
        break;
    }
    updateSolver = true;
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
            auto handleMemLambda = [](){handleMem((SOLVER_TYPE) currentSolverTmp);};
            future = std::async(std::launch::async, handleMemLambda);
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