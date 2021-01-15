#ifndef ANIMATIONMENU_H
#define ANIMATIONMENU_H

#include <string>
#include <vector>

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <Eigen/Eigen>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <filesystem>

void cacheFrame(int& cache_frame, std::string cache_loc, std::string cache_name)
{    
    switch (currentSolver)
    {
    case JSSF:
        JSSFSolver.getImage(img);
        break;

    case JSSFIter:
        JSSFSolverIter.getImage(img);
        break;

    case LBM:
        LBMSolver.getImage(img);
        break;

    case JSSF3D:
        JSSFSolver3D.getImage(img);
        break;
    }

    switch (currentRenderer)
    {
    case DIM2:
        Eigen::VectorX<unsigned char> img8bit;
        img8bit = (img * 255).template cast <unsigned char>();
        std::string full_file_path = cache_loc + cache_name + "." + std::to_string(cache_frame) + ".png";
        int flipBoolTmp = stbi__flip_vertically_on_write;
        stbi_flip_vertically_on_write(1);
        stbi_write_png(full_file_path.c_str(), N, N, 3, img8bit.data(), 3*N);
        stbi_flip_vertically_on_write(flipBoolTmp);
        break;

    }

    cache_frame += 1;

}

bool clearCache(std::string cache_loc)
{

    namespace fs = std::filesystem;

    ImGui::TextUnformatted("Clear Cache?");
    if (ImGui::Button("OK"))
    {
        std::vector<std::string> files;
        for(auto& p: fs::directory_iterator(cache_loc.c_str()))
            files.push_back(p.path());

        for (int f = 0; f < files.size(); f++)
            fs::remove(files[f]);

        return true;
    }
    if (ImGui::Button("Cancel"))
    {
        return true;
    }
    return false;
}

void doAnimationMenu(bool& checkDone, bool& acknowledgeFailedStep)
{

    // terminate animation if step failed
    if (failedStep && (isAnimating || nextFrame || acknowledgeFailedStep))
    {
        acknowledgeFailedStep = true;
        isAnimating = false;
        nextFrame = false;
        checkDone = false;
        ImGui::Text("Failed frame!");
        if (ImGui::Button("OK")){
            failedStep = false;
            acknowledgeFailedStep = false;
            isResetting = true;
        }
        return;
    }
    
    if (isUpdating)
    {
        ImGui::Text("Updating...");
        return;
    }
    else if (checkDone && isCalcFrame)
    {
        ImGui::Text("Finishing frame...");
        return;
    }
    else if (checkDone && !nextFrame && !isAnimating)
    {
        checkDone = false;
    }
    

    if (!isAnimating && ImGui::Button("Animate")) isAnimating=true;
    
    else if (isAnimating && (checkDone = ImGui::Button("Stop"))) isAnimating=false;

    if (!checkDone && !isAnimating && (checkDone = ImGui::Button("Next Frame"))) nextFrame=true;
    
    if (!checkDone && (checkDone = ImGui::Button("Reset"))) isResetting = true;

    ImGui::TextUnformatted(("FPS: " + std::to_string(std::round(fps*1000)/1000.)).c_str());

}

void doAnimationWindow()
{
    static bool isCache = false;
    static ImGuiFileDialog fileDialog;
    static std::string cache_loc = "../.cache/";
    static std::string cache_name = "result";
    cache_name.reserve(1000);
    static int cache_frame = 0;
    static bool clear_cache = false;

    static bool checkDone = false;
    static bool acknowledgeFailedStep = false;

    if (currentRenderer == NONE)
        return;

    if (ImGui::Begin("Animation"))
    {
        if (!isUpdating && !isAnimating && !checkDone && !acknowledgeFailedStep)
        {
            if (ImGui::TreeNode("Cache Results"))
            {
                if (clear_cache)
                {
                    if ( clearCache(cache_loc) )
                        clear_cache = false;
                    ImGui::End();
                    return;
                }
                ImGui::Checkbox("Cache Results?", &isCache);
                if(isCache)
                {
                    if (ImGui::Button("Cache Location"))
                        fileDialog.OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, cache_loc);
                    ImGui::TextUnformatted(cache_loc.c_str());
                    if (ImGui::InputText("Cache Name", cache_name.data(), 1000) || isResetting)
                        cache_frame = 0;
                    if (ImGui::Button("Clear Cache"))
                        clear_cache = true;
                }
                ImGui::TreePop();
            }
        }

        doAnimationMenu(checkDone, acknowledgeFailedStep);

        if (isCache && (isAnimating || checkDone) && !isCalcFrame)
            cacheFrame(cache_frame, cache_loc, cache_name);
    }
    ImGui::End();

    // display
    if (!isUpdating && !isAnimating && !nextFrame && fileDialog.Display("ChooseDirDlgKey")) 
    {
        // action if OK
        if (fileDialog.IsOk())
        {
            cache_loc = fileDialog.GetCurrentPath() + "/";
        }
        
        // close
        fileDialog.Close();
    }
}

#endif