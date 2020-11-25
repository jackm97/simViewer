#ifndef RENDERFUNCS_H
#define RENDERFUNCS_H

#include <glr/sceneViewer2D.h>

#include "fluidMenus.h"
#include "forcesMenus.h"
#include "sourcesMenus.h"

#include <future>

// Animation Flags
bool isAnimating = false;
bool nextFrame = false;
bool isResetting = false;
    
glr::sceneViewer2D renderer2D;

bool JSSFRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolver.resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        JSSFSolver.calcNextStep(forces,sources);
        nextFrame = false;
        newImage = true;
    }
    if (newImage) JSSFSolver.getImage(img);
    return newImage;
}

bool JSSFIterRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolverIter.resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        JSSFSolverIter.calcNextStep(forces,sources);
        nextFrame = false;
        newImage = true;
    }
    if (newImage) JSSFSolverIter.getImage(img);
    return newImage;
}

void renderSims()
{
    static float currentTime = 0;
    static float oldTime = 0;
    static Eigen::VectorXf img;
    static std::future<bool> future; 

    static bool firstCall = true;
    if (firstCall)
    {
        renderer2D.init(L,L);
        renderer2D.addTexture(N,N,"background");
        img.resize(N*N*3);
        img.setZero();
        renderer2D.uploadPix2Tex("background", GL_RGB, GL_FLOAT, img.data());

        firstCall = false;
    }

    // here we start the async method
    // outside of the frame rate if statement
    // so that the overhead of starting 
    // async doesn't interfere with framerate
    // calculation
    if (!isUpdating)
    {
        switch (currentSolver)
        {
        case JSSF:
            if (!isCalcFrame) {
                future = std::async(std::launch::async, JSSFRender, (void*) &img);
                isCalcFrame = true;
            }
        case JSSFIter:
            if (!isCalcFrame) {
                future = std::async(std::launch::async, JSSFIterRender, (void*) &img);
                isCalcFrame = true;
            }
        }    
    }

    if (!isUpdating && (currentTime - oldTime > dt))
    {
        switch (currentSolver)
        {
        case EMPTY:
            fps = 0;
            break;
        case JSSF:
        case JSSFIter:
            if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                fps = 1/(currentTime - oldTime);
                oldTime = glfwGetTime();
                if (future.get()) renderer2D.uploadPix2Tex("background", GL_RGB, GL_FLOAT, img.data());
            }
            break;
        }    
    }

    if (currentSolver != EMPTY) renderer2D.drawScene();

    currentTime = glfwGetTime();
}

#endif