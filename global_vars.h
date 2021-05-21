#ifndef GLOBAL_VARS
#define GLOBAL_VARS

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <future>


extern GLFWwindow* menu_window;
extern GLFWwindow* render_window;

// keeps everything thread safe
extern bool is_calc_frame; // is true when thread is running to calculate frame
extern bool is_updating; // is true when thread is running to update solver or grid
extern std::atomic<bool> waiting_to_render;

// Grid Variables
// NOTE: Only square grids supported right now
extern unsigned int grid_size; // grid size
extern float grid_length; // grid length

// Rendering
extern float max_fps; // max sim_fps, if 0, uncapped
extern const float screen_refresh_rate; // to limit load on GPU
extern float old_sim_time;
extern float old_imgui_time;
extern float old_refresh_time;
extern float current_time;

// fps counter
extern float sim_fps;
extern int iter_per_frame;
extern int iter;

// sim results
extern float* img;

// Animation Flags
extern bool is_animating;
extern bool next_frame;
extern bool re_render;
extern bool is_resetting;
extern bool failed_step;

// Renderers
#include <glr/sceneviewer2d.h>
extern glr::sceneViewer2D renderer_2d;
#include <glr/sceneviewer.h>
extern glr::sceneViewer renderer_3d;

enum RenderType : unsigned int{
    None,
    Dim2,
    Dim3
};

extern RenderType current_renderer;
extern bool update_renderer;
extern bool render_enabled;

// Solver Stuff
#include <jfs/JSSFSolver.h>
#include <jfs/JSSFSolver3D.h>
#include <jfs/LBMSolver.h>
#include <jfs/cuda/lbm_solver_cuda.h>

enum SolverType : unsigned int{
    Empty = 0,
    Jssf = 1,
    JssfIter = 2,
    Lbm = 3,
    Jssf3D = 4
};

extern const int numSolvers;
extern const char* solverNames[];
extern SolverType currentSolver;
extern bool updateSolver;

// fluid solvers
//      2D
extern jfs::JSSFSolver<>* jssf_solver; //(1,grid_length,jfs::ZERO,dt);
extern jfs::JSSFSolver<jfs::iterativeSolver>* jssf_solver_iter; //(1,grid_length,jfs::ZERO,dt);
extern jfs::CudaLBMSolver* lbm_solver; //(1,grid_length,1/dt);
//      3D
extern jfs::JSSFSolver3D<jfs::iterativeSolver>* jssf_solver_3d; //(1,grid_length,jfs::ZERO,dt);

// fluid visualization
extern bool view_density;
extern float smoke_diss;
#include <jfs/visualization/grid_smoke2d.h>
extern jfs::gridSmoke2D* grid_smoke2d;
#include <jfs/visualization/grid_smoke3d.h>
extern jfs::gridSmoke3D* grid_smoke3d;


// Sources, Forces and Points
extern std::vector<jfs::Force> forces;
extern std::vector<jfs::Source> sources;

#endif