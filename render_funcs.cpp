#include "render_funcs.h"
#include "global_vars.h"

#include <future>

void doRendererUpdate() {
    switch (currentSolver) {

        case (SolverType) Empty:
            current_renderer = None;
            break;

            // 2D
        case Jssf:
        case JssfIter:
        case Lbm:
            current_renderer = Dim2;
            break;

            // 3D
        case Jssf3D:
            current_renderer = Dim3;
            break;
    }


    switch (current_renderer) {

        case (RenderType) Empty:
            break;

            // 2D
        case Dim2:
            glfwMakeContextCurrent(render_window);
            re_render = true;
            renderer_2d.init();
            renderer_2d.getTexture("background")->genNewTexture(grid_size, grid_size);
            renderer_2d.setBounds(grid_length, grid_length);
            break;
        case Dim3:
            break;
    }
}

bool JSSFRender(void *imgPtr) {
    bool new_image = false;

    if (iter == iter_per_frame)
        iter = 0;

    if (is_resetting) {
        iter = (iter_per_frame - 1);
        jssf_solver->resetFluid();
        grid_smoke2d->resetSmoke();
        is_resetting = false;
        new_image = true;
    } else if (is_animating || next_frame) {
        failed_step = jssf_solver->calcNextStep(forces);
        if (!failed_step && iter == (iter_per_frame - 1)) {
            next_frame = false;
            new_image = true;
            if (render_enabled)
                grid_smoke2d->updateSmoke(sources, jssf_solver->velocityData());
        } else if (failed_step) {
            is_animating = false;
            next_frame = false;
        }
    } else if (re_render) {
        iter = (iter_per_frame - 1);
        re_render = false;
        new_image = true;
    }
    if (new_image) { *(float **) imgPtr = grid_smoke2d->smokeData(); }
    return new_image;
}

bool JSSFIterRender(void *imgPtr) {
    bool new_image = false;

    if (iter == iter_per_frame)
        iter = 0;

    if (is_resetting) {
        iter = (iter_per_frame - 1);
        jssf_solver_iter->resetFluid();
        grid_smoke2d->resetSmoke();
        is_resetting = false;
        new_image = true;
    } else if (is_animating || next_frame) {
        failed_step = jssf_solver_iter->calcNextStep(forces);
        if (!failed_step && iter == (iter_per_frame - 1)) {
            next_frame = false;
            new_image = true;
            if (render_enabled)
                grid_smoke2d->updateSmoke(sources, jssf_solver_iter->velocityData());
        } else if (failed_step) {
            is_animating = false;
            next_frame = false;
        }
    } else if (re_render) {
        iter = (iter_per_frame - 1);
        re_render = false;
        new_image = true;
    }
    if (new_image) { *(float **) imgPtr = grid_smoke2d->smokeData(); }
    return new_image;
}

bool LBMRender(void *imgPtr) {
    bool new_image = false;

    if (iter == iter_per_frame)
        iter = 0;

    if (is_resetting) {
        iter = (iter_per_frame - 1);
        lbm_solver->ResetFluid();
        grid_smoke2d->resetSmoke();
        is_resetting = false;
        re_render = true;
    } else if (is_animating || next_frame) {
        failed_step = lbm_solver->CalcNextStep(forces);
        if (!failed_step && iter == (iter_per_frame - 1)) {
            next_frame = false;
            new_image = true;
            if (render_enabled)
                grid_smoke2d->updateSmoke(sources, lbm_solver->VelocityData());
        } else if (failed_step) {
            is_animating = false;
            next_frame = false;
        }
    } else if (re_render) {
        iter = (iter_per_frame - 1);
        re_render = false;
        new_image = true;
    }
    if (new_image) {
        if (!view_density) { *(float **) imgPtr = grid_smoke2d->smokeData(); }
        else { *(float **) imgPtr = lbm_solver->MappedRhoData(); }
    }

    return new_image;
}

bool JSSF3DRender(void *imgPtr) {
    bool new_image = false;

    if (iter == iter_per_frame)
        iter = 0;

    if (is_resetting) {
        iter = (iter_per_frame - 1);
        jssf_solver_3d->resetFluid();
        grid_smoke3d->resetSmoke();
        is_resetting = false;
        new_image = true;
    } else if (is_animating || next_frame) {
        failed_step = jssf_solver_3d->calcNextStep(forces);
        if (!failed_step && iter == (iter_per_frame - 1)) {
            next_frame = false;
            new_image = true;
            if (render_enabled)
                grid_smoke3d->updateSmoke(sources, jssf_solver_3d->velocityData());
        } else if (failed_step) {
            is_animating = false;
            next_frame = false;
        }
    } else if (re_render) {
        iter = (iter_per_frame - 1);
        re_render = false;
        new_image = true;
    }
    if (new_image) { *(float **) imgPtr = grid_smoke3d->smokeData(); }
    return new_image;
}

void renderSims() {
    static std::future<bool> future;

    static bool waiting_to_render = false;

    if (update_renderer)
        doRendererUpdate();
    update_renderer = false;

    // here we start the async method
    // outside of the frame rate if statement
    // so that the overhead of starting 
    // async doesn't interfere with framerate
    // calculation
    if (!is_updating && !is_calc_frame) {
        switch (currentSolver) {
            case Jssf:
                if ((next_frame || is_animating || is_resetting || re_render)) {
                    future = std::async(std::launch::async, JSSFRender, (void *) &img);
                    is_calc_frame = true;
                }
                break;
            case JssfIter:
                if ((next_frame || is_animating || is_resetting || re_render)) {
                    future = std::async(std::launch::async, JSSFIterRender, (void *) &img);
                    is_calc_frame = true;
                }
                break;
            case Lbm:
                if ((next_frame || is_animating || is_resetting || re_render)) {
                    future = std::async(std::launch::async, LBMRender, (void *) &img);
                    is_calc_frame = true;
                }
                break;
            case Jssf3D:
                if ((next_frame || is_animating || is_resetting || re_render)) {
                    future = std::async(std::launch::async, JSSF3DRender, (void *) &img);
                    is_calc_frame = true;
                }
                break;
        }
    }

    if (!is_updating) {
        switch (currentSolver) {
            case Empty:
                sim_fps = 0;
                break;

                // 2D
            case Jssf:
            case JssfIter:
            case Lbm:
                if (waiting_to_render) {
                    if (max_fps == 0 || current_time - old_sim_time > 1 / max_fps) {
                        sim_fps = 1 / (current_time - old_sim_time);
                        old_sim_time = glfwGetTime();
                        renderer_2d.getTexture("background")->loadPixels(GL_RGB, GL_FLOAT, img);
                        waiting_to_render = false;
                        is_calc_frame = false;
                    }
                }
                else if (is_calc_frame && (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
                    if (future.get()) {
                        waiting_to_render = true;
                    } else {
                        is_calc_frame = false;
                    }
                    iter++;
                }
                break;

                // 3D
            case Jssf3D:
                if (is_calc_frame && (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
                    is_calc_frame = false;
                    if (future.get()) {
                        sim_fps = 1 / (current_time - old_sim_time);
                        old_sim_time = glfwGetTime();
                    }
                    iter++;
                }
                break;
        }
    }
}