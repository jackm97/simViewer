#ifndef ANIMATIONMENU_H
#define ANIMATIONMENU_H

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
                color(0) = img(N*3*N*k + N*3*j + 3*i + 0);
                color(1) = img(N*3*N*k + N*3*j + 3*i + 1);
                color(2) = img(N*3*N*k + N*3*j + 3*i + 2);

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


    Eigen::VectorX<unsigned char> img8bit;
    std::string full_file_path = cache_loc + cache_name + "." + std::to_string(cache_frame) + ".png";
    int flipBoolTmp = stbi__flip_vertically_on_write;
    switch (currentRenderer)
    {
    case DIM2:
        img8bit = (img * 255).template cast <unsigned char>();
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

bool clearCache(std::string cache_loc)
{

    namespace fs = std::filesystem;

    ImGui::TextUnformatted("Clear Cache?");
    if (ImGui::Button("OK"))
    {
        #ifdef WIN32
        std::vector<std::wstring> files;
        #endif
        #ifdef UNIX
        std::vector<std::string> files;
        #endif
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