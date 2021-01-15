#ifndef FORCESMENUS_H
#define FORCESMENUS_H

#include <imgui/imgui.h>
#include <vector>
#include <string>


void doForceWindow()
{
    static std::vector<std::string> forceList = {""};
    static int currentForce = 0;
    static float pos[3];
    static float force[3];

    if (currentRenderer == NONE)
        return;

    if (ImGui::Begin("Forces"))
    {
        if (ImGui::BeginCombo("Force List", forceList[currentForce].c_str()))
        {
            for (int n=0; n<forceList.size(); n++)
            {
                const bool is_selected = (currentForce == n);
                if (ImGui::Selectable(forceList[n].c_str(), is_selected))
                    currentForce = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (currentForce != 0)
        {
            pos[0] = forces[currentForce - 1].x;
            pos[1] = forces[currentForce - 1].y;
            force[0] = forces[currentForce - 1].Fx;
            force[1] = forces[currentForce - 1].Fy;

            if (currentRenderer == DIM2)
            {
                if (ImGui::InputFloat2("Position", pos)){
                    forces[currentForce - 1].x = pos[0];
                    forces[currentForce - 1].y = pos[1];
                    forces[currentForce - 1].z = 0;
                }
                if (ImGui::InputFloat2("Force", force)){
                    forces[currentForce - 1].Fx = force[0];
                    forces[currentForce - 1].Fy = force[1];
                    forces[currentForce - 1].Fz = 0;
                }
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Position", pos)){
                    forces[currentForce - 1].x = pos[0];
                    forces[currentForce - 1].y = pos[1];
                    forces[currentForce - 1].z = pos[2];
                }
                if (ImGui::InputFloat3("Force", force)){
                    forces[currentForce - 1].Fx = force[0];
                    forces[currentForce - 1].Fy = force[1];
                    forces[currentForce - 1].Fz = force[2];
                }
            }
        }
        if (ImGui::Button("Add Force"))
        {
            forceList.push_back("force" + std::to_string(forceList.size()));
            forces.push_back(jfs::Force());
            currentForce = forceList.size()-1;
        }
        if (ImGui::Button("Delete Force") && currentForce!=0)
        {
            forceList.erase(forceList.end());
            forces.erase(forces.begin() + currentForce - 1);
            currentForce-=1;
        }
    }
    ImGui::End();
}

#endif