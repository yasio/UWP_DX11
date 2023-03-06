# cmake -B build -DCMAKE_SYSTEM_NAME=WindowsStore "-DCMAKE_SYSTEM_VERSION=10.0"
cmake_minimum_required(VERSION 3.10)

set(APP_NAME UWP_DX11)

project(${APP_NAME})

macro(source_group_by_dir proj_dir source_files)
    if(MSVC OR APPLE)
        get_filename_component(sgbd_cur_dir ${proj_dir} ABSOLUTE)
        foreach(sgbd_file ${${source_files}})
            get_filename_component(sgbd_abs_file ${sgbd_file} ABSOLUTE)
            file(RELATIVE_PATH sgbd_fpath ${sgbd_cur_dir} ${sgbd_abs_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC OR APPLE)
endmacro(source_group_by_dir)

set(HEADER_FILES
  pch.h
  App.xaml.h
  DirectXPage.xaml.h
  Common/DeviceResources.h
  Common/DirectXHelper.h
  Common/StepTimer.h
  )
  
set(SOURCE_FILES
  pch.cpp
  App.xaml.cpp
  DirectXPage.xaml.cpp
  UWP_DX11Main.cpp
  Common/DeviceResources.cpp
  Content/Sample3DSceneRenderer.cpp
  Content/SampleFpsTextRenderer.cpp
  )

set(XAML_FILES
  App.xaml
  DirectXPage.xaml
  )
  
file(GLOB_RECURSE ASSET_FILES
    Assets/*.png
    )

set(CONTENT_FILES
    Package.appxmanifest
    )

set(HLSL_SHADER_FILES Content/SampleVertexShader.hlsl Content/SamplePixelShader.hlsl)

set(RESOURCE_FILES ${CONTENT_FILES} ${ASSET_FILES})
 
set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")

set_property(SOURCE "App.xaml" PROPERTY VS_XAML_TYPE "ApplicationDefinition")

set(APP_ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${XAML_FILES} ${HLSL_SHADER_FILES})
source_group_by_dir(${CMAKE_CURRENT_SOURCE_DIR} APP_ALL_SOURCES)

add_executable(${APP_NAME} ${APP_ALL_SOURCES})

set_source_files_properties(Content/SampleVertexShader.hlsl PROPERTIES VS_SHADER_TYPE Vertex)
set_source_files_properties(Content/SamplePixelShader.hlsl PROPERTIES VS_SHADER_TYPE Pixel)
# set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES VS_SHADER_MODEL 4.0)

set_source_files_properties(pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")
set_target_properties(${APP_NAME} PROPERTIES COMPILE_FLAGS "/Yupch.h")

set_target_properties(${APP_NAME} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.19041.0")

target_compile_options(${APP_NAME} PRIVATE /ZW)

target_compile_definitions(${APP_NAME} PRIVATE -DYASIO_HEADER_ONLY=1)

target_include_directories(${APP_NAME} PRIVATE .)

#### CUSTOM CMAKE STEPS
# set_source_files_properties(App.h PROPERTIES OBJECT_DEPENDS "App.xaml")
# set_source_files_properties(DirectXPage.h PROPERTIES OBJECT_DEPENDS "DirectXPage.xaml")
# set_target_properties(${APP_NAME} PROPERTIES VS_CPPWINRT true)
#### END CUSTOM CMAKE STEPS
