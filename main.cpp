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

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* menuWindow;
GLFWwindow* renderWindow;

// keeps everything thread safe
bool isCalcFrame = false; // is true when thread is running to calculate frame
bool isUpdating = false; // is true when thread is running to update solver or grid

// Grid Variables
// NOTE: Only square grids supported right now
unsigned int N=64; // grid size
float L=1.; // grid length

// Rendering
float max_fps = 60; // max sim_fps, if 0, uncapped
const float screen_refresh_rate = 240; // to limit load on GPU
float oldSimTime = 0;
float oldImGuiTime = 0;
float oldRefreshTime = 0;
float currentTime = 0;

// fps counter
float sim_fps;

// sim results
float* img;

// Animation Flags
bool isAnimating = false;
bool nextFrame = false;
bool reRender = false;
bool isResetting = false;
bool failedStep = false;

// Renderers
glr::sceneViewer2D renderer2D;
glr::sceneViewer renderer3D;

RENDER_TYPE currentRenderer = NONE;
bool updateRenderer = false;

// Solver Stuff
const int numSolvers = 5;
const char* solverNames[numSolvers] = {"", "JSSF", "JSSF Iterative", "Lattice Boltzmann", "JSSF3D"};
SOLVER_TYPE currentSolver = EMPTY;
bool updateSolver = false;

// fluid solvers
//      2D
jfs::JSSFSolver<>* JSSFSolver; //(1,L,jfs::ZERO,dt);
jfs::JSSFSolver<jfs::iterativeSolver>* JSSFSolverIter; //(1,L,jfs::ZERO,dt);
jfs::LBMSolver* LBMSolver; //(1,L,1/dt);
//      3D
jfs::JSSFSolver3D<jfs::iterativeSolver>* JSSFSolver3D; //(1,L,jfs::ZERO,dt);

// fluid visualization
bool view_density;
float smoke_diss = 0;
jfs::gridSmoke2D* grid_smoke2d = NULL;
jfs::gridSmoke3D* grid_smoke3d = NULL;


// Sources, Forces and Points
std::vector<jfs::Force> forces;
std::vector<jfs::Source> sources;

int main(int, char**) {
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
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    menuWindow = glfwCreateWindow(1280, 720, "simViewer!", NULL, NULL);
    if (menuWindow == NULL)
        return 1;
    glfwMakeContextCurrent(menuWindow);

    renderWindow = glfwCreateWindow(480, 480, "Output Window", NULL, NULL);
    if (renderWindow == NULL)
        return 1;

    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(menuWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // High-DPI
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5);
    io.Fonts->AddFontFromFileTTF("../fonts/Cousine-Regular.ttf", 18.0f, NULL, NULL);


    // Initialize GLR library
    glr::initialize();

    // Main loop
    while (!(glfwWindowShouldClose(menuWindow) || glfwWindowShouldClose(renderWindow)) || isUpdating || isCalcFrame)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        currentTime = glfwGetTime();
            
        // UI Stuff
        glfwMakeContextCurrent(menuWindow);
        
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

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  

        if ( (currentTime - oldImGuiTime) > 1/screen_refresh_rate )
        {          
            oldImGuiTime = glfwGetTime();
            glfwSwapBuffers(menuWindow);
        }
        
        // Render Stuff
        glfwMakeContextCurrent(renderWindow);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
            
        // viewport stuff
        int viewPortSize;
        int display_w, display_h;
        glfwGetFramebufferSize(renderWindow, &display_w, &display_h);
        viewPortSize = (display_h < display_w) ? (display_h) : (display_w);
        glViewport((display_w-viewPortSize)/2, (display_h-viewPortSize)/2, viewPortSize, viewPortSize);
        
        renderSims();
        
        if (currentTime - oldRefreshTime > 1/screen_refresh_rate)
        {
            oldRefreshTime = glfwGetTime();
            glfwSwapBuffers(renderWindow);
        }
    }

    // Imgui Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLR Cleanup
    renderer2D.cleanup();
    glr::cleanup();

    glfwDestroyWindow(menuWindow);
    glfwDestroyWindow(renderWindow);
    glfwTerminate();

    releaseMem();

    return 0;
}
