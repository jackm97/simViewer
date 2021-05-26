#include "audio_menu.h"

#define _USE_MATH_DEFINES 
#include <cmath>

#include "../global_vars.h"
#include <AudioFile/AudioFile.h>

#include <string>

std::string audio_filename = "";
bool choosing_audio_file = false;
bool is_audio_loaded = false;
std::string audio_save_location = "";
bool choosing_save_location = false;
bool is_save_location_valid = false;

AudioFile<float> audio_file;
AudioFile<float>::AudioBuffer buffer(2);
std::vector<float> rho_list_l;
std::vector<float> rho_list_r;
float head_damp = 1.f;

AudioFile<float> audio_file_play;
int n_sample = 0;

float sound_amp = 1.f;
bool do_sound = false;
bool save_sound = false;

float signal_amp = .9f;

bool is_saving = false;

int current_sample_count = 0;

void doAudioMenu()
{

    if (currentSolver != Lbm)
        return;

    if (!audioFileDialog.IsOpened())
    {
        choosing_audio_file = false;
        choosing_save_location = false;
    }

    // display
    if (!is_updating && !is_animating && !next_frame && audioFileDialog.Display("ChooseFileDlgKey")) {
        // action if OK
        if (audioFileDialog.IsOk()) {
            if (choosing_audio_file) {
                audio_filename = audioFileDialog.GetFilePathName();
                is_audio_loaded = true;
                audio_file_play.load(audio_filename);
                float min_signal = audio_file_play.samples[0][0];
                float max_signal = audio_file_play.samples[0][0];
                float mean_signal = 0;
                for (float& s : audio_file_play.samples[0])
                {
                    if (min_signal > s)
                        min_signal = s;
                    if (max_signal < s)
                        max_signal = s;
                    mean_signal += s;
                }
                mean_signal /= audio_file_play.samples[0].size();

                for (float& s : audio_file_play.samples[0])
                {
                    s = (s - mean_signal) / (max_signal - min_signal);
                    s *= signal_amp;
                }
            }
            if (choosing_save_location) {
                audio_save_location = audioFileDialog.GetFilePathName();
                is_save_location_valid = true;
            }
        }

        // close
        audioFileDialog.Close();
    }


    if ( ImGui::Begin("Audio") && !choosing_audio_file && !choosing_save_location)
    {
        ImGui::Checkbox("Generate Sound?", &do_sound);
        if (do_sound) {
            if (ImGui::Button("Audio File")) {
                audioFileDialog.OpenDialog("ChooseFileDlgKey", "Choose a File", ".wav", ".");
                choosing_audio_file = true;
            }
            ImGui::TextUnformatted(audio_filename.c_str());
            if (is_audio_loaded)
                ImGui::InputFloat("Sound Amplitude", &sound_amp);
        }

        if (do_sound && is_audio_loaded)
            ImGui::InputFloat("Head Damp", &head_damp);


        ImGui::Checkbox("Save Sound?", &save_sound);

        if (save_sound && is_audio_loaded)
        {
            if (ImGui::Button("Save Location")) {
                audioFileDialog.OpenDialog("ChooseFileDlgKey", "Choose a File", ".wav", ".");
                choosing_save_location = true;
            }
            ImGui::TextUnformatted(audio_save_location.c_str());
            if (is_save_location_valid) {
                ImGui::InputFloat("Signal Amplification", &signal_amp);
                if (ImGui::Button("Finish Sound"))
                {
                    float min_signal = buffer[0][0];
                    float max_signal = buffer[0][0];
                    for (float& s : buffer[0]) {
                        if (min_signal > s)
                            min_signal = s;
                        if (max_signal < s)
                            max_signal = s;
                    }
                    for (float& s : buffer[1]) {
                        if (min_signal > s)
                            min_signal = s;
                        if (max_signal < s)
                            max_signal = s;
                    }
                    for (float& s : buffer[0]) {
                        s = 2 * (s - min_signal) / (max_signal - min_signal) - 1;
                        s *= signal_amp;
                    }
                    for (float& s : buffer[1]) {
                        s = 2 * (s - min_signal) / (max_signal - min_signal) - 1;
                        s *= signal_amp;
                    }
                    audio_file.setAudioBuffer(buffer);
                    audio_file.setBitDepth(24);
                    audio_file.setSampleRate(audio_file_play.getSampleRate());
                    audio_file.save(audio_save_location);
                    for (float& s : buffer[0]) {
                        s /= signal_amp;
                        s += 1;
                        s = s / 2 * (max_signal - min_signal) + min_signal;
                    }
                    for (float& s : buffer[1]) {
                        s /= signal_amp;
                        s += 1;
                        s = s / 2 * (max_signal - min_signal) + min_signal;
                    }
                }
            }
        }
    }
    std::string sim_time = "Current Audio Time(s): ";
    sim_time += std::to_string((float)n_sample / (float) audio_file_play.getSampleRate());
    ImGui::TextUnformatted(sim_time.c_str());
    ImGui::End();

}

