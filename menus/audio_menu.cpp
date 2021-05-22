#include "audio_menu.h"

#define _USE_MATH_DEFINES 
#include <cmath>

#include "../global_vars.h"
#include <AudioFile/AudioFile.h>

#include <string>

#include <filesystem>

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

AudioFile<float> audio_file_play;

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
                    audio_file.setSampleRate(1 / lbm_solver->TimeStep() / (float)iter_per_frame);
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
    std::string sim_time = "Current Simulation Time(s): ";
    sim_time += std::to_string(lbm_solver->Time());
    ImGui::TextUnformatted(sim_time.c_str());
    ImGui::End();

}

void updateAudio(){


    if (currentSolver != Lbm)
        return;

    float Hz = 1.f/10.f;
    float w = 2 * (float) M_PI * Hz;
    if (is_audio_loaded && do_sound && (is_animating || next_frame) && !is_calc_frame) {
        float y = .5 * grid_length + .4 * grid_length * std::sin(w * lbm_solver->Time());
        float x = .5 * grid_length - .4 * grid_length * std::abs(std::cos(w * lbm_solver->Time()));
        int idx_y = y / lbm_solver->DeltaX();
        int idx_x = x / lbm_solver->DeltaX();
        int range = .005 * grid_size;
        // float ux = sound_amp * ( std::sin(w * LBMSolver->Time()) );
        float n_sample = audio_file_play.getSampleRate() * lbm_solver->Time();
        if (n_sample < audio_file_play.samples[0].size() - 1) {
//            if (forces.size() == 0)
//                forces.push_back(jfs::Force());
//            forces[0].pos[0] = .5 * grid_length;
//            forces[0].pos[1] = .5 * grid_length;

            float sample_down = audio_file_play.samples[0][(int)n_sample];
            float sample_up = audio_file_play.samples[0][(int)n_sample + 1];
            float sample = (n_sample - std::floor(n_sample)) * sample_up + (std::ceil(n_sample) - n_sample) * sample_down;
            float ux = sound_amp * sample * std::abs(std::cos(w * lbm_solver->Time()));
            float uy = -sound_amp * sample * std::sin(w * lbm_solver->Time());
//            forces[0].force[0] = ux;
//            lbm_solver->ForceVelocity(idx_x, idx_y, ux, uy);
            float rho_current = lbm_solver->IndexRhoData(idx_x, idx_y);
            float rho_amp = sound_amp * sample + lbm_solver->Rho0();
            lbm_solver->AddMassSource(idx_x, idx_y, rho_amp - rho_current);
            float head_depth = .178;
            int min_head = (int) ((.5 * grid_length - head_depth / 2) / grid_length * (float) grid_size);
            int max_head = (int) ((.5 * grid_length + head_depth / 2) / grid_length * (float) grid_size);
            std::vector<int> i_vec, j_vec;
            std::vector<float> u_zero;
            for (int i = min_head; i <= max_head; i++) {
                for (int j = (int) (min_head + .1 * (max_head - min_head)); j <= .9 * max_head; j++) {
                    i_vec.push_back(i);
                    j_vec.push_back(j);
                    u_zero.push_back(0);
                }
            }
            lbm_solver->ForceVelocity(i_vec.data(), j_vec.data(), u_zero.data(), u_zero.data(), u_zero.size());
        }
    }

    if (is_audio_loaded && iter == (iter_per_frame - 1) && save_sound)
        is_saving = true;

    float sample_rate = 48000;
    if ( iter == iter_per_frame && !is_calc_frame && !failed_step)
    {
        if ( is_saving && audio_file_play.getSampleRate() * lbm_solver->Time() >= current_sample_count )
        {
            float head_width = .178; // m
            int idxl = (int) ( (.5 * grid_length - head_width/2) / grid_length * (float) grid_size );
            int idxr = (int) ( (.5 * grid_length + head_width/2) / grid_length * (float) grid_size );
            int idx = .5 * grid_length / lbm_solver->DeltaX();
//            float* rho_data = lbm_solver->RhoData();
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
//            for (int i = 0; i < grid_size*grid_size; i++)
//                mean_rho += rho_data[i];
//            mean_rho /= (grid_size*grid_size);
            float rho = lbm_solver->IndexRhoData(idx, idxl);
            rho_list_l.push_back(rho);
            float sample = rho - mean_rho_l;
            buffer[0].push_back(sample);
            rho = lbm_solver->IndexRhoData(idx, idxr);
            rho_list_r.push_back(rho);
            sample = rho - mean_rho_r;
            buffer[1].push_back(sample);

            // while (sample_rate * LBMSolver->Time() > current_sample_count)
            // {

            //     current_sample_count += 1;
            // }

            current_sample_count = audio_file_play.getSampleRate() * lbm_solver->Time() + 1;
        }

        is_saving = false;
    }

    if (is_resetting)
    {
        buffer[0].clear();
        buffer[1].clear();
        rho_list_l.clear();
        rho_list_r.clear();
        current_sample_count = 0;
    }
}