#ifndef SOURCESMENUS_H
#define SOURCESMENUS_H


#include <imgui/imgui.h>
#include <vector>
#include <string>


void doSourceWindow()
{
    static std::vector<std::string> sourceList = {""};
    static int currentSource = 0;
    static float pos[3];
    static float color[3];
    static float strength;

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
            pos[0] = sources[currentSource - 1].x;
            pos[1] = sources[currentSource - 1].y;
            color[0] = sources[currentSource - 1].color(0);
            color[1] = sources[currentSource - 1].color(1);
            color[2] = sources[currentSource - 1].color(2);
            strength = sources[currentSource - 1].strength;

            if (currentRenderer == DIM2)
            {
                if (ImGui::InputFloat2("Position", pos)){
                    sources[currentSource - 1].x = pos[0];
                    sources[currentSource - 1].y = pos[1];
                    sources[currentSource - 1].z = 0;
                }
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Position", pos)){
                    sources[currentSource - 1].x = pos[0];
                    sources[currentSource - 1].y = pos[1];
                    sources[currentSource - 1].z = pos[2];
                }
            }
            if (ImGui::ColorEdit3("Color", color)){
                sources[currentSource - 1].color(0) = color[0];
                sources[currentSource - 1].color(1) = color[1];
                sources[currentSource - 1].color(2) = color[2];
            }
            if (ImGui::InputFloat("Strength", &strength)){
                sources[currentSource - 1].strength = strength;
            }
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
    }
    ImGui::End();
}

#endif