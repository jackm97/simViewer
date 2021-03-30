#include "animation_menu.h"
#include "../global_vars.h"

#include <string>
#include <vector>
#include <cmath>

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <Eigen/Eigen>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <openvdb/openvdb.h>

#include <filesystem>

void openVDBSave(int& cache_frame, std::string cache_loc, std::string cache_name)
{
    openvdb::initialize();
    openvdb::FloatGrid::Ptr density_grid = openvdb::FloatGrid::create();
    density_grid->setName("density");
    openvdb::Vec3fGrid::Ptr color_grid = openvdb::Vec3fGrid::create();
    color_grid->setName("color");

    Eigen::Vector3f color;

    // populate grids
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
            {
                color(0) = img[N*N*3*k + N*3*j + 3*i + 0];
                color(1) = img[N*N*3*k + N*3*j + 3*i + 1];
                color(2) = img[N*N*3*k + N*3*j + 3*i + 2];

                float density = color.norm()/std::sqrt(3);

                if (density == 0)
                    continue;
                    
                color = (1/density) * color;

                openvdb::Vec3f gridColor;
                gridColor.x() = color(0);
                gridColor.y() = color(1);
                gridColor.z() = color(2);

                openvdb::Vec3fGrid::Accessor c_accessor = color_grid->getAccessor();
                openvdb::Coord xyz(i, j, k);
                c_accessor.setValue(xyz, gridColor);

                openvdb::FloatGrid::Accessor d_accessor = density_grid->getAccessor();
                d_accessor.setValue(xyz, density);                                               
            }

    // Associate a scaling transform with the grid that sets the voxel size
    // to 0.5 units in world space.
    color_grid->setTransform(
        openvdb::math::Transform::createLinearTransform(/*voxel size=*/L/N));
    density_grid->setTransform(
        openvdb::math::Transform::createLinearTransform(/*voxel size=*/L/N));

    // Create a VDB file object.
    openvdb::io::File file(cache_loc + cache_name + "." + std::to_string(cache_frame) + ".vdb");
    
    // Add the grid pointer to a container.
    openvdb::GridPtrVec grids;
    grids.push_back(density_grid);
    grids.push_back(color_grid);

    // Write out the contents of the container.
    file.write(grids);
    file.close();
}

void cacheFrame(int& cache_frame, std::string cache_loc, std::string cache_name)
{    
    switch (currentSolver)
    {
    case JSSF:
    case JSSFIter:
    case LBM:
        img = grid_smoke2d->smokeData();
        break;

    case JSSF3D:
        img = grid_smoke3d->smokeData();
        break;
    }


    Eigen::VectorX<unsigned char> img8bit;
    std::string full_file_path = cache_loc + cache_name + "." + std::to_string(cache_frame) + ".png";
    int flipBoolTmp = stbi__flip_vertically_on_write;
    switch (currentRenderer)
    {
    case DIM2:
        for (int i = 0; i < 3*N*N; i++)
            img8bit[i] = img[i] * 255;
        stbi_flip_vertically_on_write(1);
        stbi_write_png(full_file_path.c_str(), N, N, 3, img8bit.data(), 3*N);
        stbi_flip_vertically_on_write(flipBoolTmp);
        break;

    case DIM3:
        openVDBSave(cache_frame, cache_loc, cache_name);
        break;

    }

    cache_frame += 1;

}

bool clearCache(std::string cache_loc, std::string cache_name)
{

    namespace fs = std::filesystem;

    ImGui::TextUnformatted("Clear Cache?");
    if (ImGui::Button("OK"))
    {
        std::vector<std::string> files;
        for(auto& p: fs::directory_iterator(cache_loc.c_str()))
            files.push_back(p.path().u8string());

        for (int f = 0; f < files.size(); f++)
        {
            const char* cmpr_str1 = cache_name.c_str();
            char* cmpr_str2 = new char [cache_name.length()];
            int current_fname_len = files[f].length() - cache_loc.length();
            
            int i;
            for (i = 0; i < cache_name.size() && i < current_fname_len; i++)
                cmpr_str2[i] = files[f][cache_loc.length() + i];
            cmpr_str2[i] = '\0';

            if (std::strcmp(cmpr_str1, cmpr_str2) == 0)
                fs::remove(files[f]);

            delete [] cmpr_str2;
        }

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

    if (!checkDone && !isAnimating && ImGui::Button("Render Frame Again")) reRender = true;
    
    if (!checkDone && (checkDone = ImGui::Button("Reset"))) isResetting = true;

    ImGui::TextUnformatted(("FPS: " + std::to_string(std::round(sim_fps*1000)/1000.)).c_str());

}

void doAnimationWindow()
{
    static bool isCache = false;
    static ImGuiFileDialog fileDialog;
    static std::string cache_loc = "../.cache/";
    static char cache_name[1000] = "result";
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
                    if ( clearCache(cache_loc, cache_name) )
                    {
                        clear_cache = false;
                        cache_frame = 0;
                    }
                    ImGui::TreePop();
                    ImGui::End();
                    return;
                }
                ImGui::Checkbox("Cache Results?", &isCache);
                if(isCache)
                {
                    if (ImGui::Button("Cache Location"))
                        fileDialog.OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, cache_loc);
                    ImGui::TextUnformatted(cache_loc.c_str());
                    if (ImGui::InputText("Cache Name", cache_name, 1000) || isResetting)
                        cache_frame = 0;
                    if (ImGui::Button("Clear Cache"))
                        clear_cache = true;
                }
                ImGui::TreePop();
            }
        }

        doAnimationMenu(checkDone, acknowledgeFailedStep);

        if (isResetting)
            cache_frame = 0;

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