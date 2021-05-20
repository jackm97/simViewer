#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "global_vars.h"

#include "render_funcs.h"
#include "./menus/animation_menu.h"
#include "./menus/solver_menus.h"
#include "./menus/forces_menu.h"
#include "./menus/sources_menu.h"
#include "./menus/p_wave_menu.h"
#include "./menus/grid_menu.h"
#include "./menus/audio_menu.h"

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow *menu_window;
GLFWwindow *render_window;

// keeps everything thread safe
bool is_calc_frame = false; // is true when thread is running to calculate frame
bool is_updating = false; // is true when thread is running to update solver or grid

// Grid Variables
// NOTE: Only square grids supported right now
unsigned int grid_size = 64; // grid size
float grid_length = 1.; // grid length

// Rendering
float max_fps = 60; // max sim_fps, if 0, uncapped
const float screen_refresh_rate = 144; // to limit load on GPU
float old_sim_time = 0;
float old_imgui_time = 0;
float old_refresh_time = 0;
float current_time = 0;

// fps counter
float sim_fps;
int iter_per_frame = 1;
int iter = 0;

// sim results
float *img;

// Animation Flags
bool is_animating = false;
bool next_frame = false;
bool re_render = false;
bool is_resetting = false;
bool failed_step = false;

// Renderers
glr::sceneViewer2D renderer_2d;
glr::sceneViewer renderer_3d;

RenderType current_renderer = None;
bool update_renderer = true;
bool render_enabled = true;

// Solver Stuff
const int numSolvers = 5;
const char *solverNames[numSolvers] = {"", "Jssf", "Jssf Iterative", "Lattice Boltzmann", "Jssf3D"};
SolverType currentSolver = Empty;
bool updateSolver = false;

// fluid solvers
//      2D
jfs::JSSFSolver<> *jssf_solver; //(1,grid_length,jfs::ZERO,dt);
jfs::JSSFSolver<jfs::iterativeSolver> *jssf_solver_iter; //(1,grid_length,jfs::ZERO,dt);
jfs::CudaLBMSolver *lbm_solver; //(1,grid_length,1/dt);
//      3D
jfs::JSSFSolver3D<jfs::iterativeSolver> *jssf_solver_3d; //(1,grid_length,jfs::ZERO,dt);

// fluid visualization
bool view_density;
float smoke_diss = 0;
jfs::gridSmoke2D *grid_smoke2d = nullptr;
jfs::gridSmoke3D *grid_smoke3d = nullptr;


// Sources, Forces and Points
std::vector<jfs::Force> forces;
std::vector<jfs::Source> sources;

int main(int, char **) {
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    menu_window = glfwCreateWindow(1280, 720, "simViewer!", nullptr, nullptr);
    if (menu_window == nullptr)
        return 1;
    glfwMakeContextCurrent(menu_window);

    render_window = glfwCreateWindow(480, 480, "Output Window", nullptr, nullptr);
    if (render_window == nullptr)
        return 1;

    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(menu_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // High-DPI
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5);
    io.Fonts->AddFontFromFileTTF("../fonts/Cousine-Regular.ttf", 18.0f, nullptr, nullptr);


    // Initialize GLR library
    glfwMakeContextCurrent(render_window);
    glr::initialize();

    // Main loop
    while (!(glfwWindowShouldClose(menu_window) || glfwWindowShouldClose(render_window)) || is_updating || is_calc_frame) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        current_time = glfwGetTime();

        // UI Stuff
        if ((current_time - old_imgui_time) > 1 / screen_refresh_rate) {
            glfwMakeContextCurrent(menu_window);


            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            doAnimationWindow();
            doMainWindow();
            doForceWindow();
            doSourceWindow();
            doPressureWindow();
            doAudioMenu();

            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            old_imgui_time = glfwGetTime();
            glfwSwapBuffers(menu_window);
        }

        // Things that need to be called on each loop for menu stuff
        cacheFrame(); // only caches frame if user activates caching
        updateAudio();

        // Render Stuff
        glfwMakeContextCurrent(render_window);
        static bool draw_and_swap = false;
        current_time = glfwGetTime();
        if (current_time - old_refresh_time > 1 / screen_refresh_rate) {

            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);

            // viewport stuff
            int viewPortSize;
            int display_w, display_h;
            glfwGetFramebufferSize(render_window, &display_w, &display_h);
            viewPortSize = (display_h < display_w) ? (display_h) : (display_w);
            glViewport((display_w - viewPortSize) / 2, (display_h - viewPortSize) / 2, viewPortSize, viewPortSize);

            draw_and_swap = true;
        }

        renderSims();

        if (draw_and_swap) {

            switch (current_renderer)
            {
                case None:
                    break;

                    // 2D
                case Dim2:
                    if (render_enabled)
                        renderer_2d.drawScene();
                    break;

                case Dim3:
                    break;
            }

            old_refresh_time = glfwGetTime();
            glfwSwapBuffers(render_window);

            draw_and_swap = false;
        }
    }

    // Imgui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLR Cleanup
    renderer_2d.cleanup();
    glr::cleanup();

    glfwDestroyWindow(menu_window);
    glfwDestroyWindow(render_window);
    glfwTerminate();

    releaseMem();

    return 0;
}
