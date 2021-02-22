cmake_minimum_required(VERSION 3.13.0)
project(simViewer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/extern)

#########################GLFW########################
find_package(glfw3 3.3 REQUIRED)
#########################GLFW########################

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
set(IMGUI_FONTS_PATH ${IMGUI_PATH}/misc/fonts)
include_directories(${IMGUI_PATH})
set(IMGUI_SRC   ${IMGUI_PATH}/imgui.cpp
                ${IMGUI_PATH}/imgui_draw.cpp 
                ${IMGUI_PATH}/imgui_widgets.cpp 
                ${IMGUI_PATH}/imgui_tables.cpp
                ${IMGUI_BACKENDS_PATH}/imgui_impl_glfw.cpp 
                ${IMGUI_BACKENDS_PATH}/imgui_impl_opengl3.cpp 
                ${PROJECT_SOURCE_DIR}/extern/ImGuiFileDialog/ImGuiFileDialog.cpp)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# put fonts in binary directory
file(COPY ${IMGUI_FONTS_PATH} DESTINATION ${CMAKE_BINARY_DIR})
#########################IMGUI##########################

#########################OPENVDB########################
# SET SOME OPTIONS FOR OPENVDB
#   OPENVDB INSTALL PATH
if (WIN32)
    set(OPENVDB_INSTALL_PATH "C:/Program Files/OpenVDB")
elseif(UNIX)
    set(OPENVDB_INSTALL_PATH "/usr/local/")
endif()
list(APPEND CMAKE_MODULE_PATH "${OPENVDB_INSTALL_PATH}/lib/cmake/OpenVDB")

if (WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    file(COPY "${OPENVDB_INSTALL_PATH}/bin/openvdb.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    
    add_definitions(-D__TBB_NO_IMPLICIT_LINKAGE=1)

    # BELOW IS REQUIRED WHEN DOING DEBUG CONFIG
    # DUE TO OPENVDB NOT HAVING PROPER DEBUG SETUP
    # ON WINDOWS
        get_filename_component(VCPKG_PATH ${CMAKE_TOOLCHAIN_FILE} DIRECTORY) 
        set(VCPKG_PACKAGE_PATH "${VCPKG_PATH}/../../packages")
        
        set(HALF_DLL_PATH "${VCPKG_PACKAGE_PATH}/openexr_x64-windows")
        set(HALF_DLL_FILENAME "Half-2_5.dll")
        file(COPY "${HALF_DLL_PATH}/bin/${HALF_DLL_FILENAME}" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        
        set(TBB_DLL_PATH "${VCPKG_PACKAGE_PATH}/tbb_x64-windows")
        file(COPY "${TBB_DLL_PATH}/bin/tbb.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        
        set(ZLIB_DLL_PATH "${VCPKG_PACKAGE_PATH}/zlib_x64-windows")
        file(COPY "${ZLIB_DLL_PATH}/bin/zlib1.dll" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

find_package(OpenVDB REQUIRED)
#########################OPENVDB########################

#########################THREADS########################
find_package (Threads REQUIRED)
#########################THREADS########################

set(SIMVIEWER_LIBS  ${THREADS_LIB}
                    jfs
                    glr
                    glad
                    glfw
                    OpenVDB::openvdb)

add_executable(simViewer main.cpp ${IMGUI_SRC})
target_link_libraries(simViewer PUBLIC ${SIMVIEWER_LIBS})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    add_definitions(-DUSE_OPENMP)
    target_link_libraries(jfs PUBLIC OpenMP::OpenMP_CXX)
endif()
