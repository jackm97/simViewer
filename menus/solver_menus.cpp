#include "solver_menus.h"
#include "../global_vars.h"

#include "../render_funcs.h"

#include <future>


// frees currentSolver from heap
void releaseMem()
{
    switch (currentSolver)
    {
    case Jssf:
        delete jssf_solver;
        break;
    case JssfIter:
        delete jssf_solver_iter;
        break;
    case Lbm:
        delete lbm_solver;
        break;
    case Jssf3D:
        delete jssf_solver_3d;
        break;
    }  
}

// must be called before newsolver==currentSolver
// allocates newsolver on heap
// frees currentsolver on heap
void handleMem(SolverType newsolver)
{
    if (currentSolver == newsolver)
    {
        std::cerr << "ERROR: Memory handling of solver types" << std::endl;
        exit(-1);
    }
    releaseMem();

    switch (newsolver)
    {
    case Jssf:
        jssf_solver = new jfs::JSSFSolver<>(1, grid_length, jfs::ZERO, .1);
        break;
    case JssfIter:
        jssf_solver_iter = new jfs::JSSFSolver<jfs::iterativeSolver>(1, grid_length, jfs::ZERO, .1);
        break;
    case Lbm:
        lbm_solver = new jfs::CudaLBMSolver (1, grid_length, jfs::ZERO, 1);
        break;
    case Jssf3D:
        jssf_solver_3d = new jfs::JSSFSolver3D<jfs::iterativeSolver>(1, grid_length, jfs::ZERO, .1);
        break;
    }
    updateSolver = true;
    
    if (grid_smoke2d != NULL)
        delete grid_smoke2d;
    if (grid_smoke3d != NULL)
        delete grid_smoke3d;

    grid_smoke2d = NULL;
    grid_smoke3d = NULL;
}

void doSolverMenu()
{
    static int currentSolverTmp = currentSolver;
    static bool isChanged = false;
    
    if (!is_updating)
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
                is_animating = false;
                next_frame = false;
                updateSolver = true;
                return;
            }
    }

    // this section reduces the memory allocation
    // for unused solvers
    static std::future<void> future;
    if (updateSolver && isChanged)
    {
        if (!is_updating)
        {
            auto handleMemLambda = [](){handleMem((SolverType) currentSolverTmp);};
            future = std::async(std::launch::async, handleMemLambda);
            is_updating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            currentSolver = (SolverType) currentSolverTmp;
            update_renderer = true;
            is_updating = false;
            isChanged = false;
        }
    }
    if (is_updating && isChanged)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }

    if (!isChanged)
        switch (currentSolver)
        {
        case Empty:
            updateSolver = false;
            break;

        case Jssf:
            doJSSFMenu();
            break;
        case JssfIter:
            doJSSFIterMenu();
            break;
        case Lbm:
            doLBMMenu();
            break;
        case Jssf3D:
            doJSSF3DMenu();
            break;
        }
}