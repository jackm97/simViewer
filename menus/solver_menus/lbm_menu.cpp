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
    static int iter_per_frame = 1;
    static float rhobounds[2]{-1, -1};

    if (updateSolver && !isCalcFrame)
    {
        if (!isUpdating)
        {
            auto initLambda = [](){LBMSolver->initialize(N,L,fluid_btype,iter_per_frame,rho0,visc,uref);};
            future = std::async(std::launch::async, initLambda);
            isUpdating = true;
        }
        if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
        {
            future.get();
            LBMSolver->setDensityMapping(rhobounds[0], rhobounds[1]);
            isUpdating = false;
            updateSolver = false;

            if (grid_smoke2d == NULL)
                grid_smoke2d = new jfs::gridSmoke2D(N, L, fluid_btype, LBMSolver->TimeStep() * iter_per_frame, smoke_diss);
            else
                grid_smoke2d->initialize(N, L, fluid_btype, LBMSolver->TimeStep() * iter_per_frame, smoke_diss);
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
        isChanged |= ImGui::InputInt("Iterations Per Frame", &(iter_per_frame));
        
        std::string sim_time_step = "Time step (s): ";
        sim_time_step += std::to_string(LBMSolver->TimeStep());
        ImGui::TextUnformatted(sim_time_step.c_str());
        
        std::string sound_speed = "Speed of Sound(m/s): ";
        sound_speed += std::to_string(LBMSolver->soundSpeed());
        ImGui::TextUnformatted(sound_speed.c_str());

        if ( ImGui::Checkbox("View Density", &view_density) );

        if ( view_density ){

            if ( ImGui::InputFloat2("Density Mapping", rhobounds) )
                LBMSolver->setDensityMapping(rhobounds[0], rhobounds[1]);
            std::string current_min_max_rho = "Current min/max density: ";
            float minmaxrho[2];
            LBMSolver->densityExtrema(minmaxrho);
            current_min_max_rho += std::to_string(minmaxrho[0]);
            current_min_max_rho += "/";
            current_min_max_rho += std::to_string(minmaxrho[1]);
            ImGui::TextUnformatted(current_min_max_rho.c_str());
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

