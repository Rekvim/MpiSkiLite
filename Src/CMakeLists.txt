set(DOMAIN_CPP
    Domain/DeviceInitializer.cpp
    Domain/Measurement/Sensor.cpp
    Domain/Mpi/Device.cpp
    Domain/Mpi/Settings.cpp
    Domain/Tests/Main/Algorithm.cpp
    Domain/Tests/Main/AlgorithmDoubleActing.cpp
    Domain/Tests/Main/Analyzer.cpp
    Domain/Tests/Main/Runner.cpp
    Domain/Tests/Main/RunnerDoubleActing.cpp
    Domain/Tests/Main/ScenarioDoubleActing.cpp
    Domain/Tests/Option/Algorithm.cpp
    Domain/Tests/Option/Step/Analyzer.cpp
    Domain/Tests/Option/Step/Runner.cpp
    Domain/Tests/Stroke/Algorithm.cpp
    Domain/Tests/Stroke/Analyzer.cpp
    Domain/Tests/Stroke/Runner.cpp
    Domain/Uart/Message.cpp
    Domain/Uart/Port.cpp
    Domain/Uart/Reader.cpp
    Domain/Tests/BaseRunner.cpp
    Domain/Tests/AbstractTestAlgorithm.cpp
    Domain/Tests/CyclicPositioner/CyclicTestPositioner.cpp
    Domain/Tests/Option/Response/Runner.cpp
    Domain/Tests/Option/Resolution/Runner.cpp
    Domain/Tests/Option/Resolution/Scenario.cpp
    Domain/Tests/Option/Response/Scenario.cpp
    Domain/Tests/Option/Step/Scenario.cpp
    Domain/Tests/Main/Scenario.cpp
    Domain/Tests/Stroke/Scenario.cpp
    Domain/Tests/AbstractScenario.cpp


    Domain/Tests/ScenarioFactory.cpp
    Domain/Program.cpp

    Storage/AppSettings.cpp
    Storage/Registry.cpp
    Storage/ChartImageStorage.cpp

    Utils/NumberUtils.cpp

    ValidatorFactory/ValidatorFactory.cpp
)

set(DOMAIN_HEADERS
    Domain/DeviceInitializer.h
    Domain/Measurement/Sample.h
    Domain/Measurement/Sensor.h
    Domain/Measurement/TestDataBuffer.h
    Domain/Mpi/Device.h
    Domain/Mpi/Settings.h

    Domain/Tests/IAnalyzer.h
    Domain/Tests/Main/Algorithm.h
    Domain/Tests/Main/AlgorithmDoubleActing.h
    Domain/Tests/Main/Analyzer.h
    Domain/Tests/Main/Params.h
    Domain/Tests/Main/Result.h
    Domain/Tests/Main/Runner.h
    Domain/Tests/Main/RunnerDoubleActing.h
    Domain/Tests/Main/ScenarioDoubleActing.h
    Domain/Tests/Option/Algorithm.h

    Domain/Tests/Option/Step/Analyzer.h
    Domain/Tests/Option/Step/Params.h
    Domain/Tests/Option/Step/Result.h
    Domain/Tests/Option/Step/Runner.h
    Domain/Tests/Stroke/Algorithm.h
    Domain/Tests/Stroke/Analyzer.h
    Domain/Tests/Stroke/Result.h
    Domain/Tests/Stroke/Runner.h
    Domain/Uart/Message.h
    Domain/Uart/Port.h
    Domain/Uart/Reader.h
    Domain/Program.h
    Domain/Tests/AbstractTestAlgorithm.h
    Domain/Tests/BaseRunner.h
    Domain/Tests/CyclicPositioner/CyclicTestPositioner.h
    Domain/Tests/Option/Params.h
    Domain/Tests/Option/Response/Runner.h
    Domain/Tests/Option/Resolution/Runner.h
    Domain/DeviceConfig.h
    Domain/Tests/Context.h

    Domain/Tests/Option/Step/Scenario.h
    Domain/Tests/Stroke/Scenario.h
    Domain/Tests/Option/Resolution/Scenario.h
    Domain/Tests/Option/Response/Scenario.h
    Domain/Tests/Main/Scenario.h
    Domain/Tests/ScenarioFactory.h
    Domain/Tests/AbstractScenario.h

    Storage/AppSettings.h
    Storage/Registry.h
    Storage/SettingsGroup.h
    Storage/Telemetry.h
    Storage/ChartImageStorage.h


    Utils/NumberUtils.h
    Utils/SignalUtils.h
    Utils/Shortcuts/TabActionRouter.h
    Utils/Shortcuts/TabBinder.h

    ValidatorFactory/ValidatorFactory.h
    ValidatorFactory/RegexPatterns.h
)

