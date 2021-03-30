#include "sources_menu.h"
#include "../global_vars.h"

void doSourceWindow()
{
    static std::vector<std::string> sourceList = {""};
    static int currentSource = 0;
    static float diss = smoke_diss;

    if (currentRenderer == NONE)
        return;

    if (ImGui::Begin("Sources"))
    {
        if (ImGui::BeginCombo("Source List", sourceList[currentSource].c_str()))
        {
            for (int n=0; n<sourceList.size(); n++)
            {
                const bool is_selected = (currentSource == n);
                if (ImGui::Selectable(sourceList[n].c_str(), is_selected))
                    currentSource = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (currentSource != 0)
        {
            if (currentRenderer == DIM2)
            {
                ImGui::InputFloat2("Position", sources[currentSource - 1].pos);
            }
            else if (currentRenderer == DIM3)
            {
                ImGui::InputFloat3("Position", sources[currentSource - 1].pos);
            }
            ImGui::ColorEdit3("Color", sources[currentSource - 1].color);
            ImGui::InputFloat("Strength", &(sources[currentSource - 1].strength));
        }
        if (ImGui::Button("Add Source"))
        {
            sourceList.push_back("source" + std::to_string(sourceList.size()));
            sources.push_back(jfs::Source());
            currentSource = sourceList.size()-1;
        }
        if (ImGui::Button("Delete Source") && currentSource!=0)
        {
            sourceList.erase(sourceList.end());
            sources.erase(sources.begin() + currentSource - 1);
            currentSource-=1;
        }
        ImGui::InputFloat("Dissipation", &diss);
        if (diss != smoke_diss)
        {
            if (ImGui::Button("Update Smoke"))
            {
                smoke_diss = diss;
                updateRenderer = true;
                updateSolver = true;
            }
        }
    }
    ImGui::End();
}