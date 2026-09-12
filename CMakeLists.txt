cmake_minimum_required(VERSION 3.16)

project(AMDO VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 第三方依赖：yaml-cpp（评价规格 evaluation-spec.yaml 序列化）。
# 获取顺序（无 yaml-cpp 的机器也能编译，排障见 docs/build.md）：
#   1) 本地已安装（find_package，支持 vcpkg / 系统安装 / 手动 install，可离线）
#   2) 仓库内 vendored 源码 third_party/yaml-cpp（若存在，完全离线）
#   3) 联网 FetchContent 从 GitHub 拉取 0.8.0（首次需可访问 GitHub）
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

find_package(yaml-cpp 0.8 QUIET)
if(NOT TARGET yaml-cpp AND NOT TARGET yaml-cpp::yaml-cpp)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp/CMakeLists.txt")
        add_subdirectory(third_party/yaml-cpp yaml-cpp-build)
    else()
        include(FetchContent)
        FetchContent_Declare(
            yaml-cpp
            GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
            GIT_TAG 0.8.0
        )
        FetchContent_MakeAvailable(yaml-cpp)
    endif()
endif()
# 统一别名：无论来源如何，都保证 yaml-cpp::yaml-cpp 可用。
if(NOT TARGET yaml-cpp::yaml-cpp AND TARGET yaml-cpp)
    add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        ui/theme.h
        ui/uihelpers.h
        ui/uihelpers.cpp
        ui/chartwidgets.h
        ui/chartwidgets.cpp
        model/srdtypes.h
        model/srdcatalogs.h
        model/srdcatalogs.cpp
        model/srdstore.h
        model/srdstore.cpp
        model/aircrafttypes.h
        model/aircraftcatalogs.h
        model/aircraftcatalogs.cpp
        model/aircraftstore.h
        model/aircraftstore.cpp
        model/analysistypes.h
        model/analysisresult.h
        model/evaluationresult.h
        model/studytypes.h
        model/workflowtypes.h
        model/analysiscatalogs.h
        model/analysiscatalogs.cpp
        model/analysisstore.h
        model/analysisstore.cpp
        service/srddocumentservice.h
        service/srddocumentservice.cpp
        service/srdcompletenessservice.h
        service/srdcompletenessservice.cpp
        service/srdderivationservice.h
        service/srdderivationservice.cpp
        service/srdimportexportservice.h
        service/srdimportexportservice.cpp
        service/aircraftdocumentservice.h
        service/aircraftdocumentservice.cpp
        service/aircraftimportexportservice.h
        service/aircraftimportexportservice.cpp
        service/aircraftcpacsservice.h
        service/aircraftcpacsservice.cpp
        service/analysisdocumentservice.h
        service/analysisdocumentservice.cpp
        service/analysiscomputeservice.h
        service/analysiscomputeservice.cpp
        service/evaluationservice.h
        service/evaluationservice.cpp
        service/studyservice.h
        service/studyservice.cpp
        service/workflowservice.h
        service/workflowservice.cpp
        controller/requirementspresenter.h
        controller/requirementspresenter.cpp
        controller/definitionpresenter.h
        controller/definitionpresenter.cpp
        controller/analysispresenter.h
        controller/analysispresenter.cpp
        controller/decisionpresenter.h
        controller/decisionpresenter.cpp
        controller/designpresenter.h
        controller/designpresenter.cpp
        controller/workflowpresenter.h
        controller/workflowpresenter.cpp
        ui/requirementspage.h
        ui/requirementspage.cpp
        ui/definitionpage.h
        ui/definitionpage.cpp
        ui/analysispage.h
        ui/analysispage.cpp
        ui/designpage.h
        ui/designpage.cpp
        ui/decisionpage.h
        ui/decisionpage.cpp
        ui/workflowpage.h
        ui/workflowpage.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AMDO
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AMDO APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AMDO SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AMDO
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AMDO PRIVATE Qt${QT_VERSION_MAJOR}::Widgets yaml-cpp::yaml-cpp)
target_include_directories(AMDO PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

if(MINGW)
    target_compile_options(AMDO PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AMDO)
endif()
set_target_properties(AMDO PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AMDO
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AMDO)
endif()
