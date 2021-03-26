#include "p_wave_menu.h"
#include "global_vars.h"

void doPressureWindow()
{
    static std::vector<std::string> pList = {""};
    static int current_p_wave = 0;
    static float x[3];
    static float u[3];
    static float u_imp;
    static float radius;
    static float t_start;
    static bool skadoosh;

    if (currentSolver != LBM)
        return;

    if (ImGui::Begin("Pressure Waves"))
    {
        if (ImGui::BeginCombo("Pressure Wave List", pList[current_p_wave].c_str()))
        {
            for (int n=0; n<pList.size(); n++)
            {
                const bool is_selected = (current_p_wave == n);
                if (ImGui::Selectable(pList[n].c_str(), is_selected))
                    current_p_wave = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (current_p_wave != 0)
        {
            x[0] = p_waves[current_p_wave - 1].x[0];
            x[1] = p_waves[current_p_wave - 1].x[1];
            x[2] = p_waves[current_p_wave - 1].x[2];
            u[0] = p_waves[current_p_wave - 1].u[0];
            u[1] = p_waves[current_p_wave - 1].u[1];
            u[2] = p_waves[current_p_wave - 1].u[2];
            u_imp = p_waves[current_p_wave - 1].u_imp;
            radius = p_waves[current_p_wave - 1].radius;
            t_start = p_waves[current_p_wave - 1].t_start;
            skadoosh = p_waves[current_p_wave - 1].skadoosh;

            if (currentRenderer == DIM2)
            {
                if (ImGui::InputFloat2("Position", x)){
                    p_waves[current_p_wave - 1].x[0] = x[0];
                    p_waves[current_p_wave - 1].x[1] = x[1];
                    p_waves[current_p_wave - 1].x[2] = 0;
                }
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Position", x)){
                    p_waves[current_p_wave - 1].x[0] = x[0];
                    p_waves[current_p_wave - 1].x[1] = x[1];
                    p_waves[current_p_wave - 1].x[2] = x[2];
                }
            }
            if (currentRenderer == DIM2)
            {
                if (ImGui::InputFloat2("Center Velocity", u)){
                    p_waves[current_p_wave - 1].u[0] = u[0];
                    p_waves[current_p_wave - 1].u[1] = u[1];
                    p_waves[current_p_wave - 1].u[2] = 0;
                }
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Center Velocity", u)){
                    p_waves[current_p_wave - 1].u[0] = u[0];
                    p_waves[current_p_wave - 1].u[1] = u[1];
                    p_waves[current_p_wave - 1].u[2] = u[2];
                }
            }
            if (ImGui::InputFloat("Impulse Speed", &u_imp)){
                p_waves[current_p_wave - 1].u_imp = u_imp;
            }
            if (ImGui::InputFloat("Radius", &radius)){
                p_waves[current_p_wave - 1].radius = radius;
            }
            if (ImGui::InputFloat("Start Time", &t_start)){
                p_waves[current_p_wave - 1].t_start = t_start;
            }
            if (ImGui::Checkbox("Skadoosh!", &skadoosh)){
                p_waves[current_p_wave - 1].skadoosh = skadoosh;
            }
        }
        if (ImGui::Button("Add Pressure Wave"))
        {
            pList.push_back("p_wave" + std::to_string(pList.size()));
            p_waves.push_back(jfs::PressureWave());
            current_p_wave = pList.size()-1;
        }
        if (ImGui::Button("Delete Pressure Wave") && current_p_wave!=0)
        {
            pList.erase(pList.end());
            p_waves.erase(p_waves.begin() + current_p_wave - 1);
            current_p_wave-=1;
        }
    }
    ImGui::End();
}