#include "forces_menu.h"
#include "../global_vars.h"



void doForceWindow()
{
    static std::vector<std::string> forceList = {""};
    static int currentForce = 0;

    if (current_renderer == None)
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
            if (current_renderer == Dim2)
            {
                ImGui::InputFloat2("Position", forces[currentForce - 1].pos);
                ImGui::InputFloat2("Force", forces[currentForce - 1].force);
            }
            else if (current_renderer == Dim3)
            {
                ImGui::InputFloat3("Position", forces[currentForce - 1].pos);
                ImGui::InputFloat3("Force", forces[currentForce - 1].force);
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
            forceList.erase(forceList.end() - 1);
            forces.erase(forces.begin() + currentForce - 1);
            currentForce-=1;
        }
    }
    ImGui::End();
}