void updateAudio(){


    if (currentSolver != Lbm)
        return;

    float Hz = 1.f/10.f;
    float w = 2 * (float) M_PI * Hz;
    std::vector<int> i_vec, j_vec;
    std::vector<float> rho0_vec;
    if (is_audio_loaded && do_sound && (is_animating || next_frame) && !is_calc_frame) {
        if (n_sample < audio_file_play.samples[0].size() - 1) {
            float t = (float)n_sample / (float)audio_file_play.getSampleRate();
            float phi = .2;

            float sample = audio_file_play.samples[0][n_sample];
            n_sample += 1;
            float rho_amp = sound_amp * sample + lbm_solver->Rho0();

            while (phi <= .8)
            {
                float y = .5 * grid_length + .2 * grid_length * std::sin(w * t);
                float x = .5 * grid_length - .2 * grid_length * std::abs(std::cos(w * t));
                int idx_y = y / lbm_solver->DeltaX();
                int idx_x = x / lbm_solver->DeltaX();
                if (i_vec.size() == 0 || (i_vec[i_vec.size() - 1] != idx_x || j_vec[j_vec.size() - 1] != idx_x))
                {
                    i_vec.push_back(idx_x);
                    j_vec.push_back(idx_y);
                    rho0_vec.push_back(rho_amp);
                phi += .8;
            }
            lbm_solver->ForceMass(i_vec.data(), j_vec.data(), rho0_vec.data(), rho0_vec.size());

            i_vec.clear();
            j_vec.clear();
            rho0_vec.clear();
            float head_depth = .178;
            int min_head = (int)((.5 * grid_length - head_depth / 2) / grid_length * (float)grid_size);
            int max_head = (int)((.5 * grid_length + head_depth / 2) / grid_length * (float)grid_size);
            for (int i = min_head; i <= max_head; i++) {
                for (int j = min_head; j <= max_head; j++) {
                    i_vec.push_back(i);
                    j_vec.push_back(j);
                    rho0_vec.push_back(lbm_solver->Rho0());
                }
            }
            lbm_solver->ForceMass(i_vec.data(), j_vec.data(), rho0_vec.data(), rho0_vec.size(), head_damp);
        }
    }

    if (is_audio_loaded && (is_animating || next_frame) && !is_calc_frame) {
    }

    if (is_audio_loaded && iter == (iter_per_frame - 1) && save_sound)
        is_saving = true;

    float sample_rate = 48000;
    if ( iter == iter_per_frame && !is_calc_frame && !failed_step)
    {
        if ( is_saving )
        {
            float head_width = .178; // m
            int idxl = (int) ( (.5 * grid_length - head_width/2) / grid_length * (float) grid_size ) - 1;
            int idxr = (int) ( (.5 * grid_length + head_width/2) / grid_length * (float) grid_size ) + 1;
            int x_head = .5 * grid_length / lbm_solver->DeltaX();
            float mean_rho_l;
            float mean_rho_r;
            int moving_mean_samples = 10;
            if (buffer[0].size() < moving_mean_samples) {
                mean_rho_l = lbm_solver->Rho0();
                mean_rho_r = lbm_solver->Rho0();
            } else{
                mean_rho_l = 0;
                mean_rho_r = 0;
                for (int i = 1; i < moving_mean_samples+1; i++){
                    mean_rho_l += rho_list_l[rho_list_l.size() - i]/moving_mean_samples;
                    mean_rho_r += rho_list_r[rho_list_r.size() - i]/moving_mean_samples;
                }
            }
            float rho = lbm_solver->IndexRhoData(x_head, idxl);
            rho_list_l.push_back(rho);
            float sample = rho - mean_rho_l;
            buffer[0].push_back(sample);
            rho = lbm_solver->IndexRhoData(x_head, idxr);
            rho_list_r.push_back(rho);
            sample = rho - mean_rho_r;
            buffer[1].push_back(sample);
        }

        is_saving = false;
    }

    if (is_resetting)
    {
        buffer[0].clear();
        buffer[1].clear();
        rho_list_l.clear();
        rho_list_r.clear();
        n_sample = 0;
    }
}