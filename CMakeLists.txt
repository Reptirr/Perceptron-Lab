cmake_minimum_required(VERSION 3.16)

project(PerceptronLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(PerceptronLab
        Network/include/ActivationApplicator.h
        Network/include/ActivationType.h
        Network/src/Layer.cpp
        Network/include/Layer.h
        Network/include/Matrix.h
        Network/src/NeuroNetwork.cpp
        Network/include/NeuroNetwork.h

        Network/include/Randomizer.h
        UI/src/controlpanel.cpp
        UI/include/controlpanel.h
        UI/main.cpp
        UIDriver/include/model.h
        UI/src/networkgraph.cpp
        UI/include/networkgraph.h
        UIDriver/src/uidriver.cpp
        UIDriver/include/uidriver.h
        UI/src/widget.cpp
        UI/include/widget.h
        UI/include/inputfield.h UI/src/inputfield.cpp
        UI/include/styleloader.h

        resources/resources.qrc
        UIDriver/include/utils.h
        UIDriver/include/defaultmodel.h
        UIDriver/include/samples.h

        UI/include/predictinputfield.h UI/src/predictinputfield.cpp
        UI/include/rightpanel.h UI/src/rightpanel.cpp
        Network/src/Matrix.cpp
)

target_include_directories(PerceptronLab PRIVATE
        Network/include
        UI/include
        UIDriver/include
)

target_link_libraries(PerceptronLab PRIVATE Qt6::Widgets)
set_target_properties(PerceptronLab PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