set(REPORT_CPP
    Report/Builder.cpp
    Report/Pages/TechnicalInspection.cpp
    Report/Pages/StepReaction.cpp
    Report/Blocks/StepReactionTable.cpp
    Report/Blocks/ObjectInfo.cpp
    Report/Blocks/ValveSpec.cpp
    Report/Blocks/TechnicalResults.cpp
    Report/Blocks/Materials.cpp
    Report/Saver.cpp
    Report/Writer.cpp
)

set(REPORT_HEADERS
    Report/Builder.h
    Report/Pages/TechnicalInspection.h
    Report/Pages/StepReaction.h
    Report/Blocks/ObjectInfo.h
    Report/Blocks/StepReactionTable.h
    Report/Blocks/TechnicalResults.h
    Report/Blocks/ValveSpec.h
    Report/Blocks/Materials.h
    Report/Saver.h
    Report/Data.h
    Report/Writer.h
)

set(GUI_CPP
    Gui/MainWindow/CrossingIndicatorsPresenter.cpp
    Gui/MainWindow/TelemetryUiMapper.cpp
    Gui/MainWindow/TestController.cpp
    Gui/Setup/ValveWindow/Logic.cpp
    Gui/Setup/ValveWindow/Mapper.cpp
    Gui/Setup/ObjectWindow.cpp
    Gui/Setup/ValveWindow/ValveWindow.cpp
    Gui/Setup/SelectTests.cpp
    Gui/MainWindow/MainWindow.cpp
    Gui/TestSettings/BaseSequenceSettingsDialog.cpp
    Gui/TestSettings/OtherTestSettings.cpp
    Gui/TestSettings/StepTestSettings.cpp
    Gui/TestSettings/MainTestSettings.cpp
    Widgets/Chart/ChartView.cpp
    Widgets/Chart/Series.cpp
    Widgets/Chart/ImageService.cpp
    Widgets/Chart/Manager.cpp
    Widgets/Slider/SliderView.cpp
)

set(GUI_HEADERS
    Gui/MainWindow/CrossingIndicatorsPresenter.h
    Gui/MainWindow/TelemetryUiMapper.h
    Gui/MainWindow/TestController.h
    Gui/Setup/ObjectWindow.h
    Gui/Setup/ValveWindow/Logic.h
    Gui/Setup/ValveWindow/ValveEnums.h
    Gui/Setup/ValveWindow/ValveWindow.h
    Gui/Setup/SelectTests.h
    Gui/Setup/ValveWindow/Mapper.h
    Gui/TestSettings/BaseSequenceSettingsDialog.h
    Gui/TestSettings/OtherTestSettings.h
    Gui/TestSettings/StepTestSettings.h
    Gui/TestSettings/MainTestSettings.h
    Gui/MainWindow/MainWindow.h
    Widgets/Chart/ChartView.h
    Widgets/Chart/Series.h
    Widgets/Chart/ChartType.h
    Widgets/Chart/Point.h
    Widgets/Chart/ImageService.h
    Widgets/Chart/Manager.h
    Widgets/Slider/SliderView.h
)

set(GUI_UI
    Gui/Setup/ObjectWindow.ui
    Gui/Setup/SelectTests.ui
    Gui/Setup/ValveWindow/ValveWindow.ui
    Gui/MainWindow/MainWindow.ui
    Gui/TestSettings/MainTestSettings.ui
    Gui/TestSettings/OtherTestSettings.ui
    Gui/TestSettings/StepTestSettings.ui
)

add_library(Domain STATIC
    ${DOMAIN_CPP}
    ${DOMAIN_HEADERS}
)

target_include_directories(Domain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Domain
    PUBLIC
        Qt6::Core
        Qt6::Charts
        Qt6::SerialPort
)

target_sources(Domain
  PRIVATE
)

add_library(Report STATIC
    ${REPORT_CPP}
    ${REPORT_HEADERS}
)

target_include_directories(Report
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Report
    PUBLIC
        Qt6::Core
        QXlsx
        Domain
)

add_library(Gui STATIC
    ${GUI_CPP}
    ${GUI_HEADERS}
    ${GUI_UI}
)

target_include_directories(Gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Gui
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Charts
        Domain
        Report
)