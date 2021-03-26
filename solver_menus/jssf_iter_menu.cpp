#include "jssf_iter_menu.h"
#include "../global_vars.h"

#include <future>

void doJSSFIterMenu()
{
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Periodic"};
    static int currentBC = 0;
    static std::future<void> future;

    static jfs::BoundType fluid_btype = jfs::ZERO;
    static float visc = 0, diff = 0, diss = 0;

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){JSSFSolverIter->initialize(N,L,fluid_btype,dt,visc,diff,diss);};
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
                    {currentBC = bc; if (bc==0) fluid_btype = jfs::ZERO; else fluid_btype = jfs::PERIODIC;}

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