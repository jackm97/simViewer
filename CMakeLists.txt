cmake_minimum_required(VERSION 3.13.0)
project(simViewer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/extern)

#########################LIBGLR########################
# SET SOME OPTIONS FOR LIBGLR
#   GLR PATH
set(PATH_TO_GLR ${PROJECT_SOURCE_DIR}/extern/GLR/)
#   EXTERNAL LIB PATHS
set(GLM_PATH ${PATH_TO_GLR}/extern/glm)
set(GLAD_PATH ${PATH_TO_GLR}/extern/glad)
set(GLAD_SRC_NAME glad.c)

add_definitions(-DGLRENDER_STATIC)
add_subdirectory(${PATH_TO_GLR})

include_directories(${PATH_TO_GLR}
                    ${GLM_PATH}
                    ${GLAD_PATH}/include)

add_library(glad STATIC ${GLAD_PATH}/src/${GLAD_SRC_NAME})
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
#########################LIBGLR########################

#########################LIBJFS########################
# SET SOME OPTIONS FOR LIBJFS
#   JFS PATH
set(PATH_TO_JFS ${PROJECT_SOURCE_DIR}/extern/JFS/)
#   EXTERNAL LIB PATHS
set(EIGEN_PATH ${PROJECT_SOURCE_DIR}/extern/JFS/extern/eigen)

add_definitions(-DJFS_STATIC)
add_subdirectory(${PATH_TO_JFS})

include_directories(${PATH_TO_JFS}
                    ${EIGEN_PATH})
#########################LIBJFS########################

#########################IMGUI##########################
set(IMGUI_PATH ${PROJECT_SOURCE_DIR}/extern/imgui)
set(IMGUI_BACKENDS_PATH ${IMGUI_PATH}/backends)
include_directories(${IMGUI_PATH})
set(IMGUI_SRC   ${IMGUI_PATH}/imgui.cpp
                ${IMGUI_PATH}/imgui_draw.cpp 
                ${IMGUI_PATH}/imgui_widgets.cpp 
                ${IMGUI_PATH}/imgui_tables.cpp
                ${IMGUI_BACKENDS_PATH}/imgui_impl_glfw.cpp 
                ${IMGUI_BACKENDS_PATH}/imgui_impl_opengl3.cpp 
                ${PROJECT_SOURCE_DIR}/extern/ImGuiFileDialog/ImGuiFileDialog.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

add_library(imgui ${IMGUI_SRC})
#########################IMGUI##########################

#########################OPENVDB########################
# SET SOME OPTIONS FOR OPENVDB
#   OPENVDB PATH
set(PATH_TO_OPENVDB ${PROJECT_SOURCE_DIR}/extern/openvdb/)

add_subdirectory(${PATH_TO_OPENVDB})
#########################OPENVDB########################

add_executable(simViewer main.cpp)
target_link_libraries(simViewer PUBLIC pthread jfs glr glad glfw imgui openvdb)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    add_definitions(-DUSE_OPENMP)
    target_link_libraries(jfs PUBLIC OpenMP::OpenMP_CXX)
endif()
