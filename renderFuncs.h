#ifndef RENDERFUNCS_H
#define RENDERFUNCS_H

#include <glr/sceneViewer2D.h>

#include <future>

void updateRenderer()
{
    switch (currentSolver)
    {
    
    case EMPTY:
        currentRenderer = NONE;
        break;

    // 2D
    case JSSF:
    case JSSFIter:
    case LBM:
        currentRenderer = DIM2;
        break;

    // 3D
    case JSSF3D:
        currentRenderer = DIM3;
        break;
    }
    
    
    switch (currentRenderer)
    {
    
    case EMPTY:
        break;

    // 2D
    case DIM2:
        Eigen::VectorXf img(N*N*3);
        img.setZero();
        renderer2D.deleteTexture("background");
        renderer2D.addTexture(N,N,"background");
        renderer2D.uploadPix2Tex("background", GL_RGB, GL_FLOAT, img.data());
        renderer2D.changeBounds(L, L);
        break;
    }
}

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
        if ( !(failedStep = JSSFSolver.calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
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
        if ( !(failedStep = JSSFSolverIter.calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    if (newImage) JSSFSolverIter.getImage(img);
    return newImage;
}

bool LBMRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        LBMSolver.resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = LBMSolver.calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    if (newImage) LBMSolver.getImage(img);
    return newImage;
}

bool JSSF3DRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolver3D.resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = JSSFSolver3D.calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    if (newImage) JSSFSolver3D.getImage(img);
    return newImage;
}

void renderSims()
{
    static float currentTime = 0;
    static float oldTime = 0;
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
            break;
        case JSSFIter:
            if (!isCalcFrame) {
                future = std::async(std::launch::async, JSSFIterRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        case LBM:
            if (!isCalcFrame) {
                future = std::async(std::launch::async, LBMRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        case JSSF3D:
            if (!isCalcFrame) {
                future = std::async(std::launch::async, JSSF3DRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        }    
    }

    if (!isUpdating && (currentTime - oldTime > dt))
    {
        switch (currentSolver)
        {
        case EMPTY:
            fps = 0;
            break;

        // 2D
        case JSSF:
        case JSSFIter:
        case LBM:
            if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                fps = 1/(currentTime - oldTime);
                oldTime = glfwGetTime();
                if (future.get()) renderer2D.uploadPix2Tex("background", GL_RGB, GL_FLOAT, img.data());
            }
            break;

        // 3D
        case JSSF3D:
            if ( (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                fps = 1/(currentTime - oldTime);
                oldTime = glfwGetTime();
                future.get();
            }
            break;
        }    
    }

    switch (currentRenderer)
    {
    case NONE:
        break;
    
    // 2D
    case DIM2:
        renderer2D.drawScene();
        break;
    }

    currentTime = glfwGetTime();
}

#endif