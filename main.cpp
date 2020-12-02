#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* window;
GLFWwindow* renderWindow;

// Animation window
void doAnimationWindow();

// keeps everything thread safe
bool isCalcFrame = false; // is true when thread is running to calculate frame
bool isUpdating = false; // is true when thread is running to update fluid or grid

// Grid Variables
// NOTE: Only square grids supported right now
unsigned int N=64; // grid size
float L=1.; // grid length

// Time step
float dt=.033;

// fps counter
float fps = 0;

// Animation Flags
bool isAnimating = false;
bool nextFrame = false;
bool isResetting = false;

// Renderers
#include <glr/sceneViewer2D.h>
glr::sceneViewer2D renderer2D;

// Solver Stuff
#include <jfs/JSSFSolver.h>


typedef enum {
    EMPTY,
    JSSF = 1,
    JSSFIter = 2
} SOLVER_TYPE;

static const int numSolvers = 3;
static const char* solverNames[numSolvers] = {"", "JSSF", "JSSF Iterative"};
SOLVER_TYPE currentSolver = EMPTY;
bool updateSolver = false;

// fluid solvers
jfs::JSSFSolver<> JSSFSolver(N,L,jfs::ZERO,dt);
jfs::JSSFSolver<jfs::iterativeSolver> JSSFSolverIter(N,L,jfs::ZERO,dt);

#include "solverMenus.h"
#include "forcesMenus.h"
#include "sourcesMenus.h"
#include "renderFuncs.h"
#include "gridMenu.h"

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
    window = glfwCreateWindow(1280, 720, "simViewer!", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);

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
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // High-DPI
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5);
    io.Fonts->AddFontFromFileTTF("../extern/imgui/misc/fonts/Cousine-Regular.ttf", 18.0f, NULL, NULL);


    // Main loop
    while (!glfwWindowShouldClose(window) || isUpdating || isCalcFrame)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        doAnimationWindow();
        doMainWindow();
        doForceWindow();
        doSourceWindow();

        // Rendering
        glfwMakeContextCurrent(window);
        ImGui::Render();
        
        // viewport stuff
        int viewPortSize;
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        viewPortSize = (display_h < display_w) ? (display_h) : (display_w);
        glViewport((display_w-viewPortSize)/2, (display_h-viewPortSize)/2, viewPortSize, viewPortSize);
        
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        renderSims();
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#include <string>

void doAnimationWindow()
{
    static bool checkDone = false;
    if (ImGui::Begin("Animation"))
    {
        
        if (isUpdating)
        {
            ImGui::Text("Updating...");
            ImGui::End();
            return;
        }
        else if (checkDone && isCalcFrame)
        {
            ImGui::Text("Finishing frame...");
            ImGui::End();
            return;
        }
        else
        {
            checkDone = false;
        }
        

        if (!isAnimating && ImGui::Button("Animate")) isAnimating=true;
        
        else if (isAnimating && (checkDone = ImGui::Button("Stop"))) isAnimating=false;

        if (!checkDone && !isAnimating && (checkDone = ImGui::Button("Next Frame"))) nextFrame=true;
        
        if (!checkDone && (checkDone = ImGui::Button("Reset"))) isResetting = true;

        ImGui::TextUnformatted(("FPS: " + std::to_string(std::round(fps*1000)/1000.)).c_str());
    }
    ImGui::End();
}
