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
    static float dt = .1, visc = 0;

    if (updateSolver && !is_calc_frame)
    {
        if (grid_smoke2d == NULL)
            grid_smoke2d = new jfs::gridSmoke2D(grid_size, grid_length, fluid_btype, iter_per_frame * dt, smoke_diss);
        else
            grid_smoke2d->initialize(grid_size, grid_length, fluid_btype, iter_per_frame * dt, smoke_diss);
            
        if (!is_updating)
        {
            auto initLambda = [](){ jssf_solver_iter->initialize(grid_size, grid_length, fluid_btype, dt, visc); };
            future = std::async(std::launch::async, initLambda);
            is_updating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            is_updating = false;
            updateSolver = false;
        }
    }
    if (updateSolver)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }


    if (!is_updating)
    {
        isChanged |= ImGui::InputFloat("Viscosity", &(visc));
        isChanged |= ImGui::InputFloat("Time Step(s)", &(dt));
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