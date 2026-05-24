cmake_minimum_required(VERSION 3.16)

project(PerceptronLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(PerceptronLab
    ActivationApplicator.h
    ActivationType.h
    Layer.cpp
    Layer.h
    Matrix.h
    NeuroNetwork.cpp
    NeuroNetwork.h

    Randomizer.h
    controlpanel.cpp
    controlpanel.h
    main.cpp
    model.h
    networkgraph.cpp
    networkgraph.h
    uidriver.cpp
    uidriver.h
    widget.cpp
    widget.h
    inputfield.h inputfield.cpp
    styleloader.h

    resources.qrc
    utils.h
    defaultmodel.h
    samples.h
    diplom.md
    predictinputfield.h predictinputfield.cpp
    rightpanel.h rightpanel.cpp
    Matrix.cpp
)


source_group("Network/headers" FILES ActivationApplicator.h ActivationType.h Layer.h Matrix.h NeuroNetwork.h NeuronType.h Randomizer.h)
source_group("Network/src" FILES Layer.cpp NeuroNetwork.cpp)
source_group("Ui" FILES main.cpp)
source_group("Ui/headers" FILES styleloader.h controlpanel.h networkgraph.h widget.h inputfield.h predictinputfield.h rightpanel.h)
source_group("Ui/src" FILES controlpanel.cpp networkgraph.cpp widget.cpp inputfield.cpp predictinputfield.cpp rightpanel.cpp)
source_group("UiDriver/headers" FILES model.h uidriver.h utils.h  defaultmodel.h samples.h)
source_group("UiDriver/src" FILES uidriver.cpp)

target_link_libraries(PerceptronLab PRIVATE Qt6::Widgets)
set_target_properties(PerceptronLab PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
