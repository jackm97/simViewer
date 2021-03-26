#ifndef GLOBAL_VARS
#define GLOBAL_VARS

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


extern GLFWwindow* menuWindow;
extern GLFWwindow* renderWindow;

// keeps everything thread safe
extern bool isCalcFrame; // is true when thread is running to calculate frame
extern bool isUpdating; // is true when thread is running to update solver or grid

// Grid Variables
// NOTE: Only square grids supported right now
extern unsigned int N; // grid size
extern float L; // grid length

// Time step
extern float dt;

// fps counter
extern float fps;

// sim results
#include <Eigen/Eigen>
extern Eigen::VectorXf img;

// Animation Flags
extern bool isAnimating;
extern bool nextFrame;
extern bool reRender;
extern bool isResetting;
extern bool failedStep;

// Renderers
#include <glr/sceneviewer2d.h>
extern glr::sceneViewer2D renderer2D;
#include <glr/sceneviewer.h>
extern glr::sceneViewer renderer3D;

enum RENDER_TYPE : unsigned int{
    NONE,
    DIM2,
    DIM3
};

extern RENDER_TYPE currentRenderer;

// Solver Stuff
#include <jfs/JSSFSolver.h>
#include <jfs/JSSFSolver3D.h>
#include <jfs/LBMSolver.h>

enum SOLVER_TYPE : unsigned int{
    EMPTY = 0,
    JSSF = 1,
    JSSFIter = 2,
    LBM = 3,
    JSSF3D = 4
};

extern const int numSolvers;
extern const char* solverNames[];
extern SOLVER_TYPE currentSolver;
extern bool updateSolver;

// fluid solvers
//      2D
extern jfs::JSSFSolver<>* JSSFSolver; //(1,L,jfs::ZERO,dt);
extern jfs::JSSFSolver<jfs::iterativeSolver>* JSSFSolverIter; //(1,L,jfs::ZERO,dt);
extern jfs::LBMSolver* LBMSolver; //(1,L,1/dt);
//      3D
extern jfs::JSSFSolver3D<jfs::iterativeSolver>* JSSFSolver3D; //(1,L,jfs::ZERO,dt);


// Sources, Forces and Points
extern std::vector<jfs::Force> forces;
extern std::vector<jfs::Source> sources;

#endif