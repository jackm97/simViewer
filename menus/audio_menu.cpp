#include "audio_menu.h"
#include "../global_vars.h"
#include <AudioFile/AudioFile.h>

#include <cmath>
#include <string>

void doAudioMenu()
{
    static AudioFile<float> audio_file;
    static AudioFile<float>::AudioBuffer buffer(1);

    static AudioFile<float> audio_file_play;

    static float sound_amp = 1.f;
    static bool do_sound = false;
    static bool save_sound = false;

    static float signal_amp = 50.f;

    static bool is_saving = false;

    if (currentSolver != LBM)
        return;

    if ( ImGui::Begin("Audio") )
    {
        if (ImGui::Checkbox("Generate Sound?", &do_sound))
            audio_file_play.load("/home/jack/Downloads/[ONTIVA.COM]-Rick Astley - Never Gonna Give You Up (Video)-HQ.wav");
        if ( do_sound )
            ImGui::InputFloat("Sound Amplitude", &sound_amp);

        
        ImGui::Checkbox("Save Sound?", &save_sound);

        if (save_sound)
        {
            ImGui::InputFloat("Signal Amplification", &signal_amp);
            if (ImGui::Button("Finish Sound"))
            {
                for (float & s : audio_file.samples[0])
                    s *= signal_amp;
                audio_file.setAudioBuffer( buffer );
                audio_file.setBitDepth(24);
                audio_file.setSampleRate(1 / lbm_solver->TimeStep() );
                audio_file.save("./test.wav");
                printf("%i\n", audio_file.getSampleRate());
                printf("%zu\n", audio_file.samples[0].size());
                for (float & s : audio_file.samples[0])
                    s /= signal_amp;
            }
        }
    }
    std::string sim_time = "Current Simulation Time(s): ";
    sim_time += std::to_string(lbm_solver->Time());
    ImGui::TextUnformatted(sim_time.c_str());
    ImGui::End();

    float Hz = 120;
    float w = 2 * M_PI * Hz;
    if (do_sound && (isAnimating || nextFrame) && !isCalcFrame )
    {
        int idx = .5 * L / lbm_solver->DeltaX();
        int range = .02 * N;
        // float ux = sound_amp * ( std::sin(w * LBMSolver->Time()) );
        int n_sample = audio_file_play.getSampleRate() * lbm_solver->Time();
        if (n_sample < audio_file_play.samples[0].size())
        {
            float sample = audio_file_play.samples[0][n_sample];
            float ux = sound_amp * sample;
            lbm_solver->ForceVelocity(idx, idx, ux, 0);
            for (int i = 1; i < range; i++)
            {
                lbm_solver->ForceVelocity(idx, idx + i, ux, 0);
                lbm_solver->ForceVelocity(idx, idx - i, ux, 0);
            }
        }
    }

    if ( save_sound && (isAnimating || nextFrame) )
        is_saving = true;

    static int current_sample_count = 0;
    float sample_rate = 48000;
    if ( is_saving && !isCalcFrame && iter == iter_per_frame)
    {
        if ( true)
        {
            float* rho_field = lbm_solver->RhoData();
            int idx = .8 * L / lbm_solver->DeltaX();
            float rho = rho_field[N*0 + idx];
            float sample = (rho - 1.3);
            
            // while (sample_rate * LBMSolver->Time() > current_sample_count)
            // {

            //     current_sample_count += 1;
            // }
                buffer[0].push_back(sample);
        }

        is_saving = false;
    }

    if (isResetting)
    {
        buffer[0].clear();
        current_sample_count = 0;
    }
}