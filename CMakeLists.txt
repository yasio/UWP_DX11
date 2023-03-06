# cmake -B build -DCMAKE_SYSTEM_NAME=WindowsStore "-DCMAKE_SYSTEM_VERSION=10.0"
cmake_minimum_required(VERSION 3.10)

set(APP_NAME UWP_DX11)
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.19041.0")

project(${APP_NAME})

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

set(RESOURCE_FILES ${CONTENT_FILES} ${ASSET_FILES})
 
set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")

set_property(SOURCE "App.xaml" PROPERTY VS_XAML_TYPE "ApplicationDefinition")
 
add_executable(${APP_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${XAML_FILES})

set_source_files_properties(pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")
set_target_properties(${APP_NAME} PROPERTIES COMPILE_FLAGS "/Yupch.h")

target_compile_options(${APP_NAME} PRIVATE /ZW)

target_compile_definitions(${APP_NAME} PRIVATE -DYASIO_HEADER_ONLY=1)

target_include_directories(${APP_NAME} PRIVATE .)

#### CUSTOM CMAKE STEPS
set_source_files_properties(App.h PROPERTIES OBJECT_DEPENDS "App.xaml")
set_source_files_properties(DirectXPage.h PROPERTIES OBJECT_DEPENDS "DirectXPage.xaml")
set_target_properties(${APP_NAME} PROPERTIES VS_CPPWINRT true)
#### END CUSTOM CMAKE STEPS
