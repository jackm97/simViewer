#include "animation_menu.h"
#include "../global_vars.h"

#include <string>
#include <vector>
#include <cmath>

#include <Eigen/Eigen>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb/stb_image_write.h>
#include <openvdb/openvdb.h>

#include <filesystem>

bool isCache = false;
std::string cache_loc = "../.cache/";
std::string cache_name = "result";
int cache_frame = 0;
bool clear_cache = false;

bool cache_last_frame = false;

void openVDBSave() {
    openvdb::initialize();
    openvdb::FloatGrid::Ptr density_grid = openvdb::FloatGrid::create();
    density_grid->setName("density");
    openvdb::Vec3fGrid::Ptr color_grid = openvdb::Vec3fGrid::create();
    color_grid->setName("color");

    Eigen::Vector3f color;

    // populate grids
    for (int i = 0; i < grid_size; i++)
        for (int j = 0; j < grid_size; j++)
            for (int k = 0; k < grid_size; k++) {
                color(0) = img[grid_size * grid_size * 3 * k + grid_size * 3 * j + 3 * i + 0];
                color(1) = img[grid_size * grid_size * 3 * k + grid_size * 3 * j + 3 * i + 1];
                color(2) = img[grid_size * grid_size * 3 * k + grid_size * 3 * j + 3 * i + 2];

                float density = color.norm();

                if (density == 0)
                    continue;

                color.normalize();

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
            openvdb::math::Transform::createLinearTransform(/*voxel size=*/grid_length / grid_size));
    density_grid->setTransform(
            openvdb::math::Transform::createLinearTransform(/*voxel size=*/grid_length / grid_size));

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

void cacheFrame() {

    if (!isCache || !(is_animating || cache_last_frame) || is_calc_frame)
        return;

    Eigen::VectorX<unsigned char> img8bit;
    std::string full_file_path = cache_loc + cache_name + "." + std::to_string(cache_frame) + ".png";
    int flipBoolTmp = stbi__flip_vertically_on_write;
    switch (current_renderer) {
        case Dim2:
            img8bit.resize(3 * grid_size * grid_size);
            for (int i = 0; i < 3 * grid_size * grid_size; i++) {
                float color_byte = img[i] * 255;
                color_byte = (color_byte > 255) ? 255 : color_byte;
                img8bit[i] = (unsigned char) color_byte;
            }
            stbi_flip_vertically_on_write(1);
            stbi_write_png(full_file_path.c_str(), grid_size, grid_size, 3, img8bit.data(), 3 * grid_size);
            stbi_flip_vertically_on_write(flipBoolTmp);
            break;

        case Dim3:
            openVDBSave();
            break;

        case None:
            break;
    }

    cache_frame += 1;

    cache_last_frame = false;
}

bool clearCache() {

    namespace fs = std::filesystem;

    ImGui::TextUnformatted("Clear Cache?");
    if (ImGui::Button("OK")) {
        std::vector<std::string> files;
        for (auto &p: fs::directory_iterator(cache_loc.c_str()))
            files.push_back(p.path().u8string());

        for (auto & file : files) {
            const char *cmpr_str1 = cache_name.c_str();
            char *cmpr_str2 = new char[cache_name.length()];
            int current_fname_len = file.length() - cache_loc.length();

            int i;
            for (i = 0; i < cache_name.size() && i < current_fname_len; i++)
                cmpr_str2[i] = file[cache_loc.length() + i];
            cmpr_str2[i] = '\0';

            if (std::strcmp(cmpr_str1, cmpr_str2) == 0)
                fs::remove(file);

            delete[] cmpr_str2;
        }

        return true;
    }
    if (ImGui::Button("Cancel")) {
        return true;
    }
    return false;
}

void doAnimationMenu() {

    // terminate animation if step failed
    if (failed_step) {
        cache_last_frame = false;
        ImGui::Text("Failed frame!");
        if (ImGui::Button("OK")) {
            failed_step = false;
            is_resetting = true;
        }
        return;
    }

    if (is_updating) {
        ImGui::Text("Updating...");
        return;
    } else if (cache_last_frame && is_calc_frame) {
        ImGui::Text("Finishing frame...");
        return;
    } else if (cache_last_frame && !next_frame && !is_animating) {
        cache_last_frame = false;
    }


    if (!is_animating && ImGui::Button("Animate")) is_animating = true;

    else if (is_animating && (cache_last_frame = ImGui::Button("Stop"))) is_animating = false;

    if (!cache_last_frame && !is_animating && (cache_last_frame = ImGui::Button("Next Frame"))) next_frame = true;

    if (!cache_last_frame && !is_animating && ImGui::Button("Render Frame Again")) re_render = true;

    if (!cache_last_frame && (cache_last_frame = ImGui::Button("Reset"))) is_resetting = true;

    ImGui::TextUnformatted(("FPS: " + std::to_string(std::round(sim_fps * 1000) / 1000.)).c_str());

    if (next_frame)
        old_sim_time = glfwGetTime();
}

void doCacheMenu(){
    if (!is_updating && !is_animating && !cache_last_frame && !failed_step) {
        if (ImGui::TreeNode("Cache Results")) {
            if (clear_cache) {
                if (clearCache()) {
                    clear_cache = false;
                    cache_frame = 0;
                }
                ImGui::TreePop();
                ImGui::End();
                return;
            }
            ImGui::Checkbox("Cache Results?", &isCache);
            if (isCache) {
                if (ImGui::Button("Cache Location"))
                    fileDialog.OpenDialog("ChooseDirDlgKey", "Choose a Directory", nullptr, cache_loc);
                ImGui::TextUnformatted(cache_loc.c_str());
                if (ImGui::InputText("Cache Name", cache_name.data(), 1000) || is_resetting){
                    cache_name = std::string(cache_name.data());
                    cache_frame = 0;
                }
                if (ImGui::Button("Clear Cache"))
                    clear_cache = true;
            }
            ImGui::TreePop();
        }
    }
}

void doAnimationWindow() {

    if (current_renderer == None)
        return;

    if (ImGui::Begin("Animation")) {
        doCacheMenu();
        doAnimationMenu();
    }
    ImGui::End();

    if (is_resetting)
        cache_frame = 0;

    // display
    if (!is_updating && !is_animating && !next_frame && fileDialog.Display("ChooseDirDlgKey")) {
        // action if OK
        if (fileDialog.IsOk()) {
            cache_loc = fileDialog.GetCurrentPath() + "/";
        }

        // close
        fileDialog.Close();
    }
}