#include "render_funcs.h"
#include "global_vars.h"

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
        reRender = true;
        renderer2D.init();
        renderer2D.getTexture("background")->genNewTexture(N,N);
        renderer2D.setBounds(L, L);
        break;
    }
}

bool JSSFRender(void* imgPtr)
{
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolver->resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = JSSFSolver->calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    else if (reRender)
    {
        reRender = false;
        newImage = true;
    }
    if (newImage) { *(float**)imgPtr = JSSFSolver->imageData();}
    return newImage;
}

bool JSSFIterRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolverIter->resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = JSSFSolverIter->calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    else if (reRender)
    {
        reRender = false;
        newImage = true;
    }
    if (newImage) { *(float**)imgPtr = JSSFSolverIter->imageData();}
    return newImage;
}

bool LBMRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        LBMSolver->resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = LBMSolver->calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    else if (reRender)
    {
        reRender = false;
        newImage = true;
    }
    if (newImage) 
        if (!view_density) { *(float**)imgPtr = LBMSolver->imageData(); }
        else { *(float**)imgPtr = LBMSolver->mappedRhoData(); }
        
    return newImage;
}

bool JSSF3DRender(void* imgPtr)
{
    Eigen::VectorXf &img = *((Eigen::VectorXf*) imgPtr);
    bool newImage = false;
    if (isResetting)
    {
        JSSFSolver3D->resetFluid();
        isResetting = false;
        newImage = true;
    }
    else if (isAnimating || nextFrame)
    {
        if ( !(failedStep = JSSFSolver3D->calcNextStep(forces,sources)) )
        {
            nextFrame = false;
            newImage = true;
        }
    }
    else if (reRender)
    {
        reRender = false;
        newImage = true;
    }
    if (newImage) { *(float**)imgPtr = JSSFSolver3D->imageData();}
    return newImage;
}

bool renderSims()
{
    static std::future<bool> future; 

    // here we start the async method
    // outside of the frame rate if statement
    // so that the overhead of starting 
    // async doesn't interfere with framerate
    // calculation
    if (!isUpdating && !isCalcFrame)
    {
        switch (currentSolver)
        {
        case JSSF:
            if ((nextFrame || isAnimating || isResetting || reRender)) {
                future = std::async(std::launch::async, JSSFRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        case JSSFIter:
            if ((nextFrame || isAnimating || isResetting || reRender)) {
                future = std::async(std::launch::async, JSSFIterRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        case LBM:
            if ((nextFrame || isAnimating || isResetting || reRender)) {
                future = std::async(std::launch::async, LBMRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        case JSSF3D:
            if ((nextFrame || isAnimating || isResetting || reRender)) {
                future = std::async(std::launch::async, JSSF3DRender, (void*) &img);
                isCalcFrame = true;
            }
            break;
        }  
    }

    currentTime = glfwGetTime();

    if ( !isUpdating && (max_fps == 0 || currentTime - oldSimTime > 1/max_fps) )
    {        
        switch (currentSolver)
        {
        case EMPTY:
            if ( isCalcFrame && (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                future.get();
            }
            sim_fps = 0;
            break;

        // 2D
        case JSSF:
        case JSSFIter:
        case LBM:
            if ( isCalcFrame && (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                sim_fps = 1/(currentTime - oldSimTime);
                oldSimTime = glfwGetTime();  
                if (future.get()) 
                    renderer2D.getTexture("background")->loadPixels( GL_RGB, GL_FLOAT, img);
            }
            break;

        // 3D
        case JSSF3D:
            if ( isCalcFrame && (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) )
            {
                isCalcFrame = false;
                sim_fps = 1/(currentTime - oldSimTime);
                oldSimTime = glfwGetTime();  
                future.get();
            }
            break;
        }    
    }

    currentTime = glfwGetTime();

    bool frameRendered = false;
    switch (currentRenderer)
    {
    case NONE:
        if (currentTime - oldRefreshTime > 1/screen_refresh_rate)
        {
            frameRendered = true;
            oldRefreshTime = glfwGetTime();
        }
        break;
    
    // 2D
    case DIM2:
        if (currentTime - oldRefreshTime > 1/screen_refresh_rate)
        {
            renderer2D.drawScene();
            frameRendered = true;
            oldRefreshTime = glfwGetTime();
        }
        break;
    }

    return frameRendered;
}