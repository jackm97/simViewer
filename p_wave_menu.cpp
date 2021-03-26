#include "p_wave_menu.h"
#include "global_vars.h"

struct PressureWave
{
    float pos[3]{0.f}; // center position
    
    float u[3]{0.f}; // center speed

    float u_imp = 0; // peak fluid speed

    float radius = 0; // pressure wave

    float t_start = 0; // start time

    bool skadoosh = false;
};

std::vector<PressureWave> p_waves;
void doPressureWave(PressureWave p_wave);

void doPressureWindow()
{
    static std::vector<std::string> pList = {""};
    static int current_p_wave = 0;

    if (currentSolver != LBM)
        return;

    if (ImGui::Begin("Pressure Waves"))
    {
        if (ImGui::BeginCombo("Pressure Wave List", pList[current_p_wave].c_str()))
        {
            for (int n=0; n<pList.size(); n++)
            {
                const bool is_selected = (current_p_wave == n);
                if (ImGui::Selectable(pList[n].c_str(), is_selected))
                    current_p_wave = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (current_p_wave != 0)
        {

            if (currentRenderer == DIM2)
            {                
                if (ImGui::InputFloat2("Position", p_waves[current_p_wave - 1].pos));
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Position", p_waves[current_p_wave - 1].pos));
            }
            if (currentRenderer == DIM2)
            {
                if (ImGui::InputFloat2("Center Velocity", p_waves[current_p_wave - 1].u));
            }
            else if (currentRenderer == DIM3)
            {
                if (ImGui::InputFloat3("Center Velocity", p_waves[current_p_wave - 1].u));
            }
            if (ImGui::InputFloat("Impulse Speed", &(p_waves[current_p_wave - 1].u_imp)));
            if (ImGui::InputFloat("Radius", &(p_waves[current_p_wave - 1].radius)));
            if (ImGui::InputFloat("Start Time", &(p_waves[current_p_wave - 1].t_start)));
            if (ImGui::Checkbox("Skadoosh!", &(p_waves[current_p_wave - 1].skadoosh)));
        }
        if (ImGui::Button("Add Pressure Wave"))
        {
            pList.push_back("p_wave" + std::to_string(pList.size()));
            p_waves.push_back(PressureWave());
            current_p_wave = pList.size()-1;
        }
        if (ImGui::Button("Delete Pressure Wave") && current_p_wave!=0)
        {
            pList.erase(pList.end());
            p_waves.erase(p_waves.begin() + current_p_wave - 1);
            current_p_wave-=1;
        }
    }
    ImGui::End();

    if (currentSolver == LBM)
        for (int p = 0; p < p_waves.size(); p++) doPressureWave(p_waves[p]);
}

void doPressureWave(PressureWave p_wave)
{

    typedef Eigen::VectorXf Vector;

    float T = LBMSolver->Time();
    float dx = LBMSolver->DeltaX();

    int i = (p_wave.pos[0] + p_wave.u[0] * (T - p_wave.t_start)) / dx;
    int j = (p_wave.pos[1] + p_wave.u[1] * (T - p_wave.t_start)) / dx;

    float r_real = p_wave.radius;
    float u_imp = p_wave.u_imp;

    float t_start = p_wave.t_start;

    float w = u_imp / r_real;
    float Hz = w / (2 * M_PI);
    float period = 1/Hz;
    u_imp *= r_real * std::cos(w * (T-t_start));

    if ( (T-t_start) > period/2 )
        return;

    int r = (r_real * std::sin(w * (T-t_start)))/dx;
    
    int x = r, y = 0; 

    Eigen::VectorXi indices(2);
    indices(0) = i + x;
    indices(1) = j + y;
    
    Vector dir(2);
    dir(0) = (float) indices(0) - i;
    dir(1) = (float) indices(1) - j;
    dir.normalize();

    float ux = dir(0) * u_imp + p_wave.u[0];
    float uy = dir(1) * u_imp + p_wave.u[1];

    bool speed_check;
    bool idx_check;

    idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
    speed_check = (ux * dir(0) + uy * dir(1)) > 0;
    if ( (idx_check && speed_check) || p_wave.skadoosh )
        LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);
      
    // When radius is zero only a single 
    // point will be printed 
    if (r > 0) 
    { 
        // 1
        indices(0) = i + x;
        indices(1) = j - y;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

        // 2
        indices(0) = i + y;
        indices(1) = j + x;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

        // 3
        indices(0) = i - y;
        indices(1) = j + x;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);
    } 
      
    // Initialising the value of P 
    int P = 1 - r; 
    while (x > y) 
    {  
        y++; 
          
        // Mid-point is inside or on the perimeter 
        if (P <= 0) 
            P = P + 2*y + 1; 
              
        // Mid-point is outside the perimeter 
        else
        { 
            x--; 
            P = P + 2*y - 2*x + 1; 
        } 
          
        // All the perimeter points have already been printed 
        if (x < y) 
            break; 
          
        // Printing the generated point and its reflection 
        // in the other octants after translation 
        // 1 
        indices(0) = i + x;
        indices(1) = j + y;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();
        
        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

        // 2 
        indices(0) = i - x;
        indices(1) = j + y;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

        // 3 
        indices(0) = i + x;
        indices(1) = j - y;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

        // 4 
        indices(0) = i - x;
        indices(1) = j - y;
        
        dir(0) = (float) indices(0) - i;
        dir(1) = (float) indices(1) - j;
        dir.normalize();

        ux = dir(0) * u_imp + p_wave.u[0];
        uy = dir(1) * u_imp + p_wave.u[1];

        idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
        speed_check = (ux * dir(0) + uy * dir(1)) > 0;
        if ( (idx_check && speed_check) || p_wave.skadoosh )
            LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);
          
        // If the generated point is on the line x = y then  
        // the perimeter points have already been printed 
        if (x != y) 
        { 
            // 1 
            indices(0) = i + y;
            indices(1) = j + x;
            
            dir(0) = (float) indices(0) - i;
            dir(1) = (float) indices(1) - j;
            dir.normalize();

            ux = dir(0) * u_imp + p_wave.u[0];
            uy = dir(1) * u_imp + p_wave.u[1];

            idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
            speed_check = (ux * dir(0) + uy * dir(1)) > 0;
            if ( (idx_check && speed_check) || p_wave.skadoosh )
                LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

            // 2 
            indices(0) = i - y;
            indices(1) = j + x;
            
            dir(0) = (float) indices(0) - i;
            dir(1) = (float) indices(1) - j;
            dir.normalize();

            ux = dir(0) * u_imp + p_wave.u[0];
            uy = dir(1) * u_imp + p_wave.u[1];

            idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
            speed_check = (ux * dir(0) + uy * dir(1)) > 0;
            if ( (idx_check && speed_check) || p_wave.skadoosh )
                LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

            // 3 
            indices(0) = i + y;
            indices(1) = j - x;
            
            dir(0) = (float) indices(0) - i;
            dir(1) = (float) indices(1) - j;
            dir.normalize();

            ux = dir(0) * u_imp + p_wave.u[0];
            uy = dir(1) * u_imp + p_wave.u[1];

            idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
            speed_check = (ux * dir(0) + uy * dir(1)) > 0;
            if ( (idx_check && speed_check) || p_wave.skadoosh )
                LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);

            // 4 
            indices(0) = i - y;
            indices(1) = j - x;
            
            dir(0) = (float) indices(0) - i;
            dir(1) = (float) indices(1) - j;
            dir.normalize();

            ux = dir(0) * u_imp + p_wave.u[0];
            uy = dir(1) * u_imp + p_wave.u[1];

            idx_check = !(indices(0) > N-1 || indices(0) < 0 || indices(1) > N-1 || indices(1) < 0);
            speed_check = (ux * dir(0) + uy * dir(1)) > 0;
            if ( (idx_check && speed_check) || p_wave.skadoosh )
                LBMSolver->forceVelocity(indices(0),indices(1), ux, uy);
        } 
    } 
}