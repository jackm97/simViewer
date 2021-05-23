#include "lbm_menu.h"
#include "../global_vars.h"

#include <future>

void doLBMMenu()
{
    static bool isChanged = false;
    static const char* bcTypes[2] = {"Zero", "Damped"};
    static int currentBC = 0;
    static std::future<void> future;

    static jfs::BoundType fluid_btype = jfs::ZERO;
    static float rho0 = 1.3, visc = 1e-4, uref = 1;
    static float rhobounds[2]{-1, -1};

    if (updateSolver && !is_calc_frame)
    {
        if (!is_updating)
        {
            auto initLambda = [](){lbm_solver->Initialize(grid_size, grid_length, fluid_btype, rho0, visc, uref);};
            future = std::async(std::launch::async, initLambda);
            is_updating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            lbm_solver->SetDensityMapping(rhobounds[0], rhobounds[1]);
            is_updating = false;
            updateSolver = false;

            if (grid_smoke2d == NULL)
                grid_smoke2d = new jfs::gridSmoke2D(grid_size, grid_length, fluid_btype, lbm_solver->TimeStep() * iter_per_frame, smoke_diss);
            else
                grid_smoke2d->initialize(grid_size, grid_length, fluid_btype, iter_per_frame * lbm_solver->TimeStep(), smoke_diss);
        }
    }
    if (updateSolver)
    {
        ImGui::TextUnformatted("Updating...");
        return;
    }


    if (!is_updating)
    {
        isChanged |= ImGui::InputFloat("Density", &(rho0));
        isChanged |= ImGui::InputFloat("Viscosity", &(visc), 0, 0, "%.0e");
        isChanged |= ImGui::InputFloat("Reference Speed", &(uref));
        if (ImGui::BeginCombo("Boundary Type", bcTypes[currentBC]))
        {
            for (int bc=0; bc < 2; bc++)
            {
                const bool is_selected = (currentBC == bc);
                if (ImGui::Selectable(bcTypes[bc], is_selected))
                    {currentBC = bc; if (bc==0) fluid_btype = jfs::ZERO; else fluid_btype = jfs::DAMPED;}

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                    isChanged = true;
                }            
            }
            ImGui::EndCombo();
        }
        
        std::string sim_time_step = "Time step (s): ";
        sim_time_step += std::to_string(lbm_solver->TimeStep());
        ImGui::TextUnformatted(sim_time_step.c_str());
        
        std::string sound_speed = "Speed of Sound(m/s): ";
        sound_speed += std::to_string(lbm_solver->SoundSpeed());
        ImGui::TextUnformatted(sound_speed.c_str());

        if ( ImGui::Checkbox("View Density", &view_density) );

        if ( view_density ){

            if ( ImGui::InputFloat2("Density Mapping", rhobounds) )
                lbm_solver->SetDensityMapping(rhobounds[0], rhobounds[1]);
            /*std::string current_min_max_rho = "Current min/max density: ";
            float minmaxrho[2];
            lbm_solver->DensityExtrema(minmaxrho);
            current_min_max_rho += std::to_string(minmaxrho[0]);
            current_min_max_rho += "/";
            current_min_max_rho += std::to_string(minmaxrho[1]);
            ImGui::TextUnformatted(current_min_max_rho.c_str());*/
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

