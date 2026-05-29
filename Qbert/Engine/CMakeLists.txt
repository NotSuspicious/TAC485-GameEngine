project(Engine)

# This grabs all files in this directory that end with .cpp, .h, and .hpp
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp" "*.h" "*.hpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})

add_library(Engine STATIC ${SOURCE_FILES}
        TestCube.cpp
        TestCube.h
        Material.cpp
        Material.h
        Lighting.cpp
        Lighting.h
        Component.cpp
        Component.h
        Components/PointLight.cpp
        Components/PointLight.h
        BoneTransform.cpp
        BoneTransform.h
        Skeleton.cpp
        Skeleton.h
        Animation.cpp
        Animation.h
        SkinnedObj.cpp
        SkinnedObj.h
        Components/Character.cpp
        Components/Character.h
        Physics.cpp
        Physics.h
        Components/CollisionBox.cpp
        Components/CollisionBox.h
        JobManager.cpp
        JobManager.h
        ParabolicCurve.cpp
        ParabolicCurve.h
        Curve.h)

# debug define
target_compile_definitions(Engine PRIVATE _DEBUG)

# Precompiled Headers
target_precompile_headers(Engine PRIVATE pch.h)

# Add additional include directories
target_include_directories(Engine PRIVATE .)
target_include_directories(Engine PRIVATE ../../external/SDL3/include)
target_include_directories(Engine PRIVATE ../../external)
