#set the minimum cmake version required
cmake_minimum_required(VERSION 3.18.0)

#set the name of the project
project(SerialPrograms)

#enable c++ 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#produce clang tidy file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set root dir for convenience
set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)

add_custom_target(build-time-make-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory Assembly/)

# Find repo root folder path
string(FIND "${CMAKE_CURRENT_SOURCE_DIR}" "Arduino-Source-Internal" internal_repro_position)  # determine if this is an internal repo
    if (internal_repro_position EQUAL -1)  # no "Arduino-Source-Internal" substr in the current source dir
        # we are building the public repo
        set(REPO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
    else()
        # we are building the internal repo
        set(REPO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
    endif()

#Find threads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#find Qt
if(NOT QT_MAJOR)
    set(QT_MAJOR 6)
endif()
find_package(Qt${QT_MAJOR} COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets REQUIRED)
#disable deprecated Qt APIs
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

#add current directory to find tesseractPA.lib
link_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB MAIN_SOURCES
    ../3rdParty/ONNX/OnnxToolsPA.h
    ../3rdParty/QtWavFile/WavFile.cpp
    ../3rdParty/QtWavFile/WavFile.h
    ../3rdParty/TesseractPA/TesseractPA.cpp
    ../3rdParty/TesseractPA/TesseractPA.h
    ../3rdParty/qdarkstyle/dark/darkstyle.qrc
    ../ClientSource/Connection/BotBase.cpp
    ../ClientSource/Connection/BotBase.h
    ../ClientSource/Connection/BotBaseMessage.h
    ../ClientSource/Connection/MessageLogger.cpp
    ../ClientSource/Connection/MessageLogger.h
    ../ClientSource/Connection/MessageSniffer.h
    ../ClientSource/Connection/PABotBase.cpp
    ../ClientSource/Connection/PABotBase.h
    ../ClientSource/Connection/PABotBaseConnection.cpp
    ../ClientSource/Connection/PABotBaseConnection.h
    ../ClientSource/Connection/SerialConnection.h
    ../ClientSource/Connection/SerialConnectionPOSIX.h
    ../ClientSource/Connection/SerialConnectionWinAPI.h
    ../ClientSource/Connection/StreamInterface.h
    ../ClientSource/Libraries/Logging.cpp
    ../ClientSource/Libraries/Logging.h
    ../ClientSource/Libraries/MessageConverter.cpp
    ../ClientSource/Libraries/MessageConverter.h
    ../Common/CRC32.cpp
    ../Common/CRC32.h
    ../Common/Compiler.h
    ../Common/ControllerStates/HID_Keyboard_State.h
    ../Common/ControllerStates/NintendoSwitch2_WiredController_State.h
    ../Common/ControllerStates/NintendoSwitch_WirelessController_State.h
    ../Common/Cpp/AbstractLogger.h
    ../Common/Cpp/BitmapConversion.cpp
    ../Common/Cpp/BitmapConversion.h
    ../Common/Cpp/CancellableScope.cpp
    ../Common/Cpp/CancellableScope.h
    ../Common/Cpp/Color.cpp
    ../Common/Cpp/Color.h
    ../Common/Cpp/Concurrency/AsyncDispatcher.cpp
    ../Common/Cpp/Concurrency/AsyncDispatcher.h
    ../Common/Cpp/Concurrency/AsyncTask.cpp
    ../Common/Cpp/Concurrency/AsyncTask.h
    ../Common/Cpp/Concurrency/ComputationThreadPool.cpp
    ../Common/Cpp/Concurrency/ComputationThreadPool.h
    ../Common/Cpp/Concurrency/ComputationThreadPoolCore.cpp
    ../Common/Cpp/Concurrency/ComputationThreadPoolCore.h
    ../Common/Cpp/Concurrency/FireForgetDispatcher.cpp
    ../Common/Cpp/Concurrency/FireForgetDispatcher.h
    ../Common/Cpp/Concurrency/PeriodicScheduler.cpp
    ../Common/Cpp/Concurrency/PeriodicScheduler.h
    ../Common/Cpp/Concurrency/ReverseLockGuard.h
    ../Common/Cpp/Concurrency/ScheduledTaskRunner.cpp
    ../Common/Cpp/Concurrency/ScheduledTaskRunner.h
    ../Common/Cpp/Concurrency/SpinLock.cpp
    ../Common/Cpp/Concurrency/SpinLock.h
    ../Common/Cpp/Concurrency/SpinPause.h
    ../Common/Cpp/Concurrency/Watchdog.cpp
    ../Common/Cpp/Concurrency/Watchdog.h
    ../Common/Cpp/Containers/AlignedMalloc.cpp
    ../Common/Cpp/Containers/AlignedMalloc.h
    ../Common/Cpp/Containers/AlignedVector.h
    ../Common/Cpp/Containers/AlignedVector.tpp
    ../Common/Cpp/Containers/BoxSet.h
    ../Common/Cpp/Containers/CircularBuffer.h
    ../Common/Cpp/Containers/DllSafeString.h
    ../Common/Cpp/Containers/FixedLimitVector.h
    ../Common/Cpp/Containers/FixedLimitVector.tpp
    ../Common/Cpp/Containers/Pimpl.h
    ../Common/Cpp/Containers/Pimpl.tpp
    ../Common/Cpp/Containers/SparseArray.cpp
    ../Common/Cpp/Containers/SparseArray.h
    ../Common/Cpp/CpuId/CpuId.cpp
    ../Common/Cpp/CpuId/CpuId.h
    ../Common/Cpp/CpuId/CpuId_arm64.h
    ../Common/Cpp/CpuId/CpuId_arm64.tpp
    ../Common/Cpp/CpuId/CpuId_x86.h
    ../Common/Cpp/CpuId/CpuId_x86.tpp
    ../Common/Cpp/CpuUtilization/CpuUtilization.cpp
    ../Common/Cpp/CpuUtilization/CpuUtilization.h
    ../Common/Cpp/CpuUtilization/CpuUtilization_Linux.h
    ../Common/Cpp/CpuUtilization/CpuUtilization_Linux.tpp
    ../Common/Cpp/CpuUtilization/CpuUtilization_Windows.h
    ../Common/Cpp/CpuUtilization/CpuUtilization_Windows.tpp
    ../Common/Cpp/DateTime.h
    ../Common/Cpp/EnumStringMap.h
    ../Common/Cpp/EventRateTracker.h
    ../Common/Cpp/Exceptions.cpp
    ../Common/Cpp/Exceptions.h
    ../Common/Cpp/ExpressionEvaluator.cpp
    ../Common/Cpp/ExpressionEvaluator.h
    ../Common/Cpp/ImageResolution.cpp
    ../Common/Cpp/ImageResolution.h
    ../Common/Cpp/Json/JsonArray.cpp
    ../Common/Cpp/Json/JsonArray.h
    ../Common/Cpp/Json/JsonObject.cpp
    ../Common/Cpp/Json/JsonObject.h
    ../Common/Cpp/Json/JsonTools.cpp
    ../Common/Cpp/Json/JsonTools.h
    ../Common/Cpp/Json/JsonValue.cpp
    ../Common/Cpp/Json/JsonValue.h
    ../Common/Cpp/LifetimeSanitizer.cpp
    ../Common/Cpp/LifetimeSanitizer.h
    ../Common/Cpp/ListenerSet.h
    ../Common/Cpp/MemoryUtilization/MemoryUtilization.cpp
    ../Common/Cpp/MemoryUtilization/MemoryUtilization.h
    ../Common/Cpp/MemoryUtilization/MemoryUtilization_Linux.tpp
    ../Common/Cpp/MemoryUtilization/MemoryUtilization_Mac.tpp
    ../Common/Cpp/MemoryUtilization/MemoryUtilization_Windows.tpp
    ../Common/Cpp/Options/BatchOption.cpp
    ../Common/Cpp/Options/BatchOption.h
    ../Common/Cpp/Options/BooleanCheckBoxOption.cpp
    ../Common/Cpp/Options/BooleanCheckBoxOption.h
    ../Common/Cpp/Options/BoxFloatOption.cpp
    ../Common/Cpp/Options/BoxFloatOption.h
    ../Common/Cpp/Options/ButtonOption.cpp
    ../Common/Cpp/Options/ButtonOption.h
    ../Common/Cpp/Options/ColorOption.cpp
    ../Common/Cpp/Options/ColorOption.h
    ../Common/Cpp/Options/ConfigOption.cpp
    ../Common/Cpp/Options/ConfigOption.h
    ../Common/Cpp/Options/DateOption.cpp
    ../Common/Cpp/Options/DateOption.h
    ../Common/Cpp/Options/EditableTableOption.cpp
    ../Common/Cpp/Options/EditableTableOption.h
    ../Common/Cpp/Options/EnumDropdownDatabase.cpp
    ../Common/Cpp/Options/EnumDropdownDatabase.h
    ../Common/Cpp/Options/EnumDropdownOption.cpp
    ../Common/Cpp/Options/EnumDropdownOption.h
    ../Common/Cpp/Options/FixedCodeOption.cpp
    ../Common/Cpp/Options/FixedCodeOption.h
    ../Common/Cpp/Options/FloatingPointOption.cpp
    ../Common/Cpp/Options/FloatingPointOption.h
    ../Common/Cpp/Options/GroupOption.cpp
    ../Common/Cpp/Options/GroupOption.h
    ../Common/Cpp/Options/IntegerRangeOption.cpp
    ../Common/Cpp/Options/IntegerRangeOption.h
    ../Common/Cpp/Options/KeyBindingOption.cpp
    ../Common/Cpp/Options/KeyBindingOption.h
    ../Common/Cpp/Options/MacAddressOption.cpp
    ../Common/Cpp/Options/MacAddressOption.h
    ../Common/Cpp/Options/RandomCodeOption.cpp
    ../Common/Cpp/Options/RandomCodeOption.h
    ../Common/Cpp/Options/SimpleIntegerOption.cpp
    ../Common/Cpp/Options/SimpleIntegerOption.h
    ../Common/Cpp/Options/StaticTableOption.cpp
    ../Common/Cpp/Options/StaticTableOption.h
    ../Common/Cpp/Options/StaticTextOption.cpp
    ../Common/Cpp/Options/StaticTextOption.h
    ../Common/Cpp/Options/StringOption.cpp
    ../Common/Cpp/Options/StringOption.h
    ../Common/Cpp/Options/TextEditOption.cpp
    ../Common/Cpp/Options/TextEditOption.h
    ../Common/Cpp/Options/TimeDurationOption.cpp
    ../Common/Cpp/Options/TimeDurationOption.h
    ../Common/Cpp/Options/TimeExpressionOption.cpp
    ../Common/Cpp/Options/TimeExpressionOption.h
    ../Common/Cpp/PanicDump.cpp
    ../Common/Cpp/PanicDump.h
    ../Common/Cpp/PixelRGB32.h
    ../Common/Cpp/PrettyPrint.cpp
    ../Common/Cpp/PrettyPrint.h
    ../Common/Cpp/PrintDebuggers.h
    ../Common/Cpp/Rectangle.h
    ../Common/Cpp/Rectangle.tpp
    ../Common/Cpp/RecursiveThrottler.h
    ../Common/Cpp/SIMDDebuggers.h
    ../Common/Cpp/Sockets/AbstractClientSocket.h
    ../Common/Cpp/Sockets/ClientSocket.cpp
    ../Common/Cpp/Sockets/ClientSocket.h
    ../Common/Cpp/Sockets/ClientSocket_POSIX.h
    ../Common/Cpp/Sockets/ClientSocket_Qt.h
    ../Common/Cpp/Sockets/ClientSocket_WinSocket.h
    ../Common/Cpp/Stopwatch.h
    ../Common/Cpp/StreamConverters.cpp
    ../Common/Cpp/StreamConverters.h
    ../Common/Cpp/StringTools.cpp
    ../Common/Cpp/StringTools.h
    ../Common/Cpp/Time.cpp
    ../Common/Cpp/Time.h
    ../Common/Cpp/Unicode.cpp
    ../Common/Cpp/Unicode.h
    ../Common/Cpp/ValueDebouncer.h
    ../Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h
    ../Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h
    ../Common/NintendoSwitch/NintendoSwitch_SlotDatabase.h
    ../Common/PokemonSwSh/PokemonSwSh_FossilTable.h
    ../Common/PokemonSwSh/PokemonSwSh_MultiHostTable.cpp
    ../Common/PokemonSwSh/PokemonSwSh_MultiHostTable.h
    ../Common/Qt/AutoHeightTable.cpp
    ../Common/Qt/AutoHeightTable.h
    ../Common/Qt/AutoWidthLineEdit.cpp
    ../Common/Qt/AutoWidthLineEdit.h
    ../Common/Qt/CodeValidator.cpp
    ../Common/Qt/CodeValidator.h
    ../Common/Qt/CollapsibleGroupBox.cpp
    ../Common/Qt/CollapsibleGroupBox.h
    ../Common/Qt/NoWheelComboBox.h
    ../Common/Qt/Options/BatchWidget.cpp
    ../Common/Qt/Options/BatchWidget.h
    ../Common/Qt/Options/BooleanCheckBoxWidget.cpp
    ../Common/Qt/Options/BooleanCheckBoxWidget.h
    ../Common/Qt/Options/BoxFloatWidget.cpp
    ../Common/Qt/Options/BoxFloatWidget.h
    ../Common/Qt/Options/ButtonWidget.cpp
    ../Common/Qt/Options/ButtonWidget.h
    ../Common/Qt/Options/ColorWidget.cpp
    ../Common/Qt/Options/ColorWidget.h
    ../Common/Qt/Options/ConfigWidget.cpp
    ../Common/Qt/Options/ConfigWidget.h
    ../Common/Qt/Options/DateWidget.cpp
    ../Common/Qt/Options/DateWidget.h
    ../Common/Qt/Options/EditableTableWidget.cpp
    ../Common/Qt/Options/EditableTableWidget.h
    ../Common/Qt/Options/EnumDropdownWidget.cpp
    ../Common/Qt/Options/EnumDropdownWidget.h
    ../Common/Qt/Options/FixedCodeWidget.cpp
    ../Common/Qt/Options/FixedCodeWidget.h
    ../Common/Qt/Options/FloatingPointWidget.cpp
    ../Common/Qt/Options/FloatingPointWidget.h
    ../Common/Qt/Options/GroupWidget.cpp
    ../Common/Qt/Options/GroupWidget.h
    ../Common/Qt/Options/IntegerRangeWidget.cpp
    ../Common/Qt/Options/IntegerRangeWidget.h
    ../Common/Qt/Options/KeyBindingWidget.cpp
    ../Common/Qt/Options/KeyBindingWidget.h
    ../Common/Qt/Options/MacAddressWidget.cpp
    ../Common/Qt/Options/MacAddressWidget.h
    ../Common/Qt/Options/RandomCodeWidget.cpp
    ../Common/Qt/Options/RandomCodeWidget.h
    ../Common/Qt/Options/SimpleIntegerWidget.cpp
    ../Common/Qt/Options/SimpleIntegerWidget.h
    ../Common/Qt/Options/StaticTableWidget.cpp
    ../Common/Qt/Options/StaticTableWidget.h
    ../Common/Qt/Options/StaticTextWidget.cpp
    ../Common/Qt/Options/StaticTextWidget.h
    ../Common/Qt/Options/StringWidget.cpp
    ../Common/Qt/Options/StringWidget.h
    ../Common/Qt/Options/TextEditWidget.cpp
    ../Common/Qt/Options/TextEditWidget.h
    ../Common/Qt/Options/TimeDurationWidget.cpp
    ../Common/Qt/Options/TimeDurationWidget.h
    ../Common/Qt/Options/TimeExpressionWidget.cpp
    ../Common/Qt/Options/TimeExpressionWidget.h
    ../Common/Qt/Redispatch.cpp
    ../Common/Qt/Redispatch.h
    ../Common/Qt/SpinWaitWithEvents.h
    ../Common/Qt/StringToolsQt.cpp
    ../Common/Qt/StringToolsQt.h
    ../Common/Qt/TimeQt.cpp
    ../Common/Qt/TimeQt.h
    ../Common/Qt/WidgetStackFixedAspectRatio.cpp
    ../Common/Qt/WidgetStackFixedAspectRatio.h
    ../Common/SerialPABotBase/SerialPABotBase_Messages_HID_Keyboard.h
    ../Common/SerialPABotBase/SerialPABotBase_Messages_NS1_WirelessControllers.h
    ../Common/SerialPABotBase/SerialPABotBase_Messages_NS2_WiredController.h
    ../Common/SerialPABotBase/SerialPABotBase_Protocol.h
    ../Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h
    ../IconResource/IconResource.rc
    Source/CommonFramework/AudioPipeline/AudioConstants.h
    Source/CommonFramework/AudioPipeline/AudioFeed.h
    Source/CommonFramework/AudioPipeline/AudioInfo.cpp
    Source/CommonFramework/AudioPipeline/AudioInfo.h
    Source/CommonFramework/AudioPipeline/AudioNormalization.h
    Source/CommonFramework/AudioPipeline/AudioOption.cpp
    Source/CommonFramework/AudioPipeline/AudioOption.h
    Source/CommonFramework/AudioPipeline/AudioPassthroughPair.h
    Source/CommonFramework/AudioPipeline/AudioPipelineOptions.h
    Source/CommonFramework/AudioPipeline/AudioSession.cpp
    Source/CommonFramework/AudioPipeline/AudioSession.h
    Source/CommonFramework/AudioPipeline/AudioStream.cpp
    Source/CommonFramework/AudioPipeline/AudioStream.h
    Source/CommonFramework/AudioPipeline/AudioTemplate.cpp
    Source/CommonFramework/AudioPipeline/AudioTemplate.h
    Source/CommonFramework/AudioPipeline/Backends/AudioPassthroughPairQt.cpp
    Source/CommonFramework/AudioPipeline/Backends/AudioPassthroughPairQt.h
    Source/CommonFramework/AudioPipeline/Backends/AudioPassthroughPairQtThread.cpp
    Source/CommonFramework/AudioPipeline/Backends/AudioPassthroughPairQtThread.h
    Source/CommonFramework/AudioPipeline/IO/AudioFileLoader.cpp
    Source/CommonFramework/AudioPipeline/IO/AudioFileLoader.h
    Source/CommonFramework/AudioPipeline/IO/AudioSink.cpp
    Source/CommonFramework/AudioPipeline/IO/AudioSink.h
    Source/CommonFramework/AudioPipeline/IO/AudioSource.cpp
    Source/CommonFramework/AudioPipeline/IO/AudioSource.h
    Source/CommonFramework/AudioPipeline/Spectrum/AudioSpectrumHolder.cpp
    Source/CommonFramework/AudioPipeline/Spectrum/AudioSpectrumHolder.h
    Source/CommonFramework/AudioPipeline/Spectrum/FFTStreamer.cpp
    Source/CommonFramework/AudioPipeline/Spectrum/FFTStreamer.h
    Source/CommonFramework/AudioPipeline/Spectrum/Spectrograph.cpp
    Source/CommonFramework/AudioPipeline/Spectrum/Spectrograph.h
    Source/CommonFramework/AudioPipeline/Tools/AudioFormatUtils.cpp
    Source/CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h
    Source/CommonFramework/AudioPipeline/Tools/AudioNormalization.h
    Source/CommonFramework/AudioPipeline/Tools/TimeSampleBuffer.cpp
    Source/CommonFramework/AudioPipeline/Tools/TimeSampleBuffer.h
    Source/CommonFramework/AudioPipeline/Tools/TimeSampleBufferReader.cpp
    Source/CommonFramework/AudioPipeline/Tools/TimeSampleBufferReader.h
    Source/CommonFramework/AudioPipeline/Tools/TimeSampleWriter.h
    Source/CommonFramework/AudioPipeline/UI/AudioDisplayWidget.cpp
    Source/CommonFramework/AudioPipeline/UI/AudioDisplayWidget.h
    Source/CommonFramework/AudioPipeline/UI/AudioSelectorWidget.cpp
    Source/CommonFramework/AudioPipeline/UI/AudioSelectorWidget.h
    Source/CommonFramework/Environment/Environment.cpp
    Source/CommonFramework/Environment/Environment.h
    Source/CommonFramework/Environment/Environment_Linux.h
    Source/CommonFramework/Environment/Environment_Linux.tpp
    Source/CommonFramework/Environment/Environment_Windows.h
    Source/CommonFramework/Environment/Environment_Windows.tpp
    Source/CommonFramework/Environment/Environment_arm64_Linux.tpp
    Source/CommonFramework/Environment/Environment_x86.tpp
    Source/CommonFramework/Environment/Environment_x86_Linux.tpp
    Source/CommonFramework/Environment/Environment_x86_Windows.tpp
    Source/CommonFramework/Environment/HardwareValidation.cpp
    Source/CommonFramework/Environment/HardwareValidation.h
    Source/CommonFramework/Environment/HardwareValidation_arm64.tpp
    Source/CommonFramework/Environment/HardwareValidation_x86.tpp
    Source/CommonFramework/Environment/SystemSleep.cpp
    Source/CommonFramework/Environment/SystemSleep.h
    Source/CommonFramework/Environment/SystemSleep_Apple.tpp
    Source/CommonFramework/Environment/SystemSleep_Windows.tpp
    Source/CommonFramework/ErrorReports/ErrorReports.cpp
    Source/CommonFramework/ErrorReports/ErrorReports.h
    Source/CommonFramework/ErrorReports/ProgramDumper.cpp
    Source/CommonFramework/ErrorReports/ProgramDumper.h
    Source/CommonFramework/ErrorReports/ProgramDumper_Windows.tpp
    Source/CommonFramework/Exceptions/FatalProgramException.h
    Source/CommonFramework/Exceptions/OperationFailedException.h
    Source/CommonFramework/Exceptions/ProgramFinishedException.cpp
    Source/CommonFramework/Exceptions/ProgramFinishedException.h
    Source/CommonFramework/Exceptions/ScreenshotException.cpp
    Source/CommonFramework/Exceptions/ScreenshotException.h
    Source/CommonFramework/Exceptions/UnexpectedBattleException.h
    Source/CommonFramework/GlobalServices.cpp
    Source/CommonFramework/GlobalServices.h
    Source/CommonFramework/GlobalSettingsPanel.cpp
    Source/CommonFramework/GlobalSettingsPanel.h
    Source/CommonFramework/Globals.cpp
    Source/CommonFramework/Globals.h
    Source/CommonFramework/ImageTools/FloatPixel.cpp
    Source/CommonFramework/ImageTools/FloatPixel.h
    Source/CommonFramework/ImageTools/ImageBoxes.cpp
    Source/CommonFramework/ImageTools/ImageBoxes.h
    Source/CommonFramework/ImageTools/ImageDiff.cpp
    Source/CommonFramework/ImageTools/ImageDiff.h
    Source/CommonFramework/ImageTools/ImageStats.cpp
    Source/CommonFramework/ImageTools/ImageStats.h
    Source/CommonFramework/ImageTypes/BinaryImage.cpp
    Source/CommonFramework/ImageTypes/BinaryImage.h
    Source/CommonFramework/ImageTypes/ImageHSV32.cpp
    Source/CommonFramework/ImageTypes/ImageHSV32.h
    Source/CommonFramework/ImageTypes/ImageRGB32.cpp
    Source/CommonFramework/ImageTypes/ImageRGB32.h
    Source/CommonFramework/ImageTypes/ImageViewHSV32.cpp
    Source/CommonFramework/ImageTypes/ImageViewHSV32.h
    Source/CommonFramework/ImageTypes/ImageViewPlanar32.cpp
    Source/CommonFramework/ImageTypes/ImageViewPlanar32.h
    Source/CommonFramework/ImageTypes/ImageViewRGB32.cpp
    Source/CommonFramework/ImageTypes/ImageViewRGB32.h
    Source/CommonFramework/Language.cpp
    Source/CommonFramework/Language.h
    Source/CommonFramework/Logging/FileWindowLogger.cpp
    Source/CommonFramework/Logging/FileWindowLogger.h
    Source/CommonFramework/Logging/Logger.cpp
    Source/CommonFramework/Logging/Logger.h
    Source/CommonFramework/Logging/OutputRedirector.cpp
    Source/CommonFramework/Logging/OutputRedirector.h
    Source/CommonFramework/Logging/QueuedLogger.cpp
    Source/CommonFramework/Logging/QueuedLogger.h
    Source/CommonFramework/Main.cpp
    Source/CommonFramework/Notifications/EventNotificationOption.cpp
    Source/CommonFramework/Notifications/EventNotificationOption.h
    Source/CommonFramework/Notifications/EventNotificationsTable.cpp
    Source/CommonFramework/Notifications/EventNotificationsTable.h
    Source/CommonFramework/Notifications/MessageAttachment.cpp
    Source/CommonFramework/Notifications/MessageAttachment.h
    Source/CommonFramework/Notifications/ProgramInfo.h
    Source/CommonFramework/Notifications/ProgramNotifications.cpp
    Source/CommonFramework/Notifications/ProgramNotifications.h
    Source/CommonFramework/Notifications/SenderNotificationTable.cpp
    Source/CommonFramework/Notifications/SenderNotificationTable.h
    Source/CommonFramework/Options/CheckForUpdatesOption.h
    Source/CommonFramework/Options/Environment/PerformanceOptions.h
    Source/CommonFramework/Options/Environment/ProcessPriorityOption.h
    Source/CommonFramework/Options/Environment/ProcessorLevelOption.cpp
    Source/CommonFramework/Options/Environment/ProcessorLevelOption.h
    Source/CommonFramework/Options/Environment/SleepSuppressOption.cpp
    Source/CommonFramework/Options/Environment/SleepSuppressOption.h
    Source/CommonFramework/Options/Environment/ThemeSelectorOption.cpp
    Source/CommonFramework/Options/Environment/ThemeSelectorOption.h
    Source/CommonFramework/Options/LabelCellOption.cpp
    Source/CommonFramework/Options/LabelCellOption.h
    Source/CommonFramework/Options/QtWidget/LabelCellWidget.cpp
    Source/CommonFramework/Options/QtWidget/LabelCellWidget.h
    Source/CommonFramework/Options/ResolutionOption.cpp
    Source/CommonFramework/Options/ResolutionOption.h
    Source/CommonFramework/Options/ScreenshotFormatOption.h
    Source/CommonFramework/Options/ThreadPoolOption.cpp
    Source/CommonFramework/Options/ThreadPoolOption.h
    Source/CommonFramework/Panels/PanelDescriptor.cpp
    Source/CommonFramework/Panels/PanelDescriptor.h
    Source/CommonFramework/Panels/PanelInstance.cpp
    Source/CommonFramework/Panels/PanelInstance.h
    Source/CommonFramework/Panels/PanelList.cpp
    Source/CommonFramework/Panels/PanelList.h
    Source/CommonFramework/Panels/PanelTools.h
    Source/CommonFramework/Panels/ProgramDescriptor.cpp
    Source/CommonFramework/Panels/ProgramDescriptor.h
    Source/CommonFramework/Panels/SettingsPanel.cpp
    Source/CommonFramework/Panels/SettingsPanel.h
    Source/CommonFramework/Panels/UI/PanelElements.cpp
    Source/CommonFramework/Panels/UI/PanelElements.h
    Source/CommonFramework/Panels/UI/PanelListWidget.cpp
    Source/CommonFramework/Panels/UI/PanelListWidget.h
    Source/CommonFramework/Panels/UI/PanelWidget.cpp
    Source/CommonFramework/Panels/UI/PanelWidget.h
    Source/CommonFramework/Panels/UI/SettingsPanelWidget.cpp
    Source/CommonFramework/Panels/UI/SettingsPanelWidget.h
    Source/CommonFramework/PersistentSettings.cpp
    Source/CommonFramework/PersistentSettings.h
    Source/CommonFramework/ProgramSession.cpp
    Source/CommonFramework/ProgramSession.h
    Source/CommonFramework/ProgramStats/StatsDatabase.cpp
    Source/CommonFramework/ProgramStats/StatsDatabase.h
    Source/CommonFramework/ProgramStats/StatsTracking.cpp
    Source/CommonFramework/ProgramStats/StatsTracking.h
    Source/CommonFramework/Recording/StreamHistoryOption.cpp
    Source/CommonFramework/Recording/StreamHistoryOption.h
    Source/CommonFramework/Recording/StreamHistorySession.cpp
    Source/CommonFramework/Recording/StreamHistorySession.h
    Source/CommonFramework/Recording/StreamHistoryTracker_Null.h
    Source/CommonFramework/Recording/StreamHistoryTracker_ParallelStreams.h
    Source/CommonFramework/Recording/StreamHistoryTracker_RecordOnTheFly.h
    Source/CommonFramework/Recording/StreamHistoryTracker_SaveFrames.h
    Source/CommonFramework/Recording/StreamRecorder.cpp
    Source/CommonFramework/Recording/StreamRecorder.h
    Source/CommonFramework/Startup/NewVersionCheck.cpp
    Source/CommonFramework/Startup/NewVersionCheck.h
    Source/CommonFramework/Startup/SetupSettings.cpp
    Source/CommonFramework/Startup/SetupSettings.h
    Source/CommonFramework/Tools/DebugDumper.cpp
    Source/CommonFramework/Tools/DebugDumper.h
    Source/CommonFramework/Tools/ErrorDumper.cpp
    Source/CommonFramework/Tools/ErrorDumper.h
    Source/CommonFramework/Tools/FileDownloader.cpp
    Source/CommonFramework/Tools/FileDownloader.h
    Source/CommonFramework/Tools/GlobalThreadPools.cpp
    Source/CommonFramework/Tools/GlobalThreadPools.h
    Source/CommonFramework/Tools/ProgramEnvironment.cpp
    Source/CommonFramework/Tools/ProgramEnvironment.h
    Source/CommonFramework/Tools/StatAccumulator.cpp
    Source/CommonFramework/Tools/StatAccumulator.h
    Source/CommonFramework/Tools/VideoStream.cpp
    Source/CommonFramework/Tools/VideoStream.h
    Source/CommonFramework/VideoPipeline/Backends/CameraImplementations.cpp
    Source/CommonFramework/VideoPipeline/Backends/CameraImplementations.h
    Source/CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.5.cpp
    Source/CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.5.h
    Source/CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.cpp
    Source/CommonFramework/VideoPipeline/Backends/CameraWidgetQt6.h
    Source/CommonFramework/VideoPipeline/Backends/MediaServicesQt6.cpp
    Source/CommonFramework/VideoPipeline/Backends/MediaServicesQt6.h
    Source/CommonFramework/VideoPipeline/Backends/QCameraThread.h
    Source/CommonFramework/VideoPipeline/Backends/QVideoFrameCache.h
    Source/CommonFramework/VideoPipeline/Backends/SnapshotManager.cpp
    Source/CommonFramework/VideoPipeline/Backends/SnapshotManager.h
    Source/CommonFramework/VideoPipeline/Backends/VideoFrameQt.h
    Source/CommonFramework/VideoPipeline/CameraInfo.h
    Source/CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.cpp
    Source/CommonFramework/VideoPipeline/Stats/CpuUtilizationStats.h
    Source/CommonFramework/VideoPipeline/Stats/MemoryUtilizationStats.cpp
    Source/CommonFramework/VideoPipeline/Stats/MemoryUtilizationStats.h
    Source/CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.cpp
    Source/CommonFramework/VideoPipeline/Stats/ThreadUtilizationStats.h
    Source/CommonFramework/VideoPipeline/UI/VideoDisplayWidget.cpp
    Source/CommonFramework/VideoPipeline/UI/VideoDisplayWidget.h
    Source/CommonFramework/VideoPipeline/UI/VideoDisplayWindow.cpp
    Source/CommonFramework/VideoPipeline/UI/VideoDisplayWindow.h
    Source/CommonFramework/VideoPipeline/UI/VideoOverlayWidget.cpp
    Source/CommonFramework/VideoPipeline/UI/VideoOverlayWidget.h
    Source/CommonFramework/VideoPipeline/UI/VideoSourceSelectorWidget.cpp
    Source/CommonFramework/VideoPipeline/UI/VideoSourceSelectorWidget.h
    Source/CommonFramework/VideoPipeline/VideoFeed.h
    Source/CommonFramework/VideoPipeline/VideoOverlay.cpp
    Source/CommonFramework/VideoPipeline/VideoOverlay.h
    Source/CommonFramework/VideoPipeline/VideoOverlayOption.cpp
    Source/CommonFramework/VideoPipeline/VideoOverlayOption.h
    Source/CommonFramework/VideoPipeline/VideoOverlayScopes.h
    Source/CommonFramework/VideoPipeline/VideoOverlaySession.cpp
    Source/CommonFramework/VideoPipeline/VideoOverlaySession.h
    Source/CommonFramework/VideoPipeline/VideoOverlayTypes.cpp
    Source/CommonFramework/VideoPipeline/VideoOverlayTypes.h
    Source/CommonFramework/VideoPipeline/VideoPipelineOptions.h
    Source/CommonFramework/VideoPipeline/VideoSession.cpp
    Source/CommonFramework/VideoPipeline/VideoSession.h
    Source/CommonFramework/VideoPipeline/VideoSource.cpp
    Source/CommonFramework/VideoPipeline/VideoSource.h
    Source/CommonFramework/VideoPipeline/VideoSourceDescriptor.cpp
    Source/CommonFramework/VideoPipeline/VideoSourceDescriptor.h
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_Camera.cpp
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_Camera.h
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_Null.cpp
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_Null.h
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.cpp
    Source/CommonFramework/VideoPipeline/VideoSources/VideoSource_StillImage.h
    Source/CommonFramework/Windows/ButtonDiagram.cpp
    Source/CommonFramework/Windows/ButtonDiagram.h
    Source/CommonFramework/Windows/DpiScaler.cpp
    Source/CommonFramework/Windows/DpiScaler.h
    Source/CommonFramework/Windows/MainWindow.cpp
    Source/CommonFramework/Windows/MainWindow.h
    Source/CommonFramework/Windows/WindowTracker.cpp
    Source/CommonFramework/Windows/WindowTracker.h
    Source/CommonTools/Async/InferenceRoutines.cpp
    Source/CommonTools/Async/InferenceRoutines.h
    Source/CommonTools/Async/InferenceSession.cpp
    Source/CommonTools/Async/InferenceSession.h
    Source/CommonTools/Async/InterruptableCommands.h
    Source/CommonTools/Async/InterruptableCommands.tpp
    Source/CommonTools/Async/SuperControlSession.h
    Source/CommonTools/Async/SuperControlSession.tpp
    Source/CommonTools/Audio/AudioPerSpectrumDetectorBase.cpp
    Source/CommonTools/Audio/AudioPerSpectrumDetectorBase.h
    Source/CommonTools/Audio/AudioTemplateCache.cpp
    Source/CommonTools/Audio/AudioTemplateCache.h
    Source/CommonTools/Audio/SpectrogramMatcher.cpp
    Source/CommonTools/Audio/SpectrogramMatcher.h
    Source/CommonTools/DetectionDebouncer.h
    Source/CommonTools/FailureWatchdog.h
    Source/CommonTools/ImageMatch/CroppedImageDictionaryMatcher.cpp
    Source/CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h
    Source/CommonTools/ImageMatch/ExactImageDictionaryMatcher.cpp
    Source/CommonTools/ImageMatch/ExactImageDictionaryMatcher.h
    Source/CommonTools/ImageMatch/ExactImageMatcher.cpp
    Source/CommonTools/ImageMatch/ExactImageMatcher.h
    Source/CommonTools/ImageMatch/FilterToAlpha.cpp
    Source/CommonTools/ImageMatch/FilterToAlpha.h
    Source/CommonTools/ImageMatch/ImageCropper.cpp
    Source/CommonTools/ImageMatch/ImageCropper.h
    Source/CommonTools/ImageMatch/ImageMatchOption.cpp
    Source/CommonTools/ImageMatch/ImageMatchOption.h
    Source/CommonTools/ImageMatch/ImageMatchResult.cpp
    Source/CommonTools/ImageMatch/ImageMatchResult.h
    Source/CommonTools/ImageMatch/SilhouetteDictionaryMatcher.cpp
    Source/CommonTools/ImageMatch/SilhouetteDictionaryMatcher.h
    Source/CommonTools/ImageMatch/SubObjectTemplateMatcher.cpp
    Source/CommonTools/ImageMatch/SubObjectTemplateMatcher.h
    Source/CommonTools/ImageMatch/WaterfillTemplateMatcher.cpp
    Source/CommonTools/ImageMatch/WaterfillTemplateMatcher.h
    Source/CommonTools/Images/BinaryImage_FilterRgb32.cpp
    Source/CommonTools/Images/BinaryImage_FilterRgb32.h
    Source/CommonTools/Images/ColorClustering.cpp
    Source/CommonTools/Images/ColorClustering.h
    Source/CommonTools/Images/DistanceToLine.h
    Source/CommonTools/Images/ImageFilter.cpp
    Source/CommonTools/Images/ImageFilter.h
    Source/CommonTools/Images/ImageGradient.cpp
    Source/CommonTools/Images/ImageGradient.h
    Source/CommonTools/Images/ImageManip.cpp
    Source/CommonTools/Images/ImageManip.h
    Source/CommonTools/Images/ImageTools.cpp
    Source/CommonTools/Images/ImageTools.h
    Source/CommonTools/Images/SolidColorTest.cpp
    Source/CommonTools/Images/SolidColorTest.h
    Source/CommonTools/Images/WaterfillUtilities.cpp
    Source/CommonTools/Images/WaterfillUtilities.h
    Source/CommonTools/InferenceCallbacks/AudioInferenceCallback.h
    Source/CommonTools/InferenceCallbacks/InferenceCallback.h
    Source/CommonTools/InferenceCallbacks/VisualInferenceCallback.cpp
    Source/CommonTools/InferenceCallbacks/VisualInferenceCallback.h
    Source/CommonTools/InferencePivots/AudioInferencePivot.cpp
    Source/CommonTools/InferencePivots/AudioInferencePivot.h
    Source/CommonTools/InferencePivots/VisualInferencePivot.cpp
    Source/CommonTools/InferencePivots/VisualInferencePivot.h
    Source/CommonTools/InferenceThrottler.h
    Source/CommonTools/MultiConsoleErrors.cpp
    Source/CommonTools/MultiConsoleErrors.h
    Source/CommonTools/OCR/OCR_DictionaryMatcher.cpp
    Source/CommonTools/OCR/OCR_DictionaryMatcher.h
    Source/CommonTools/OCR/OCR_DictionaryOCR.cpp
    Source/CommonTools/OCR/OCR_DictionaryOCR.h
    Source/CommonTools/OCR/OCR_LargeDictionaryMatcher.cpp
    Source/CommonTools/OCR/OCR_LargeDictionaryMatcher.h
    Source/CommonTools/OCR/OCR_NumberReader.cpp
    Source/CommonTools/OCR/OCR_NumberReader.h
    Source/CommonTools/OCR/OCR_RawOCR.cpp
    Source/CommonTools/OCR/OCR_RawOCR.h
    Source/CommonTools/OCR/OCR_Routines.cpp
    Source/CommonTools/OCR/OCR_Routines.h
    Source/CommonTools/OCR/OCR_SmallDictionaryMatcher.cpp
    Source/CommonTools/OCR/OCR_SmallDictionaryMatcher.h
    Source/CommonTools/OCR/OCR_StringMatchResult.cpp
    Source/CommonTools/OCR/OCR_StringMatchResult.h
    Source/CommonTools/OCR/OCR_StringNormalization.cpp
    Source/CommonTools/OCR/OCR_StringNormalization.h
    Source/CommonTools/OCR/OCR_TextMatcher.cpp
    Source/CommonTools/OCR/OCR_TextMatcher.h
    Source/CommonTools/OCR/OCR_TrainingTools.cpp
    Source/CommonTools/OCR/OCR_TrainingTools.h
    Source/CommonTools/Options/LanguageOCROption.cpp
    Source/CommonTools/Options/LanguageOCROption.h
    Source/CommonTools/Options/QtWidgets/LanguageOCRWidget.cpp
    Source/CommonTools/Options/QtWidgets/LanguageOCRWidget.h
    Source/CommonTools/Options/QtWidgets/ScreenWatchWidget.cpp
    Source/CommonTools/Options/QtWidgets/ScreenWatchWidget.h
    Source/CommonTools/Options/QtWidgets/StringSelectWidget.cpp
    Source/CommonTools/Options/QtWidgets/StringSelectWidget.h
    Source/CommonTools/Options/ScreenWatchOption.cpp
    Source/CommonTools/Options/ScreenWatchOption.h
    Source/CommonTools/Options/StringSelectOption.cpp
    Source/CommonTools/Options/StringSelectOption.h
    Source/CommonTools/Options/StringSelectTableOption.h
    Source/CommonTools/Options/TrainOCRModeOption.h
    Source/CommonTools/Resources/SpriteDatabase.cpp
    Source/CommonTools/Resources/SpriteDatabase.h
    Source/CommonTools/StartupChecks/StartProgramChecks.cpp
    Source/CommonTools/StartupChecks/StartProgramChecks.h
    Source/CommonTools/StartupChecks/VideoResolutionCheck.cpp
    Source/CommonTools/StartupChecks/VideoResolutionCheck.h
    Source/CommonTools/TrendInference/AnomalyDetector.cpp
    Source/CommonTools/TrendInference/AnomalyDetector.h
    Source/CommonTools/TrendInference/TimeWindowStatTracker.h
    Source/CommonTools/VisualDetector.h
    Source/CommonTools/VisualDetectors/BlackBorderDetector.cpp
    Source/CommonTools/VisualDetectors/BlackBorderDetector.h
    Source/CommonTools/VisualDetectors/BlackScreenDetector.cpp
    Source/CommonTools/VisualDetectors/BlackScreenDetector.h
    Source/CommonTools/VisualDetectors/FrozenImageDetector.cpp
    Source/CommonTools/VisualDetectors/FrozenImageDetector.h
    Source/CommonTools/VisualDetectors/ImageMatchDetector.cpp
    Source/CommonTools/VisualDetectors/ImageMatchDetector.h
    Source/ComputerPrograms/ComputerProgram.cpp
    Source/ComputerPrograms/ComputerProgram.h
    Source/ComputerPrograms/Framework/ComputerProgramOption.cpp
    Source/ComputerPrograms/Framework/ComputerProgramOption.h
    Source/ComputerPrograms/Framework/ComputerProgramSession.cpp
    Source/ComputerPrograms/Framework/ComputerProgramSession.h
    Source/ComputerPrograms/Framework/ComputerProgramWidget.cpp
    Source/ComputerPrograms/Framework/ComputerProgramWidget.h
    Source/Controllers/Controller.cpp
    Source/Controllers/Controller.h
    Source/Controllers/ControllerConnection.cpp
    Source/Controllers/ControllerConnection.h
    Source/Controllers/ControllerDescriptor.cpp
    Source/Controllers/ControllerDescriptor.h
    Source/Controllers/ControllerSelectorWidget.cpp
    Source/Controllers/ControllerSelectorWidget.h
    Source/Controllers/ControllerSession.cpp
    Source/Controllers/ControllerSession.h
    Source/Controllers/ControllerTypeStrings.cpp
    Source/Controllers/ControllerTypeStrings.h
    Source/Controllers/ControllerTypes.h
    Source/Controllers/JoystickTools.h
    Source/Controllers/KeyboardInput/GlobalQtKeyMap.cpp
    Source/Controllers/KeyboardInput/GlobalQtKeyMap.h
    Source/Controllers/KeyboardInput/KeyboardEventHandler.cpp
    Source/Controllers/KeyboardInput/KeyboardEventHandler.h
    Source/Controllers/KeyboardInput/KeyboardInput.cpp
    Source/Controllers/KeyboardInput/KeyboardInput.h
    Source/Controllers/KeyboardInput/KeyboardStateTracker.cpp
    Source/Controllers/KeyboardInput/KeyboardStateTracker.h
    Source/Controllers/NullController.cpp
    Source/Controllers/NullController.h
    Source/Controllers/Schedulers/ControllerWithScheduler.h
    Source/Controllers/Schedulers/SuperscalarScheduler.cpp
    Source/Controllers/Schedulers/SuperscalarScheduler.h
    Source/Controllers/SerialPABotBase/SerialPABotBase.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Connection.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Connection.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Descriptor.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Descriptor.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_HID_Keyboard.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_HID_Keyboard.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_NS1_WirelessControllers.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_NS1_WirelessControllers.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_NS2_WiredController.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_NS2_WiredController.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.cpp
    Source/Controllers/SerialPABotBase/SerialPABotBase_Routines_Protocol.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_SelectorWidget.h
    Source/Controllers/SerialPABotBase/SerialPABotBase_StatusThread.h
    Source/Controllers/StandardHid/StandardHid_Keyboard.cpp
    Source/Controllers/StandardHid/StandardHid_Keyboard.h
    Source/Controllers/StandardHid/StandardHid_Keyboard_ControllerButtons.h
    Source/Controllers/StandardHid/StandardHid_Keyboard_KeyMappings.cpp
    Source/Controllers/StandardHid/StandardHid_Keyboard_KeyMappings.h
    Source/Controllers/StandardHid/StandardHid_Keyboard_SerialPABotBase.cpp
    Source/Controllers/StandardHid/StandardHid_Keyboard_SerialPABotBase.h
    Source/Controllers/StandardHid/StandardHid_KeyboardWithScheduler.cpp
    Source/Controllers/StandardHid/StandardHid_KeyboardWithScheduler.h
    Source/Integrations/DiscordIntegrationSettings.cpp
    Source/Integrations/DiscordIntegrationSettings.h
    Source/Integrations/DiscordIntegrationTable.cpp
    Source/Integrations/DiscordIntegrationTable.h
    Source/Integrations/DiscordSettingsOption.cpp
    Source/Integrations/DiscordSettingsOption.h
    Source/Integrations/DiscordSocial/DiscordSocial.cpp
    Source/Integrations/DiscordSocial/DiscordSocial.h
    Source/Integrations/DiscordWebhook.cpp
    Source/Integrations/DiscordWebhook.h
    Source/Integrations/DiscordWebhookSettings.cpp
    Source/Integrations/DiscordWebhookSettings.h
    Source/Integrations/DppIntegration/DppClient.cpp
    Source/Integrations/DppIntegration/DppClient.h
    Source/Integrations/DppIntegration/DppCommandHandler.cpp
    Source/Integrations/DppIntegration/DppCommandHandler.h
    Source/Integrations/DppIntegration/DppUtility.cpp
    Source/Integrations/DppIntegration/DppUtility.h
    Source/Integrations/IntegrationsAPI.cpp
    Source/Integrations/IntegrationsAPI.h
    Source/Integrations/ProgramTracker.cpp
    Source/Integrations/ProgramTracker.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Arch.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Arch_Default.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Arch_x86_AVX2.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Arch_x86_SSE41.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_BaseTransform_x86_AVX2.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_BaseTransform_x86_SSE41.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_BitReverse.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Butterflies.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_ComplexScalar.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_ComplexToAbs.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_ComplexVector.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_Default.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_AVX2.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_SSE41.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_FullTransform.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_FullTransform.tpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Reductions.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_TwiddleTable.h
    Source/Kernels/AbsFFT/Kernels_AbsFFT_TwiddleTable.tpp
    Source/Kernels/Algorithm/Kernels_Algorithm_DisjointSet.cpp
    Source/Kernels/Algorithm/Kernels_Algorithm_DisjointSet.h
    Source/Kernels/AudioStreamConversion/AudioStreamConversion.cpp
    Source/Kernels/AudioStreamConversion/AudioStreamConversion.h
    Source/Kernels/AudioStreamConversion/AudioStreamConversion_Core_Default.cpp
    Source/Kernels/AudioStreamConversion/AudioStreamConversion_Core_x86_SSE41.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x16_x64_AVX2.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x4_Default.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x64_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x8_arm64_NEON.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x8_x64_SSE42.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Default.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Routines.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_arm64_NEON.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX2.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX512.h
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h
    Source/Kernels/BinaryImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x16_x64_AVX2.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x32_x64_AVX512.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x4_Default.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x64_x64_AVX512.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x8_arm64_NEON.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x8_x64_SSE42.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64xH_Default.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_Debugging.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x16_x64_AVX2.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x32_x64_AVX512.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x4_Default.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x64_x64_AVX512.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_arm64_NEON.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_x64_SSE42.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x16_x64_AVX2.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x64_x64_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x8_x64_SSE42.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64xH_Default.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_arm64_NEON.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_t.h
    Source/Kernels/BinaryMatrix/Kernels_PackedBinaryMatrix.h
    Source/Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h
    Source/Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.tpp
    Source/Kernels/BinaryMatrix/Kernels_SparseBinaryMatrixCore.h
    Source/Kernels/BinaryMatrix/Kernels_SparseBinaryMatrixCore.tpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_ARM64_NEON.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Default.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_ARM64_NEON.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_Default.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX2.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX512.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_SSE42.h
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX2.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX512.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_SSE42.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Green_Default.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness.h
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_Default.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX512-VNNI.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean.h
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_ARM64_NEON.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_Default.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range.h
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_ARM64_NEON.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_Default.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_SSE42.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_Default.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_arm64_NEON.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX2.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX512.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr.h
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev.h
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_Default.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_Default.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_SSE41.cpp
    Source/Kernels/Kernels_Alignment.h
    Source/Kernels/Kernels_BitScan.h
    Source/Kernels/Kernels_BitSet.h
    Source/Kernels/Kernels_arm64_NEON.h
    Source/Kernels/Kernels_x64_AVX2.h
    Source/Kernels/Kernels_x64_AVX512.h
    Source/Kernels/Kernels_x64_SSE41.h
    Source/Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h
    Source/Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h
    Source/Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_Default.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX2.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX512.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_SSE.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Routines.h
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution.h
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_Default.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX2.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX512.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_SSE41.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Routines.h
    Source/Kernels/Waterfill/Kernels_Waterfill.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x16_x64_AVX2.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x16_x64_AVX2.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x4_Default.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512-GF.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512-GF.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_arm64_NEON.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_arm64_NEON.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64xH_Default.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64xH_Default.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512-GF.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Routines.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Session.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Session.h
    Source/Kernels/Waterfill/Kernels_Waterfill_Session.tpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Types.h
    Source/ML/DataLabeling/ML_AnnotationIO.cpp
    Source/ML/DataLabeling/ML_AnnotationIO.h
    Source/ML/DataLabeling/ML_ObjectAnnotation.cpp
    Source/ML/DataLabeling/ML_ObjectAnnotation.h
    Source/ML/DataLabeling/ML_SegmentAnythingModel.cpp
    Source/ML/DataLabeling/ML_SegmentAnythingModel.h
    Source/ML/DataLabeling/ML_SegmentAnythingModelConstants.h
    Source/ML/Inference/ML_YOLOv5Detector.cpp
    Source/ML/Inference/ML_YOLOv5Detector.h
    Source/ML/ML_Panels.cpp
    Source/ML/ML_Panels.h
    Source/ML/Models/ML_ONNXRuntimeHelpers.cpp
    Source/ML/Models/ML_ONNXRuntimeHelpers.h
    Source/ML/Models/ML_YOLOv5Model.cpp
    Source/ML/Models/ML_YOLOv5Model.h
    Source/ML/Programs/ML_LabelImages.cpp
    Source/ML/Programs/ML_LabelImages.h
    Source/ML/Programs/ML_LabelImagesOverlayManager.cpp
    Source/ML/Programs/ML_LabelImagesOverlayManager.h
    Source/ML/Programs/ML_LabelImagesWidget.cpp
    Source/ML/Programs/ML_LabelImagesWidget.h
    Source/ML/Programs/ML_RunYOLO.cpp
    Source/ML/Programs/ML_RunYOLO.h
    Source/ML/UI/ML_ImageAnnotationCommandRow.cpp
    Source/ML/UI/ML_ImageAnnotationCommandRow.h
    Source/ML/UI/ML_ImageAnnotationDisplayOption.cpp
    Source/ML/UI/ML_ImageAnnotationDisplayOption.h
    Source/ML/UI/ML_ImageAnnotationDisplaySession.cpp
    Source/ML/UI/ML_ImageAnnotationDisplaySession.h
    Source/ML/UI/ML_ImageAnnotationDisplayWidget.cpp
    Source/ML/UI/ML_ImageAnnotationDisplayWidget.h
    Source/ML/UI/ML_ImageAnnotationSourceSelectorWidget.cpp
    Source/ML/UI/ML_ImageAnnotationSourceSelectorWidget.h
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.cpp
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.cpp
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_Routines.h
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.cpp
    Source/NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerSettings.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerSettings.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_ControllerWithScheduler.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_Joycon.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_Joycon.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_KeyboardMapping.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_KeyboardMapping.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_ProController.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_ProController.h
    Source/NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.cpp
    Source/NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_Controller.cpp
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_Controller.h
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.cpp
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessController.cpp
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessController.h
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessJoycon.cpp
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessJoycon.h
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessProController.cpp
    Source/NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WirelessProController.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase3_ControllerState.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase3_ProController.cpp
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase3_ProController.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_Connection.cpp
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_Connection.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_Descriptor.cpp
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_Descriptor.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_ProController.cpp
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_ProController.h
    Source/NintendoSwitch/Controllers/SysbotBase/SysbotBase_SelectorWidget.h
    Source/NintendoSwitch/DevPrograms/BoxDraw.cpp
    Source/NintendoSwitch/DevPrograms/BoxDraw.h
    Source/NintendoSwitch/DevPrograms/JoyconProgram.cpp
    Source/NintendoSwitch/DevPrograms/JoyconProgram.h
    Source/NintendoSwitch/DevPrograms/TestDudunsparceFormDetector.cpp
    Source/NintendoSwitch/DevPrograms/TestDudunsparceFormDetector.h
    Source/NintendoSwitch/DevPrograms/TestProgramComputer.cpp
    Source/NintendoSwitch/DevPrograms/TestProgramComputer.h
    Source/NintendoSwitch/DevPrograms/TestProgramSwitch.cpp
    Source/NintendoSwitch/DevPrograms/TestProgramSwitch.h
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramOption.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramOption.h
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramSession.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramSession.h
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemOption.h
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_MultiSwitchSystemSession.h
    Source/NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramOption.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramOption.h
    Source/NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramSession.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramSession.h
    Source/NintendoSwitch/Framework/NintendoSwitch_SwitchSystemOption.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_SwitchSystemOption.h
    Source/NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.cpp
    Source/NintendoSwitch/Framework/NintendoSwitch_SwitchSystemSession.h
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_CommandRow.cpp
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_CommandRow.h
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_MultiSwitchProgramWidget.cpp
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_MultiSwitchProgramWidget.h
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_MultiSwitchSystemWidget.cpp
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_MultiSwitchSystemWidget.h
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_SingleSwitchProgramWidget.cpp
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_SingleSwitchProgramWidget.h
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.cpp
    Source/NintendoSwitch/Framework/UI/NintendoSwitch_SwitchSystemWidget.h
    Source/NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_DateChangeDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_DateChangeDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_DetectHome.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_DetectHome.h
    Source/NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.h
    Source/NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.cpp
    Source/NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h
    Source/NintendoSwitch/NintendoSwitch_ConsoleHandle.cpp
    Source/NintendoSwitch/NintendoSwitch_ConsoleHandle.h
    Source/NintendoSwitch/NintendoSwitch_ConsoleState.cpp
    Source/NintendoSwitch/NintendoSwitch_ConsoleState.h
    Source/NintendoSwitch/NintendoSwitch_MultiSwitchProgram.cpp
    Source/NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h
    Source/NintendoSwitch/NintendoSwitch_Panels.cpp
    Source/NintendoSwitch/NintendoSwitch_Panels.h
    Source/NintendoSwitch/NintendoSwitch_Settings.cpp
    Source/NintendoSwitch/NintendoSwitch_Settings.h
    Source/NintendoSwitch/NintendoSwitch_SingleSwitchProgram.cpp
    Source/NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h
    Source/NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.cpp
    Source/NintendoSwitch/Options/NintendoSwitch_CodeEntrySettingsOption.h
    Source/NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.cpp
    Source/NintendoSwitch/Options/NintendoSwitch_FriendCodeListOption.h
    Source/NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.cpp
    Source/NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h
    Source/NintendoSwitch/Options/NintendoSwitch_ModelType.h
    Source/NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h
    Source/NintendoSwitch/Options/TurboMacroTable.cpp
    Source/NintendoSwitch/Options/TurboMacroTable.h
    Source/NintendoSwitch/Options/UI/NintendoSwitch_FriendCodeListWidget.cpp
    Source/NintendoSwitch/Options/UI/NintendoSwitch_FriendCodeListWidget.h
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.cpp
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManipBase.h
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManipTools.cpp
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManipTools.h
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_24h.cpp
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_24h.h
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_US.cpp
    Source/NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_US.h
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch1_HomeToDateTime.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch2_HomeToDateTime.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_NeutralDateSkip.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_NeutralDateSkip.h
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateBackwardN.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateBackwardN.h
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.cpp
    Source/NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_CodeEntryTools.cpp
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_CodeEntryTools.h
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardCodeEntry.cpp
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardCodeEntry.h
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardEntryMappings.cpp
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardEntryMappings.h
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.cpp
    Source/NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h
    Source/NintendoSwitch/Programs/NintendoSwitch_FriendCodeAdder.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_FriendCodeAdder.h
    Source/NintendoSwitch/Programs/NintendoSwitch_FriendDelete.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_FriendDelete.h
    Source/NintendoSwitch/Programs/NintendoSwitch_GameEntry.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_GameEntry.h
    Source/NintendoSwitch/Programs/NintendoSwitch_MenuStabilityTester.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_MenuStabilityTester.h
    Source/NintendoSwitch/Programs/NintendoSwitch_PreventSleep.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_PreventSleep.h
    Source/NintendoSwitch/Programs/NintendoSwitch_PushJoySticks.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_PushJoySticks.h
    Source/NintendoSwitch/Programs/NintendoSwitch_RecordKeyboardController.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_RecordKeyboardController.h
    Source/NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_SnapshotDumper.h
    Source/NintendoSwitch/Programs/NintendoSwitch_SwitchViewer.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_SwitchViewer.h
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboA.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboA.h
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboButton.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboButton.h
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboMacro.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_TurboMacro.h
    Source/NintendoSwitch/Programs/NintendoSwitch_VirtualConsole.cpp
    Source/NintendoSwitch/Programs/NintendoSwitch_VirtualConsole.h
    Source/PanelLists.cpp
    Source/PanelLists.h
    Source/Pokemon/Inference/Pokemon_BerryNameReader.cpp
    Source/Pokemon/Inference/Pokemon_BerryNameReader.h
    Source/Pokemon/Inference/Pokemon_BoxGenderDetector.cpp
    Source/Pokemon/Inference/Pokemon_BoxGenderDetector.h
    Source/Pokemon/Inference/Pokemon_IvJudgeReader.cpp
    Source/Pokemon/Inference/Pokemon_IvJudgeReader.h
    Source/Pokemon/Inference/Pokemon_NameReader.cpp
    Source/Pokemon/Inference/Pokemon_NameReader.h
    Source/Pokemon/Inference/Pokemon_NatureReader.cpp
    Source/Pokemon/Inference/Pokemon_NatureReader.h
    Source/Pokemon/Inference/Pokemon_PokeballNameReader.cpp
    Source/Pokemon/Inference/Pokemon_PokeballNameReader.h
    Source/Pokemon/Inference/Pokemon_ReadHpBar.cpp
    Source/Pokemon/Inference/Pokemon_ReadHpBar.h
    Source/Pokemon/Inference/Pokemon_TrainIVCheckerOCR.cpp
    Source/Pokemon/Inference/Pokemon_TrainIVCheckerOCR.h
    Source/Pokemon/Inference/Pokemon_TrainPokemonOCR.cpp
    Source/Pokemon/Inference/Pokemon_TrainPokemonOCR.h
    Source/Pokemon/Options/Pokemon_EncounterBotOptions.h
    Source/Pokemon/Options/Pokemon_HomeSpriteSelectOption.cpp
    Source/Pokemon/Options/Pokemon_HomeSpriteSelectOption.h
    Source/Pokemon/Options/Pokemon_IvJudgeOption.cpp
    Source/Pokemon/Options/Pokemon_IvJudgeOption.h
    Source/Pokemon/Options/Pokemon_NameSelectOption.cpp
    Source/Pokemon/Options/Pokemon_NameSelectOption.h
    Source/Pokemon/Options/Pokemon_NameSelectWidget.cpp
    Source/Pokemon/Options/Pokemon_NameSelectWidget.h
    Source/Pokemon/Options/Pokemon_StatsHuntFilter.cpp
    Source/Pokemon/Options/Pokemon_StatsHuntFilter.h
    Source/Pokemon/Pokemon_DataTypes.h
    Source/Pokemon/Pokemon_EncounterStats.cpp
    Source/Pokemon/Pokemon_EncounterStats.h
    Source/Pokemon/Pokemon_IvJudge.cpp
    Source/Pokemon/Pokemon_IvJudge.h
    Source/Pokemon/Pokemon_NatureChecker.cpp
    Source/Pokemon/Pokemon_NatureChecker.h
    Source/Pokemon/Pokemon_Notification.cpp
    Source/Pokemon/Pokemon_Notification.h
    Source/Pokemon/Pokemon_ShinySparkleSet.cpp
    Source/Pokemon/Pokemon_ShinySparkleSet.h
    Source/Pokemon/Pokemon_StatsCalculation.cpp
    Source/Pokemon/Pokemon_StatsCalculation.h
    Source/Pokemon/Pokemon_Strings.cpp
    Source/Pokemon/Pokemon_Strings.h
    Source/Pokemon/Pokemon_Types.cpp
    Source/Pokemon/Pokemon_Types.h
    Source/Pokemon/Pokemon_Xoroshiro128Plus.cpp
    Source/Pokemon/Pokemon_Xoroshiro128Plus.h
    Source/Pokemon/Resources/Pokemon_BerryNames.cpp
    Source/Pokemon/Resources/Pokemon_BerryNames.h
    Source/Pokemon/Resources/Pokemon_BerrySprites.cpp
    Source/Pokemon/Resources/Pokemon_BerrySprites.h
    Source/Pokemon/Resources/Pokemon_EggSteps.cpp
    Source/Pokemon/Resources/Pokemon_EggSteps.h
    Source/Pokemon/Resources/Pokemon_PokeballNames.cpp
    Source/Pokemon/Resources/Pokemon_PokeballNames.h
    Source/Pokemon/Resources/Pokemon_PokemonForms.cpp
    Source/Pokemon/Resources/Pokemon_PokemonForms.h
    Source/Pokemon/Resources/Pokemon_PokemonHomeSprites.cpp
    Source/Pokemon/Resources/Pokemon_PokemonHomeSprites.h
    Source/Pokemon/Resources/Pokemon_PokemonNames.cpp
    Source/Pokemon/Resources/Pokemon_PokemonNames.h
    Source/Pokemon/Resources/Pokemon_PokemonSlugs.cpp
    Source/Pokemon/Resources/Pokemon_PokemonSlugs.h
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.cpp
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.cpp
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.cpp
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.cpp
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.cpp
    Source/PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.cpp
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.h
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.cpp
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxNatureDetector.cpp
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxNatureDetector.h
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxShinyDetector.cpp
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxShinyDetector.h
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IvJudgeReader.cpp
    Source/PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IvJudgeReader.h
    Source/PokemonBDSP/Inference/PokemonBDSP_DialogDetector.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h
    Source/PokemonBDSP/Inference/PokemonBDSP_MapDetector.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_MapDetector.h
    Source/PokemonBDSP/Inference/PokemonBDSP_MarkFinder.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h
    Source/PokemonBDSP/Inference/PokemonBDSP_MenuDetector.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h
    Source/PokemonBDSP/Inference/PokemonBDSP_PokeballSpriteMatcher.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_PokeballSpriteMatcher.h
    Source/PokemonBDSP/Inference/PokemonBDSP_ReceivePokemonDetector.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_ReceivePokemonDetector.h
    Source/PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h
    Source/PokemonBDSP/Inference/PokemonBDSP_VSSeekerReaction.cpp
    Source/PokemonBDSP/Inference/PokemonBDSP_VSSeekerReaction.h
    Source/PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.cpp
    Source/PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h
    Source/PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinySparkleSet.cpp
    Source/PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinySparkleSet.h
    Source/PokemonBDSP/Inference/Sounds/PokemonBDSP_ShinySoundDetector.cpp
    Source/PokemonBDSP/Inference/Sounds/PokemonBDSP_ShinySoundDetector.h
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterEnums.cpp
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterEnums.h
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterOption.cpp
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterOption.h
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterOverride.cpp
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterOverride.h
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterWidget.cpp
    Source/PokemonBDSP/Options/EncounterFilter/PokemonBDSP_EncounterFilterWidget.h
    Source/PokemonBDSP/Options/PokemonBDSP_BerrySelectOption.cpp
    Source/PokemonBDSP/Options/PokemonBDSP_BerrySelectOption.h
    Source/PokemonBDSP/Options/PokemonBDSP_BerryTable.cpp
    Source/PokemonBDSP/Options/PokemonBDSP_BerryTable.h
    Source/PokemonBDSP/Options/PokemonBDSP_EggStepOption.cpp
    Source/PokemonBDSP/Options/PokemonBDSP_EggStepOption.h
    Source/PokemonBDSP/Options/PokemonBDSP_EncounterBotCommon.h
    Source/PokemonBDSP/Options/PokemonBDSP_LearnMove.h
    Source/PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.cpp
    Source/PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h
    Source/PokemonBDSP/Panels_PokemonBDSP.cpp
    Source/PokemonBDSP/Panels_PokemonBDSP.h
    Source/PokemonBDSP/PokemonBDSP_Panels.cpp
    Source/PokemonBDSP/PokemonBDSP_Panels.h
    Source/PokemonBDSP/PokemonBDSP_Settings.cpp
    Source/PokemonBDSP/PokemonBDSP_Settings.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggAutonomous.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggAutonomous.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggAutonomousState.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggAutonomousState.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggFeedback.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggFeedback.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggFetcher.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggFetcher.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggHatcher.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggHatcher.h
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggRoutines.cpp
    Source/PokemonBDSP/Programs/Eggs/PokemonBDSP_EggRoutines.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_AmitySquarePickUpFarmer.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_AmitySquarePickUpFarmer.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_DoublesLeveling.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_DoublesLeveling.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_GiftBerryReset.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_GiftBerryReset.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_MoneyFarmerRoute210.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_MoneyFarmerRoute210.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_MoneyFarmerRoute212.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_MoneyFarmerRoute212.h
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_PoffinCooker.cpp
    Source/PokemonBDSP/Programs/Farming/PokemonBDSP_PoffinCooker.h
    Source/PokemonBDSP/Programs/General/PokemonBDSP_AutonomousBallThrower.cpp
    Source/PokemonBDSP/Programs/General/PokemonBDSP_AutonomousBallThrower.h
    Source/PokemonBDSP/Programs/General/PokemonBDSP_MassRelease.cpp
    Source/PokemonBDSP/Programs/General/PokemonBDSP_MassRelease.h
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.2.cpp
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.2.h
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.3.cpp
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.3.h
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy2.cpp
    Source/PokemonBDSP/Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy2.h
    Source/PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_BasicCatcher.h
    Source/PokemonBDSP/Programs/PokemonBDSP_BoxRelease.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h
    Source/PokemonBDSP/Programs/PokemonBDSP_EncounterDetection.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_EncounterDetection.h
    Source/PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_EncounterHandler.h
    Source/PokemonBDSP/Programs/PokemonBDSP_GameEntry.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_GameEntry.h
    Source/PokemonBDSP/Programs/PokemonBDSP_GameNavigation.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h
    Source/PokemonBDSP/Programs/PokemonBDSP_GlobalRoomHeal.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_GlobalRoomHeal.h
    Source/PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_OverworldTrigger.h
    Source/PokemonBDSP/Programs/PokemonBDSP_RunFromBattle.cpp
    Source/PokemonBDSP/Programs/PokemonBDSP_RunFromBattle.h
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_LegendaryReset.cpp
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_LegendaryReset.h
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Fishing.cpp
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Fishing.h
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Overworld.cpp
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Overworld.h
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Shaymin.cpp
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Shaymin.h
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_StarterReset.cpp
    Source/PokemonBDSP/Programs/ShinyHunting/PokemonBDSP_StarterReset.h
    Source/PokemonBDSP/Programs/TestPrograms/PokemonBDSP_ShinyEncounterTester.cpp
    Source/PokemonBDSP/Programs/TestPrograms/PokemonBDSP_ShinyEncounterTester.h
    Source/PokemonBDSP/Programs/TestPrograms/PokemonBDSP_SoundListener.cpp
    Source/PokemonBDSP/Programs/TestPrograms/PokemonBDSP_SoundListener.h
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_SelfBoxTrade.cpp
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_SelfBoxTrade.h
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_SelfTouchTrade.cpp
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_SelfTouchTrade.h
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_TradeRoutines.cpp
    Source/PokemonBDSP/Programs/Trading/PokemonBDSP_TradeRoutines.h
    Source/PokemonBDSP/Resources/PokemonBDSP_NameDatabase.cpp
    Source/PokemonBDSP/Resources/PokemonBDSP_NameDatabase.h
    Source/PokemonHome/Inference/PokemonHome_BallReader.cpp
    Source/PokemonHome/Inference/PokemonHome_BallReader.h
    Source/PokemonHome/Inference/PokemonHome_BoxGenderDetector.cpp
    Source/PokemonHome/Inference/PokemonHome_BoxGenderDetector.h
    Source/PokemonHome/Options/PokemonHome_BoxSortingTable.cpp
    Source/PokemonHome/Options/PokemonHome_BoxSortingTable.h
    Source/PokemonHome/PokemonHome_Panels.cpp
    Source/PokemonHome/PokemonHome_Panels.h
    Source/PokemonHome/PokemonHome_Settings.cpp
    Source/PokemonHome/PokemonHome_Settings.h
    Source/PokemonHome/Programs/PokemonHome_BoxSorting.cpp
    Source/PokemonHome/Programs/PokemonHome_BoxSorting.h
    Source/PokemonHome/Programs/PokemonHome_GenerateNameOCR.cpp
    Source/PokemonHome/Programs/PokemonHome_GenerateNameOCR.h
    Source/PokemonHome/Programs/PokemonHome_PageSwap.cpp
    Source/PokemonHome/Programs/PokemonHome_PageSwap.h
    Source/PokemonHome/Resources/PokemonHome_PokeballSprites.cpp
    Source/PokemonHome/Resources/PokemonHome_PokeballSprites.h
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleMoveSelectionDetector.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleMoveSelectionDetector.h
    Source/PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_BattlePokemonSwitchDetector.h
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleSpriteWatcher.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleSpriteWatcher.h
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleStartDetector.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_BattleStartDetector.h
    Source/PokemonLA/Inference/Battles/PokemonLA_TransparentDialogueDetector.cpp
    Source/PokemonLA/Inference/Battles/PokemonLA_TransparentDialogueDetector.h
    Source/PokemonLA/Inference/Map/PokemonLA_MMOSpriteStarSymbolDetector.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MMOSpriteStarSymbolDetector.h
    Source/PokemonLA/Inference/Map/PokemonLA_MapDetector.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MapDetector.h
    Source/PokemonLA/Inference/Map/PokemonLA_MapMarkerLocator.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MapMarkerLocator.h
    Source/PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.h
    Source/PokemonLA/Inference/Map/PokemonLA_MapWeatherAndTimeReader.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MapWeatherAndTimeReader.h
    Source/PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h
    Source/PokemonLA/Inference/Map/PokemonLA_OutbreakReader.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h
    Source/PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h
    Source/PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.cpp
    Source/PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_ArcDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_ArcDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_BattleSpriteArrowDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_BattleSpriteArrowDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_BubbleDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_BubbleDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_FlagDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_FlagDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_FlagTracker.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h
    Source/PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_QuestMarkDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_QuestMarkDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h
    Source/PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.cpp
    Source/PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h
    Source/PokemonLA/Inference/PokemonLA_BerryTreeDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_BerryTreeDetector.h
    Source/PokemonLA/Inference/PokemonLA_BlackOutDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_BlackOutDetector.h
    Source/PokemonLA/Inference/PokemonLA_CommonColorCheck.cpp
    Source/PokemonLA/Inference/PokemonLA_CommonColorCheck.h
    Source/PokemonLA/Inference/PokemonLA_DialogDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_DialogDetector.h
    Source/PokemonLA/Inference/PokemonLA_ItemCompatibilityDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_ItemCompatibilityDetector.h
    Source/PokemonLA/Inference/PokemonLA_MountDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_MountDetector.h
    Source/PokemonLA/Inference/PokemonLA_NotificationReader.cpp
    Source/PokemonLA/Inference/PokemonLA_NotificationReader.h
    Source/PokemonLA/Inference/PokemonLA_OverworldDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_OverworldDetector.h
    Source/PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h
    Source/PokemonLA/Inference/PokemonLA_UnderAttackDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_UnderAttackDetector.h
    Source/PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.cpp
    Source/PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.h
    Source/PokemonLA/Inference/Sounds/PokemonLA_AlphaMusicDetector.cpp
    Source/PokemonLA/Inference/Sounds/PokemonLA_AlphaMusicDetector.h
    Source/PokemonLA/Inference/Sounds/PokemonLA_AlphaRoarDetector.cpp
    Source/PokemonLA/Inference/Sounds/PokemonLA_AlphaRoarDetector.h
    Source/PokemonLA/Inference/Sounds/PokemonLA_ItemDropSoundDetector.cpp
    Source/PokemonLA/Inference/Sounds/PokemonLA_ItemDropSoundDetector.h
    Source/PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.cpp
    Source/PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h
    Source/PokemonLA/Options/PokemonLA_BattlePokemonActionTable.cpp
    Source/PokemonLA/Options/PokemonLA_BattlePokemonActionTable.h
    Source/PokemonLA/Options/PokemonLA_CustomPathTable.cpp
    Source/PokemonLA/Options/PokemonLA_CustomPathTable.h
    Source/PokemonLA/Options/PokemonLA_IngoOpponent.cpp
    Source/PokemonLA/Options/PokemonLA_IngoOpponent.h
    Source/PokemonLA/Options/PokemonLA_MiscOptions.h
    Source/PokemonLA/Options/PokemonLA_ShinyDetectedAction.cpp
    Source/PokemonLA/Options/PokemonLA_ShinyDetectedAction.h
    Source/PokemonLA/Options/PokemonLA_TradeCountTable.cpp
    Source/PokemonLA/Options/PokemonLA_TradeCountTable.h
    Source/PokemonLA/Options/PokemonLA_TravelLocation.cpp
    Source/PokemonLA/Options/PokemonLA_TravelLocation.h
    Source/PokemonLA/Panels_PokemonLA.cpp
    Source/PokemonLA/Panels_PokemonLA.h
    Source/PokemonLA/PokemonLA_Locations.cpp
    Source/PokemonLA/PokemonLA_Locations.h
    Source/PokemonLA/PokemonLA_Panels.cpp
    Source/PokemonLA/PokemonLA_Panels.h
    Source/PokemonLA/PokemonLA_Settings.cpp
    Source/PokemonLA/PokemonLA_Settings.h
    Source/PokemonLA/PokemonLA_TravelLocations.cpp
    Source/PokemonLA/PokemonLA_TravelLocations.h
    Source/PokemonLA/PokemonLA_WeatherAndTime.cpp
    Source/PokemonLA/PokemonLA_WeatherAndTime.h
    Source/PokemonLA/Programs/Farming/PokemonLA_IngoBattleGrinder.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_IngoBattleGrinder.h
    Source/PokemonLA/Programs/Farming/PokemonLA_IngoMoveGrinder.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_IngoMoveGrinder.h
    Source/PokemonLA/Programs/Farming/PokemonLA_LeapGrinder.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_LeapGrinder.h
    Source/PokemonLA/Programs/Farming/PokemonLA_MagikarpMoveGrinder.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_MagikarpMoveGrinder.h
    Source/PokemonLA/Programs/Farming/PokemonLA_NuggetFarmerHighlands.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_NuggetFarmerHighlands.h
    Source/PokemonLA/Programs/Farming/PokemonLA_TenacityCandyFarmer.cpp
    Source/PokemonLA/Programs/Farming/PokemonLA_TenacityCandyFarmer.h
    Source/PokemonLA/Programs/General/PokemonLA_ApplyGrits.cpp
    Source/PokemonLA/Programs/General/PokemonLA_ApplyGrits.h
    Source/PokemonLA/Programs/General/PokemonLA_BraviaryHeightGlitch.cpp
    Source/PokemonLA/Programs/General/PokemonLA_BraviaryHeightGlitch.h
    Source/PokemonLA/Programs/General/PokemonLA_ClothingBuyer.cpp
    Source/PokemonLA/Programs/General/PokemonLA_ClothingBuyer.h
    Source/PokemonLA/Programs/General/PokemonLA_DistortionWaiter.cpp
    Source/PokemonLA/Programs/General/PokemonLA_DistortionWaiter.h
    Source/PokemonLA/Programs/General/PokemonLA_MMORoutines.cpp
    Source/PokemonLA/Programs/General/PokemonLA_MMORoutines.h
    Source/PokemonLA/Programs/General/PokemonLA_OutbreakFinder.cpp
    Source/PokemonLA/Programs/General/PokemonLA_OutbreakFinder.h
    Source/PokemonLA/Programs/General/PokemonLA_PokedexTasksReader.cpp
    Source/PokemonLA/Programs/General/PokemonLA_PokedexTasksReader.h
    Source/PokemonLA/Programs/General/PokemonLA_RamanasIslandCombee.cpp
    Source/PokemonLA/Programs/General/PokemonLA_RamanasIslandCombee.h
    Source/PokemonLA/Programs/General/PokemonLA_SkipToFullMoon.cpp
    Source/PokemonLA/Programs/General/PokemonLA_SkipToFullMoon.h
    Source/PokemonLA/Programs/ML/PokemonLA_GeneratePokemonImageTrainingData.cpp
    Source/PokemonLA/Programs/ML/PokemonLA_GeneratePokemonImageTrainingData.h
    Source/PokemonLA/Programs/PokemonLA_BattleRoutines.cpp
    Source/PokemonLA/Programs/PokemonLA_BattleRoutines.h
    Source/PokemonLA/Programs/PokemonLA_EscapeFromAttack.cpp
    Source/PokemonLA/Programs/PokemonLA_EscapeFromAttack.h
    Source/PokemonLA/Programs/PokemonLA_FlagNavigationAir.cpp
    Source/PokemonLA/Programs/PokemonLA_FlagNavigationAir.h
    Source/PokemonLA/Programs/PokemonLA_GameEntry.cpp
    Source/PokemonLA/Programs/PokemonLA_GameEntry.h
    Source/PokemonLA/Programs/PokemonLA_GameSave.cpp
    Source/PokemonLA/Programs/PokemonLA_GameSave.h
    Source/PokemonLA/Programs/PokemonLA_LeapPokemonActions.cpp
    Source/PokemonLA/Programs/PokemonLA_LeapPokemonActions.h
    Source/PokemonLA/Programs/PokemonLA_MountChange.cpp
    Source/PokemonLA/Programs/PokemonLA_MountChange.h
    Source/PokemonLA/Programs/PokemonLA_RegionNavigation.cpp
    Source/PokemonLA/Programs/PokemonLA_RegionNavigation.h
    Source/PokemonLA/Programs/PokemonLA_TimeOfDayChange.cpp
    Source/PokemonLA/Programs/PokemonLA_TimeOfDayChange.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_AutoMultiSpawn.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_AutoMultiSpawn.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_BurmyFinder.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_BurmyFinder.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_CrobatFinder.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_CrobatFinder.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_FroslassFinder.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_FroslassFinder.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_GalladeFinder.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_GalladeFinder.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_PostMMOSpawnReset.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_PostMMOSpawnReset.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-CustomPath.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-CustomPath.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-FlagPin.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-FlagPin.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-LakeTrio.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_ShinyHunt-LakeTrio.h
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_UnownFinder.cpp
    Source/PokemonLA/Programs/ShinyHunting/PokemonLA_UnownFinder.h
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_FlagNavigationTest.cpp
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_FlagNavigationTest.h
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_MountDetectionTest.cpp
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_MountDetectionTest.h
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_OverworldWatcher.cpp
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_OverworldWatcher.h
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_SoundListener.cpp
    Source/PokemonLA/Programs/TestPrograms/PokemonLA_SoundListener.h
    Source/PokemonLA/Programs/Trading/PokemonLA_SelfBoxTrade.cpp
    Source/PokemonLA/Programs/Trading/PokemonLA_SelfBoxTrade.h
    Source/PokemonLA/Programs/Trading/PokemonLA_SelfTouchTrade.cpp
    Source/PokemonLA/Programs/Trading/PokemonLA_SelfTouchTrade.h
    Source/PokemonLA/Programs/Trading/PokemonLA_TradeRoutines.cpp
    Source/PokemonLA/Programs/Trading/PokemonLA_TradeRoutines.h
    Source/PokemonLA/Resources/PokemonLA_AvailablePokemon.cpp
    Source/PokemonLA/Resources/PokemonLA_AvailablePokemon.h
    Source/PokemonLA/Resources/PokemonLA_NameDatabase.cpp
    Source/PokemonLA/Resources/PokemonLA_NameDatabase.h
    Source/PokemonLA/Resources/PokemonLA_PokemonInfo.cpp
    Source/PokemonLA/Resources/PokemonLA_PokemonInfo.h
    Source/PokemonLA/Resources/PokemonLA_PokemonSprites.cpp
    Source/PokemonLA/Resources/PokemonLA_PokemonSprites.h
    Source/PokemonLA/Resources/PokemonLA_WeatherAndTimeIcons.cpp
    Source/PokemonLA/Resources/PokemonLA_WeatherAndTimeIcons.h
    Source/PokemonLGPE/Commands/PokemonLGPE_DateSpam.cpp
    Source/PokemonLGPE/Commands/PokemonLGPE_DateSpam.h
    Source/PokemonLGPE/Inference/Battles/PokemonLGPE_BattleArrowDetector.cpp
    Source/PokemonLGPE/Inference/Battles/PokemonLGPE_BattleArrowDetector.h
    Source/PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.cpp
    Source/PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h
    Source/PokemonLGPE/Inference/Sounds/PokemonLGPE_ShinySoundDetector.cpp
    Source/PokemonLGPE/Inference/Sounds/PokemonLGPE_ShinySoundDetector.h
    Source/PokemonLGPE/PokemonLGPE_Panels.cpp
    Source/PokemonLGPE/PokemonLGPE_Panels.h
    Source/PokemonLGPE/PokemonLGPE_Settings.cpp
    Source/PokemonLGPE/PokemonLGPE_Settings.h
    Source/PokemonLGPE/Programs/Farming/PokemonLGPE_DailyItemFarmer.cpp
    Source/PokemonLGPE/Programs/Farming/PokemonLGPE_DailyItemFarmer.h
    Source/PokemonLGPE/Programs/PokemonLGPE_GameEntry.cpp
    Source/PokemonLGPE/Programs/PokemonLGPE_GameEntry.h
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_AlolanTrade.cpp
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_AlolanTrade.h
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_FossilRevival.cpp
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_FossilRevival.h
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_GiftReset.cpp
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_GiftReset.h
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_LegendaryReset.cpp
    Source/PokemonLGPE/Programs/ShinyHunting/PokemonLGPE_LegendaryReset.h
    Source/PokemonLGPE/Programs/TestPrograms/PokemonLGPE_SoundListener.cpp
    Source/PokemonLGPE/Programs/TestPrograms/PokemonLGPE_SoundListener.h
    Source/PokemonLZA/Inference/PokemonLZA_DialogDetector.cpp
    Source/PokemonLZA/Inference/PokemonLZA_DialogDetector.h
    Source/PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.cpp
    Source/PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h
    Source/PokemonRSE/Inference/PokemonRSE_ShinyNumberDetector.cpp
    Source/PokemonRSE/Inference/PokemonRSE_ShinyNumberDetector.h
    Source/PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.cpp
    Source/PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h
    Source/PokemonRSE/PokemonRSE_Navigation.cpp
    Source/PokemonRSE/PokemonRSE_Navigation.h
    Source/PokemonRSE/PokemonRSE_Panels.cpp
    Source/PokemonRSE/PokemonRSE_Panels.h
    Source/PokemonRSE/PokemonRSE_Settings.cpp
    Source/PokemonRSE/PokemonRSE_Settings.h
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_AudioStarterReset.cpp
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_AudioStarterReset.h
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_LegendaryHunt-Emerald.cpp
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_LegendaryHunt-Emerald.h
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_ShinyHunt-Deoxys.cpp
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_ShinyHunt-Deoxys.h
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_ShinyHunt-Mew.cpp
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_ShinyHunt-Mew.h
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_StarterReset.cpp
    Source/PokemonRSE/Programs/ShinyHunting/PokemonRSE_StarterReset.h
    Source/PokemonRSE/Programs/TestPrograms/PokemonRSE_SoundListener.cpp
    Source/PokemonRSE/Programs/TestPrograms/PokemonRSE_SoundListener.h
    Source/PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h
    Source/PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h
    Source/PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h
    Source/PokemonSV/Inference/Battles/PokemonSV_PostCatchDetector.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_PostCatchDetector.h
    Source/PokemonSV/Inference/Battles/PokemonSV_ShinySoundDetector.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_ShinySoundDetector.h
    Source/PokemonSV/Inference/Battles/PokemonSV_StartBattleYellowBar.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_StartBattleYellowBar.h
    Source/PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h
    Source/PokemonSV/Inference/Battles/PokemonSV_TeraRewardsMenu.cpp
    Source/PokemonSV/Inference/Battles/PokemonSV_TeraRewardsMenu.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxNatureDetector.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxNatureDetector.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h
    Source/PokemonSV/Inference/Boxes/PokemonSV_StatsResetChecker.cpp
    Source/PokemonSV/Inference/Boxes/PokemonSV_StatsResetChecker.h
    Source/PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.cpp
    Source/PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h
    Source/PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.cpp
    Source/PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h
    Source/PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.cpp
    Source/PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.cpp
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.h
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMaterialDetector.cpp
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMaterialDetector.h
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMenuDetector.cpp
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMenuDetector.h
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.cpp
    Source/PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.h
    Source/PokemonSV/Inference/Map/PokemonSV_DestinationMarkerDetector.cpp
    Source/PokemonSV/Inference/Map/PokemonSV_DestinationMarkerDetector.h
    Source/PokemonSV/Inference/Map/PokemonSV_FastTravelDetector.cpp
    Source/PokemonSV/Inference/Map/PokemonSV_FastTravelDetector.h
    Source/PokemonSV/Inference/Map/PokemonSV_MapDetector.cpp
    Source/PokemonSV/Inference/Map/PokemonSV_MapDetector.h
    Source/PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.cpp
    Source/PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h
    Source/PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.cpp
    Source/PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_NoMinimapDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_OliveDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_OliveDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h
    Source/PokemonSV/Inference/Overworld/PokemonSV_StationaryOverworldWatcher.cpp
    Source/PokemonSV/Inference/Overworld/PokemonSV_StationaryOverworldWatcher.h
    Source/PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.cpp
    Source/PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.cpp
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.cpp
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichPlateDetector.cpp
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichPlateDetector.h
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.cpp
    Source/PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h
    Source/PokemonSV/Inference/PokemonSV_AuctionItemNameReader.cpp
    Source/PokemonSV/Inference/PokemonSV_AuctionItemNameReader.h
    Source/PokemonSV/Inference/PokemonSV_BagDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_BagDetector.h
    Source/PokemonSV/Inference/PokemonSV_BlueberryQuestDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_BlueberryQuestDetector.h
    Source/PokemonSV/Inference/PokemonSV_BlueberryQuestReader.cpp
    Source/PokemonSV/Inference/PokemonSV_BlueberryQuestReader.h
    Source/PokemonSV/Inference/PokemonSV_ClothingTopDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_ClothingTopDetector.h
    Source/PokemonSV/Inference/PokemonSV_ESPEmotionDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_ESPEmotionDetector.h
    Source/PokemonSV/Inference/PokemonSV_MainMenuDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_MainMenuDetector.h
    Source/PokemonSV/Inference/PokemonSV_MenuOptionReader.cpp
    Source/PokemonSV/Inference/PokemonSV_MenuOptionReader.h
    Source/PokemonSV/Inference/PokemonSV_MoneyReader.cpp
    Source/PokemonSV/Inference/PokemonSV_MoneyReader.h
    Source/PokemonSV/Inference/PokemonSV_PokePortalDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_PokePortalDetector.h
    Source/PokemonSV/Inference/PokemonSV_PokemonMovesReader.cpp
    Source/PokemonSV/Inference/PokemonSV_PokemonMovesReader.h
    Source/PokemonSV/Inference/PokemonSV_PokemonSummaryReader.cpp
    Source/PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h
    Source/PokemonSV/Inference/PokemonSV_StatHexagonReader.cpp
    Source/PokemonSV/Inference/PokemonSV_StatHexagonReader.h
    Source/PokemonSV/Inference/PokemonSV_SweatBubbleDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h
    Source/PokemonSV/Inference/PokemonSV_TournamentPrizeNameReader.cpp
    Source/PokemonSV/Inference/PokemonSV_TournamentPrizeNameReader.h
    Source/PokemonSV/Inference/PokemonSV_TutorialDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_TutorialDetector.h
    Source/PokemonSV/Inference/PokemonSV_WhiteButtonDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h
    Source/PokemonSV/Inference/PokemonSV_WhiteTriangleDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_WhiteTriangleDetector.h
    Source/PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.cpp
    Source/PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraCodeReader.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraCodeReader.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraRewardsReader.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraRewardsReader.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.h
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.cpp
    Source/PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.h
    Source/PokemonSV/Options/PokemonSV_AuctionItemSelectOption.cpp
    Source/PokemonSV/Options/PokemonSV_AuctionItemSelectOption.h
    Source/PokemonSV/Options/PokemonSV_AuctionItemTable.cpp
    Source/PokemonSV/Options/PokemonSV_AuctionItemTable.h
    Source/PokemonSV/Options/PokemonSV_AutoHostOptions.h
    Source/PokemonSV/Options/PokemonSV_BBQOption.cpp
    Source/PokemonSV/Options/PokemonSV_BBQOption.h
    Source/PokemonSV/Options/PokemonSV_BattleMoveTable.cpp
    Source/PokemonSV/Options/PokemonSV_BattleMoveTable.h
    Source/PokemonSV/Options/PokemonSV_EggPowerSandwichOption.cpp
    Source/PokemonSV/Options/PokemonSV_EggPowerSandwichOption.h
    Source/PokemonSV/Options/PokemonSV_EncounterActionsTable.cpp
    Source/PokemonSV/Options/PokemonSV_EncounterActionsTable.h
    Source/PokemonSV/Options/PokemonSV_EncounterBotCommon.h
    Source/PokemonSV/Options/PokemonSV_PlayerList.cpp
    Source/PokemonSV/Options/PokemonSV_PlayerList.h
    Source/PokemonSV/Options/PokemonSV_SandwichIngredientsOption.cpp
    Source/PokemonSV/Options/PokemonSV_SandwichIngredientsOption.h
    Source/PokemonSV/Options/PokemonSV_SandwichIngredientsTable.cpp
    Source/PokemonSV/Options/PokemonSV_SandwichIngredientsTable.h
    Source/PokemonSV/Options/PokemonSV_SandwichMakerOption.cpp
    Source/PokemonSV/Options/PokemonSV_SandwichMakerOption.h
    Source/PokemonSV/Options/PokemonSV_SinglesAIOption.cpp
    Source/PokemonSV/Options/PokemonSV_SinglesAIOption.h
    Source/PokemonSV/Options/PokemonSV_SinglesMoveTable.cpp
    Source/PokemonSV/Options/PokemonSV_SinglesMoveTable.h
    Source/PokemonSV/Options/PokemonSV_TeraAIOption.cpp
    Source/PokemonSV/Options/PokemonSV_TeraAIOption.h
    Source/PokemonSV/Options/PokemonSV_TeraCatchOnWinOption.cpp
    Source/PokemonSV/Options/PokemonSV_TeraCatchOnWinOption.h
    Source/PokemonSV/Options/PokemonSV_TeraMoveTable.cpp
    Source/PokemonSV/Options/PokemonSV_TeraMoveTable.h
    Source/PokemonSV/Options/PokemonSV_TeraRollFilter.cpp
    Source/PokemonSV/Options/PokemonSV_TeraRollFilter.h
    Source/PokemonSV/Options/PokemonSV_TournamentPrizeSelectOption.cpp
    Source/PokemonSV/Options/PokemonSV_TournamentPrizeSelectOption.h
    Source/PokemonSV/Options/PokemonSV_TournamentPrizeTable.cpp
    Source/PokemonSV/Options/PokemonSV_TournamentPrizeTable.h
    Source/PokemonSV/PokemonSV_Panels.cpp
    Source/PokemonSV/PokemonSV_Panels.h
    Source/PokemonSV/PokemonSV_Settings.cpp
    Source/PokemonSV/PokemonSV_Settings.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStoryTools.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStoryTools.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_00.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_00.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_01.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_01.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_02.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_02.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_03.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_03.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_04.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_04.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_05.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_05.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_06.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_06.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_07.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_07.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_08.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_08.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_09.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_09.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_10.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_10.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_11.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_11.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_12.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_12.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_13.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_13.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_14.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_14.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_15.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_15.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_16.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_16.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_17.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_17.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_18.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_18.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_19.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_19.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_20.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_20.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_21.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_21.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_22.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_22.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_23.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_23.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_24.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_AutoStory_Segment_24.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_MenuOption.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_MenuOptionDatabase.cpp
    Source/PokemonSV/Programs/AutoStory/PokemonSV_MenuOptionDatabase.h
    Source/PokemonSV/Programs/AutoStory/PokemonSV_OliveActionFailedException.h
    Source/PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.cpp
    Source/PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h
    Source/PokemonSV/Programs/Battles/PokemonSV_Battles.cpp
    Source/PokemonSV/Programs/Battles/PokemonSV_Battles.h
    Source/PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.cpp
    Source/PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxAttach.cpp
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxAttach.h
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.cpp
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.h
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.cpp
    Source/PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h
    Source/PokemonSV/Programs/Boxes/PokemonSV_MassAttachItems.cpp
    Source/PokemonSV/Programs/Boxes/PokemonSV_MassAttachItems.h
    Source/PokemonSV/Programs/Boxes/PokemonSV_MassRelease.cpp
    Source/PokemonSV/Programs/Boxes/PokemonSV_MassRelease.h
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggAutonomous.cpp
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggAutonomous.h
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggFetcher.cpp
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggFetcher.h
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggHatcher.cpp
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggHatcher.h
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.cpp
    Source/PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h
    Source/PokemonSV/Programs/Farming/PokemonSV_AuctionFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_AuctionFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_BBQSoloFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_BBQSoloFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_BlueberryCatchPhoto.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_BlueberryCatchPhoto.h
    Source/PokemonSV/Programs/Farming/PokemonSV_BlueberryQuests.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_BlueberryQuests.h
    Source/PokemonSV/Programs/Farming/PokemonSV_ClaimMysteryGift.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_ClaimMysteryGift.h
    Source/PokemonSV/Programs/Farming/PokemonSV_ESPTraining.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_ESPTraining.h
    Source/PokemonSV/Programs/Farming/PokemonSV_FlyingTrialFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_FlyingTrialFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_GimmighoulChestFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_GimmighoulChestFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_GimmighoulRoamingFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_GimmighoulRoamingFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_LPFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_LPFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_MaterialFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_MaterialFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h
    Source/PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer.h
    Source/PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer2.cpp
    Source/PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer2.h
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_ClipboardFastCodeEntry.cpp
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_ClipboardFastCodeEntry.h
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.cpp
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_FastCodeEntry.cpp
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_FastCodeEntry.h
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_VideoFastCodeEntry.cpp
    Source/PokemonSV/Programs/FastCodeEntry/PokemonSV_VideoFastCodeEntry.h
    Source/PokemonSV/Programs/FormHunting/PokemonSV_ThreeSegmentDudunsparceFinder.cpp
    Source/PokemonSV/Programs/FormHunting/PokemonSV_ThreeSegmentDudunsparceFinder.h
    Source/PokemonSV/Programs/General/PokemonSV_AutonomousBallThrower.cpp
    Source/PokemonSV/Programs/General/PokemonSV_AutonomousBallThrower.h
    Source/PokemonSV/Programs/General/PokemonSV_ClothingBuyer.cpp
    Source/PokemonSV/Programs/General/PokemonSV_ClothingBuyer.h
    Source/PokemonSV/Programs/General/PokemonSV_MassPurchase.cpp
    Source/PokemonSV/Programs/General/PokemonSV_MassPurchase.h
    Source/PokemonSV/Programs/General/PokemonSV_SizeChecker.cpp
    Source/PokemonSV/Programs/General/PokemonSV_SizeChecker.h
    Source/PokemonSV/Programs/General/PokemonSV_StatsReset.cpp
    Source/PokemonSV/Programs/General/PokemonSV_StatsReset.h
    Source/PokemonSV/Programs/General/PokemonSV_StatsResetEventBattle.cpp
    Source/PokemonSV/Programs/General/PokemonSV_StatsResetEventBattle.h
    Source/PokemonSV/Programs/Glitches/PokemonSV_CloneItems-1.0.1.cpp
    Source/PokemonSV/Programs/Glitches/PokemonSV_CloneItems-1.0.1.h
    Source/PokemonSV/Programs/Glitches/PokemonSV_RideCloner-1.0.1.cpp
    Source/PokemonSV/Programs/Glitches/PokemonSV_RideCloner-1.0.1.h
    Source/PokemonSV/Programs/Glitches/PokemonSV_WildItemFarmer.cpp
    Source/PokemonSV/Programs/Glitches/PokemonSV_WildItemFarmer.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_AutoItemPrinter.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_AutoItemPrinter.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterDatabase.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterDatabase.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterRNG.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterRNG.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterRNGTable.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterRNGTable.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterSeedCalc.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterSeedCalc.h
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterTools.cpp
    Source/PokemonSV/Programs/ItemPrinter/PokemonSV_ItemPrinterTools.h
    Source/PokemonSV/Programs/PokemonSV_AreaZero.cpp
    Source/PokemonSV/Programs/PokemonSV_AreaZero.h
    Source/PokemonSV/Programs/PokemonSV_ConnectToInternet.cpp
    Source/PokemonSV/Programs/PokemonSV_ConnectToInternet.h
    Source/PokemonSV/Programs/PokemonSV_GameEntry.cpp
    Source/PokemonSV/Programs/PokemonSV_GameEntry.h
    Source/PokemonSV/Programs/PokemonSV_MenuNavigation.cpp
    Source/PokemonSV/Programs/PokemonSV_MenuNavigation.h
    Source/PokemonSV/Programs/PokemonSV_SaveGame.cpp
    Source/PokemonSV/Programs/PokemonSV_SaveGame.h
    Source/PokemonSV/Programs/PokemonSV_Terarium.cpp
    Source/PokemonSV/Programs/PokemonSV_Terarium.h
    Source/PokemonSV/Programs/PokemonSV_WorldNavigation.cpp
    Source/PokemonSV/Programs/PokemonSV_WorldNavigation.h
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.cpp
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_SandwichMaker.cpp
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_SandwichMaker.h
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.cpp
    Source/PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_AreaZeroPlatform.cpp
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_AreaZeroPlatform.h
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.cpp
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-AreaZeroPlatform.cpp
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-AreaZeroPlatform.h
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-Scatterbug.cpp
    Source/PokemonSV/Programs/ShinyHunting/PokemonSV_ShinyHunt-Scatterbug.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHost.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHost.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHostLobbyWaiter.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHostLobbyWaiter.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHostTools.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_AutoHostTools.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_JoinTracker.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_JoinTracker.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraMultiFarmer.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraMultiFarmer.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoller.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoller.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraSelfFarmer.cpp
    Source/PokemonSV/Programs/TeraRaids/PokemonSV_TeraSelfFarmer.h
    Source/PokemonSV/Programs/TestPrograms/PokemonSV_SoundListener.cpp
    Source/PokemonSV/Programs/TestPrograms/PokemonSV_SoundListener.h
    Source/PokemonSV/Programs/Trading/PokemonSV_SelfBoxTrade.cpp
    Source/PokemonSV/Programs/Trading/PokemonSV_SelfBoxTrade.h
    Source/PokemonSV/Programs/Trading/PokemonSV_TradeRoutines.cpp
    Source/PokemonSV/Programs/Trading/PokemonSV_TradeRoutines.h
    Source/PokemonSV/Resources/PokemonSV_AuctionItemNames.cpp
    Source/PokemonSV/Resources/PokemonSV_AuctionItemNames.h
    Source/PokemonSV/Resources/PokemonSV_FillingsCoordinates.h
    Source/PokemonSV/Resources/PokemonSV_Ingredients.cpp
    Source/PokemonSV/Resources/PokemonSV_Ingredients.h
    Source/PokemonSV/Resources/PokemonSV_ItemSprites.cpp
    Source/PokemonSV/Resources/PokemonSV_ItemSprites.h
    Source/PokemonSV/Resources/PokemonSV_NameDatabase.cpp
    Source/PokemonSV/Resources/PokemonSV_NameDatabase.h
    Source/PokemonSV/Resources/PokemonSV_PokemonSprites.cpp
    Source/PokemonSV/Resources/PokemonSV_PokemonSprites.h
    Source/PokemonSV/Resources/PokemonSV_TournamentPrizeNames.cpp
    Source/PokemonSV/Resources/PokemonSV_TournamentPrizeNames.h
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.cpp
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.cpp
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.cpp
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.cpp
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.cpp
    Source/PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.h
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.cpp
    Source/PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_BeamSetter.cpp
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_BeamSetter.h
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.cpp
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.cpp
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.cpp
    Source/PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.h
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxNatureDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxNatureDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxShinySymbolDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_BoxShinySymbolDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_DialogTriangleDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_DialogTriangleDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_FishingDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkFinder.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h
    Source/PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h
    Source/PokemonSwSh/Inference/PokemonSwSh_QuantityReader.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_QuantityReader.h
    Source/PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h
    Source/PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h
    Source/PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h
    Source/PokemonSwSh/Inference/PokemonSwSh_YCommDetector.cpp
    Source/PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h
    Source/PokemonSwSh/Inference/RNG/PokemonSwSh_OrbeetleAttackAnimationDetector.cpp
    Source/PokemonSwSh/Inference/RNG/PokemonSwSh_OrbeetleAttackAnimationDetector.h
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.cpp
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinySparkleSet.cpp
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinySparkleSet.h
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleDetectorRadial.cpp
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleDetectorRadial.h
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleDetectorSquare.cpp
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleDetectorSquare.h
    Source/PokemonSwSh/Inference/Sounds/PokemonSwSh_BerryTreeRustlingSoundDetector.cpp
    Source/PokemonSwSh/Inference/Sounds/PokemonSwSh_BerryTreeRustlingSoundDetector.h
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GenerateIVCheckerOCR.cpp
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GenerateIVCheckerOCR.h
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GenerateNameOCRPokedex.cpp
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GenerateNameOCRPokedex.h
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GeneratePokedexSprites.cpp
    Source/PokemonSwSh/InferenceTraining/PokemonSwSh_GeneratePokedexSprites.h
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_PathMatchup.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_PathMatchup.h
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_RentalBossMatchup.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_RentalBossMatchup.h
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SelectItem.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SelectMove.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SelectPath.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SelectStarter.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SwapCatch.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_SwapProfessor.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_Tools.cpp
    Source/PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_Tools.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_CatchScreenTracker.cpp
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_CatchScreenTracker.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.cpp
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.cpp
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.cpp
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.cpp
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h
    Source/PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Stats.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_HPPP.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_HPPP.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathMap.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathMap.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSide.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSide.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ProfessorSwap.cpp
    Source/PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ProfessorSwap.h
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.cpp
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_BossAction.cpp
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_BossAction.h
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.cpp
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.h
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.cpp
    Source/PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Hosting.h
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_BossFinder.cpp
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_BossFinder.h
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_Standard.cpp
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_Standard.h
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_StrongBoss.cpp
    Source/PokemonSwSh/MaxLair/PokemonSwSh_MaxLair_StrongBoss.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Adventure.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Adventure.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Battle.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Battle.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_CaughtScreen.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_CaughtScreen.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_EnterLobby.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_EnterLobby.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Entrance.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Entrance.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ItemSelect.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ItemSelect.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PathSelect.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PathSelect.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSelect.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSelect.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSwap.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSwap.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ProfessorSwap.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ProfessorSwap.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Start.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Start.h
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_StartSolo.cpp
    Source/PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_StartSolo.h
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterEnums.cpp
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterEnums.h
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOption.cpp
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOption.h
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOverride.cpp
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterOverride.h
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterWidget.cpp
    Source/PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterWidget.h
    Source/PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.cpp
    Source/PokemonSwSh/Options/PokemonSwSh_AutoHostNotification.h
    Source/PokemonSwSh/Options/PokemonSwSh_BallSelectOption.cpp
    Source/PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h
    Source/PokemonSwSh/Options/PokemonSwSh_Catchability.h
    Source/PokemonSwSh/Options/PokemonSwSh_CramomaticTable.cpp
    Source/PokemonSwSh/Options/PokemonSwSh_CramomaticTable.h
    Source/PokemonSwSh/Options/PokemonSwSh_DateToucher.cpp
    Source/PokemonSwSh/Options/PokemonSwSh_DateToucher.h
    Source/PokemonSwSh/Options/PokemonSwSh_EggStepOption.cpp
    Source/PokemonSwSh/Options/PokemonSwSh_EggStepOption.h
    Source/PokemonSwSh/Options/PokemonSwSh_EncounterBotCommon.h
    Source/PokemonSwSh/Options/PokemonSwSh_RegiSelector.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Battle.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Battle.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Field.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Field.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Moves.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Moves.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Stats.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Stats.h
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Types.cpp
    Source/PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Types.h
    Source/PokemonSwSh/PokemonSwSh_Panels.cpp
    Source/PokemonSwSh/PokemonSwSh_Panels.h
    Source/PokemonSwSh/PokemonSwSh_Settings.cpp
    Source/PokemonSwSh/PokemonSwSh_Settings.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer2.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer2.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-PokeJobsFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-PokeJobsFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.h
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattTraderFarmer.cpp
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattTraderFarmer.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_BeamReset.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_BeamReset.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperEU.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperEU.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperStats.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperUS.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperUS.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_EventBeamFinder.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_EventBeamFinder.h
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.cpp
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggAutonomous.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggAutonomous.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombined2.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombined2.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombinedShared.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcher2.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcher2.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcherMultiple.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcherMultiple.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHatcher.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHatcher.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHelpers.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggDuplication.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.cpp
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_AutonomousBallThrower.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_AutonomousBallThrower.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_BallThrower.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_BallThrower.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_BoxReorderNationalDex.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_BoxReorderNationalDex.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_ClothingBuyer.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_ClothingBuyer.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_DexRecFinder.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_DexRecFinder.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_MassRelease.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_MassRelease.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_SurpriseTrade.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_SurpriseTrade.h
    Source/PokemonSwSh/Programs/General/PokemonSwSh_TradeBot.cpp
    Source/PokemonSwSh/Programs/General/PokemonSwSh_TradeBot.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-Rolling.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-Rolling.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHostStats.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHostStats.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenRoller.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenRoller.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.cpp
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_LobbyWait.h
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Calyrex.cpp
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Calyrex.h
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Moltres.cpp
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Moltres.h
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Regi.cpp
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset-Regi.h
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset.cpp
    Source/PokemonSwSh/Programs/NonShinyHunting/PokemonSwSh_StatsReset.h
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldMovement.cpp
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldMovement.h
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTargetTracker.cpp
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTargetTracker.h
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTrajectory.cpp
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTrajectory.h
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTrigger.cpp
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTrigger.h
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_ShinyHuntAutonomous-Overworld.cpp
    Source/PokemonSwSh/Programs/OverworldBot/PokemonSwSh_ShinyHuntAutonomous-Overworld.h
    Source/PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h
    Source/PokemonSwSh/Programs/PokemonSwSh_BoxHelpers.h
    Source/PokemonSwSh/Programs/PokemonSwSh_EncounterDetection.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_EncounterDetection.h
    Source/PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h
    Source/PokemonSwSh/Programs/PokemonSwSh_GameEntry.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_GameEntry.h
    Source/PokemonSwSh/Programs/PokemonSwSh_Internet.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_Internet.h
    Source/PokemonSwSh/Programs/PokemonSwSh_MenuNavigation.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_MenuNavigation.h
    Source/PokemonSwSh/Programs/PokemonSwSh_RaidItemFarmerOKHO.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_RaidItemFarmerOKHO.h
    Source/PokemonSwSh/Programs/PokemonSwSh_ReleaseHelpers.h
    Source/PokemonSwSh/Programs/PokemonSwSh_SynchronizedSpinning.cpp
    Source/PokemonSwSh/Programs/PokemonSwSh_SynchronizedSpinning.h
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.cpp
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.cpp
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.cpp
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_CramomaticRNG.cpp
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_CramomaticRNG.h
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_DailyHighlightRNG.cpp
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_DailyHighlightRNG.h
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_SeedFinder.cpp
    Source/PokemonSwSh/Programs/RNG/PokemonSwSh_SeedFinder.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-BerryTree.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-BerryTree.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Fishing.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Fishing.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-IoATrade.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-IoATrade.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regi.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regi.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Whistling.cpp
    Source/PokemonSwSh/Programs/ShinyHuntAutonomous/PokemonSwSh_ShinyHuntAutonomous-Whistling.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_CurryHunter.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_CurryHunter.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_MultiGameFossil.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_MultiGameFossil.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHunt-Regi.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHunt-Regi.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntTools.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntTools.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-IoATrade.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-IoATrade.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regi.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regi.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regigigas2.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-Regigigas2.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.cpp
    Source/PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h
    Source/PokemonSwSh/Programs/TestPrograms/PokemonSwSh_ShinyEncounterTester.cpp
    Source/PokemonSwSh/Programs/TestPrograms/PokemonSwSh_ShinyEncounterTester.h
    Source/PokemonSwSh/Resources/PokemonSwSh_DailyHighlightDatabase.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_DailyHighlightDatabase.h
    Source/PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h
    Source/PokemonSwSh/Resources/PokemonSwSh_NameDatabase.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_NameDatabase.h
    Source/PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h
    Source/PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h
    Source/PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_TypeMatchup.h
    Source/PokemonSwSh/Resources/PokemonSwSh_TypeSprites.cpp
    Source/PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h
    Source/PokemonSwSh/ShinyHuntTracker.cpp
    Source/PokemonSwSh/ShinyHuntTracker.h
    Source/Tests/CommandLineTests.cpp
    Source/Tests/CommandLineTests.h
    Source/Tests/CommonFramework_Tests.cpp
    Source/Tests/CommonFramework_Tests.h
    Source/Tests/Kernels_Tests.cpp
    Source/Tests/Kernels_Tests.h
    Source/Tests/NintendoSwitch_Tests.cpp
    Source/Tests/NintendoSwitch_Tests.h
    Source/Tests/PokemonLA_Tests.cpp
    Source/Tests/PokemonLA_Tests.h
    Source/Tests/PokemonLZA_Tests.cpp
    Source/Tests/PokemonLZA_Tests.h
    Source/Tests/PokemonSV_Tests.cpp
    Source/Tests/PokemonSV_Tests.h
    Source/Tests/PokemonSwSh_Tests.cpp
    Source/Tests/PokemonSwSh_Tests.h
    Source/Tests/TestMap.cpp
    Source/Tests/TestMap.h
    Source/Tests/TestUtils.cpp
    Source/Tests/TestUtils.h
    Source/ZeldaTotK/Programs/ZeldaTotK_BowItemDuper.cpp
    Source/ZeldaTotK/Programs/ZeldaTotK_BowItemDuper.h
    Source/ZeldaTotK/Programs/ZeldaTotK_MineruItemDuper.cpp
    Source/ZeldaTotK/Programs/ZeldaTotK_MineruItemDuper.h
    Source/ZeldaTotK/Programs/ZeldaTotK_ParaglideItemDuper.cpp
    Source/ZeldaTotK/Programs/ZeldaTotK_ParaglideItemDuper.h
    Source/ZeldaTotK/Programs/ZeldaTotK_SurfItemDuper.cpp
    Source/ZeldaTotK/Programs/ZeldaTotK_SurfItemDuper.h
    Source/ZeldaTotK/Programs/ZeldaTotK_WeaponDuper.cpp
    Source/ZeldaTotK/Programs/ZeldaTotK_WeaponDuper.h
    Source/ZeldaTotK/ZeldaTotK_Panels.cpp
    Source/ZeldaTotK/ZeldaTotK_Panels.h
    Source/ZeldaTotK/ZeldaTotK_Settings.cpp
    Source/ZeldaTotK/ZeldaTotK_Settings.h
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}../../ FILES ${MAIN_SOURCES})

if (APPLE)
    set(SerialPrograms_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../IconResource/icon.icns)
    # set_source_files_properties(SerialPrograms_ICON PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Links on how to create a MacOS app bundle with cmake
    # https://stackoverflow.com/questions/53560288/how-to-create-a-macos-app-bundle-with-cmake
    # https://cmake.org/cmake/help/latest/command/add_executable.html
    add_executable(SerialPrograms MACOSX_BUNDLE ${SerialPrograms_ICON} ${MAIN_SOURCES})
    set_target_properties(SerialPrograms PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER PokemonAutomation.SerialPrograms
        MACOSX_BUNDLE_BUNDLE_NAME SerialPrograms
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        # MacOSXBundleInfo.plist.in is modified from https://github.com/Kitware/CMake/blob/master/Modules/MacOSXBundleInfo.plist.in
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in
        RESOURCE ${SerialPrograms_ICON}
    )

    # make sure Packages repo, https://github.com/PokemonAutomation/Packages is placed in the same folder as this Arduino-Source repo
    # so the post-build command can copy the resources folder Packages/SerialPrograms/Resources/ into the built app bundle:
    set(RESOURCES_PATH "${REPO_ROOT_DIR}../Packages/SerialPrograms/Resources")
    message(STATUS "Set Resources folder path from Packages repo: ${RESOURCES_PATH}")
    add_custom_command(
        TARGET SerialPrograms
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCES_PATH}"
        "$<TARGET_FILE_DIR:SerialPrograms>/../Resources"
    )
else() # WIN and Linux:
    add_executable(SerialPrograms WIN32 ${MAIN_SOURCES})
endif()

set_target_properties(SerialPrograms PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(SerialPrograms PRIVATE Qt${QT_MAJOR}::Widgets Qt${QT_MAJOR}::SerialPort Qt${QT_MAJOR}::Multimedia Qt${QT_MAJOR}::MultimediaWidgets)
target_link_libraries(SerialPrograms PRIVATE Threads::Threads)

#add defines
target_compile_definitions(SerialPrograms PRIVATE NOMINMAX DPP_NO_DEPRECATED)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../Internal/SerialPrograms/Internal0.cpp")
    target_compile_definitions(SerialPrograms PRIVATE PA_OFFICIAL)
    target_sources(SerialPrograms PRIVATE ../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.cpp)
    target_sources(SerialPrograms PRIVATE ../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h)
    target_sources(SerialPrograms PRIVATE ../../Internal/SerialPrograms/Internal0.cpp)
    target_sources(SerialPrograms PRIVATE ../../Internal/SerialPrograms/Internal1.cpp)
endif()

#extract opencv_world4110d.dll from archive on Windows Debug build, extract Discord Social SDK
if (WIN32)
    file(ARCHIVE_EXTRACT
      INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/opencv_world4110d.zip
      DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/
    )

    file(ARCHIVE_EXTRACT
      INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_win.zip
      DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/
    )
endif()

#add include directory
target_include_directories(SerialPrograms SYSTEM PRIVATE ../3rdParty/)
target_include_directories(SerialPrograms PRIVATE ../ ../../Internal/ Source/)
target_link_directories(SerialPrograms PRIVATE ../3rdPartyBinaries/)




if (WIN32)
    add_library(OpenCV_lib IMPORTED UNKNOWN)
    target_include_directories(SerialPrograms SYSTEM PRIVATE ../3rdParty/opencv-4.11.0/)
    set_target_properties(OpenCV_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/opencv_world4110.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/opencv_world4110d.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    add_library(ONNX_lib IMPORTED UNKNOWN)
    target_include_directories(SerialPrograms SYSTEM PRIVATE ../3rdParty/ONNX/)
    set_target_properties(ONNX_lib PROPERTIES
        IMPORTED_LOCATION   ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/onnxruntime.lib
    )

    add_library(ONNX_Providers_lib IMPORTED UNKNOWN)
    set_target_properties(ONNX_Providers_lib PROPERTIES
        IMPORTED_LOCATION   ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/onnxruntime_providers_shared.lib
    )

    add_library(dpp_lib IMPORTED UNKNOWN)
    set_target_properties(dpp_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/dpp.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/dppd.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    add_library(discord_lib IMPORTED UNKNOWN)
    target_include_directories(SerialPrograms SYSTEM PRIVATE ../3rdParty/discord_social_sdk_win/)
    set_target_properties(discord_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_win/lib/release/discord_partner_sdk.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_win/lib/debug/discord_partner_sdk.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    target_link_libraries(
        SerialPrograms PRIVATE
        tesseractPA.lib
        OpenCV_lib
        ONNX_lib
        ONNX_Providers_lib
        dpp_lib
        discord_lib
    )

    target_compile_definitions(
        SerialPrograms PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
        PA_TESSERACT
        PA_DPP
        PA_SOCIAL_SDK
    )

    target_compile_options(SerialPrograms PRIVATE /FAs /FaAssembly/ /MP /W4 /WX /external:anglebrackets /external:W0 /utf-8)
    target_compile_options(SerialPrograms PRIVATE /wd5054)  # Deprecated enum arithemtic
    target_compile_options(SerialPrograms PRIVATE /wd4505)  # unreferenced local function has been removed

    set(ARCH_FLAGS_09_Nehalem       /W4)    # Dummy parameter
    set(ARCH_FLAGS_13_Haswell       /arch:AVX2)
    set(ARCH_FLAGS_17_Skylake       /arch:AVX512)
    set(ARCH_FLAGS_19_IceLake       /arch:AVX512)

    if(CMAKE_GENERATOR_TOOLSET MATCHES "ClangCL")
        target_compile_options(SerialPrograms PRIVATE -Wno-unused-function)
        target_compile_options(SerialPrograms PRIVATE -march=nehalem)
        set(ARCH_FLAGS_09_Nehalem       -march=nehalem)
        set(ARCH_FLAGS_13_Haswell       -march=haswell)
        set(ARCH_FLAGS_17_Skylake       -march=skylake-avx512)
        set(ARCH_FLAGS_19_IceLake       -march=icelake-client)
    endif()

    #   Run-time ISA dispatching
    target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_08_Nehalem)
    target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_13_Haswell)
    target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_17_Skylake)
    target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_19_IceLake)

#    target_compile_options(SerialPrograms PRIVATE /arch:AVX2   /DPA_Arch_x64_AVX2)
#    target_compile_options(SerialPrograms PRIVATE /arch:AVX512 /DPA_Arch_x64_AVX512)
#    target_compile_options(SerialPrograms PRIVATE /arch:AVX512 /DPA_Arch_x64_AVX512GF)
else() # macOS and Linux
    if (APPLE)
        # Get root path where Homebrew installs libraries. Example root path: "/opt/homebrew/",
        # where you can find libraries in "/opt/homebrew/lib/" and headers in "/opt/homebrew/include/".
        # CMake does not support ONNX Runtime, so we have to add ONNX Runtime paths on our own.
        # Save the root path as CMake variable HOMEBREW_PREFIX.
        execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

        # ==== Get ONNX Runtime paths ====
        # First, try to see if there is an ONNX Runtime library built from source.
        # Assume the onnxruntime repo, https://github.com/microsoft/onnxruntime is placed in the same folder as
        # this Arduino-Source repo:
        set(ONNXRUNTIME_ROOTDIR "${REPO_ROOT_DIR}../onnxruntime/")
        set(ONNXRUNTIME_HEADER_DIR "${ONNXRUNTIME_ROOTDIR}include/onnxruntime/core/session/")
        if(EXISTS "${ONNXRUNTIME_HEADER_DIR}")
            # we have ONNX Runtime built from source
            message("Use ONNX Runtime built from source at ${ONNXRUNTIME_ROOTDIR}")
            target_include_directories(SerialPrograms PRIVATE ${ONNXRUNTIME_HEADER_DIR})
            link_directories(SerialPrograms PRIVATE ${ONNXRUNTIME_ROOTDIR}/build/MacOS/Release)
            target_link_libraries(SerialPrograms PRIVATE ${ONNXRUNTIME_ROOTDIR}/build/MacOS/Release/libonnxruntime.dylib)
        else()
            message("Built-from-source ONNX Runtime folder ${ONNXRUNTIME_ROOTDIR} does not exist.")
            message("Use ONNX Runtime installed by Homebrew.")
            target_include_directories(SerialPrograms PRIVATE ${HOMEBREW_PREFIX}/include/onnxruntime)
            target_link_libraries(SerialPrograms PRIVATE ${HOMEBREW_PREFIX}/lib/libonnxruntime.dylib)
        endif()

    else() # Linux
        # Instructions took from https://github.com/microsoft/onnxruntime/discussions/6489
        # Step 1: Download the file
        file(DOWNLOAD
            https://github.com/microsoft/onnxruntime/releases/download/v1.22.0/onnxruntime-linux-x64-1.22.0.tgz
            ${CMAKE_BINARY_DIR}/onnxruntime-linux-x64-1.22.0.tgz
            SHOW_PROGRESS
        )

        # Step 2: Extract the archive
        execute_process(
            COMMAND tar -xzf onnxruntime-linux-x64-1.22.0.tgz
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        # Step 3: Copy files to /usr/local
        execute_process(
            COMMAND sudo cp -r ${CMAKE_BINARY_DIR}/onnxruntime-linux-x64-1.22.0/include /usr/local/
        )
        execute_process(
            COMMAND sudo cp -r ${CMAKE_BINARY_DIR}/onnxruntime-linux-x64-1.22.0/lib /usr/local/
        )

        # Step 4: Run ldconfig
        execute_process(
            COMMAND sudo ldconfig
            WORKING_DIRECTORY /usr/local/lib
        )

        # Link against ONNX Runtime
        target_link_libraries(SerialPrograms PRIVATE onnxruntime)
    endif()

    # Find OpenCV
    # set(OPENCV_DIR, "/usr/local/opt/opencv/lib/cmake/opencv4")
    find_package(OpenCV REQUIRED HINTS "/usr/local/opt/opencv/lib/cmake/opencv4/")
    # set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/usr/local/opt/opencv/lib/pkgconfig")
    # pkg_search_module(OPENCV REQUIRED opencv)
    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
    target_link_libraries(SerialPrograms PRIVATE ${OpenCV_LIBS})

    #we hope to use our own Tesseract build in future so we can rid that dependency
    #but right now to run on Linux and Mac we need to use external Tesseract library
    if (UNIX_LINK_TESSERACT)
        find_package(PkgConfig REQUIRED)
        target_compile_definitions(SerialPrograms PRIVATE PA_TESSERACT UNIX_LINK_TESSERACT)
        pkg_search_module(TESSERACT REQUIRED tesseract)
        pkg_search_module(LEPTONICA REQUIRED lept)
        include_directories(${TESSERACT_INCLUDE_DIRS})
        include_directories(${LEPTONICA_INCLUDE_DIRS})
        link_directories(${TESSERACT_LIBRARY_DIRS})
        link_directories(${LEPTONICA_LIBRARY_DIRS})
        target_link_libraries(SerialPrograms PRIVATE ${TESSERACT_LINK_LIBRARIES})
        target_link_libraries(SerialPrograms PRIVATE ${LEPTONICA_LINK_LIBRARIES})
    endif()

    # enable dpp integration
    if ((APPLE) AND (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64)|(ARM64)"))
        add_library(libdpp STATIC IMPORTED)
        set_target_properties(libdpp PROPERTIES
            IMPORTED_LOCATION ../3rdPartyBinaries/libdpp_macos_arm64.a
            INTERFACE_COMPILE_DEFINITIONS "PA_DPP"
            INTERFACE_LINK_LIBRARIES "-L/opt/homebrew/lib -lssl -lcrypto -lopus -lsodium -lz" # add dpp's deps
        )
        target_link_libraries(SerialPrograms PRIVATE libdpp)
    endif()

    if (APPLE)
        # Add -Wno-c11-extensions to avoid clang gives
        # /usr/local/Cellar/opencv/4.5.5_3/include/opencv4/opencv2/core/mat.inl.hpp:2116:9: error: '_Atomic' is a C11 extension
        # when compiling OpenCV
        # target_compile_options(SerialPrograms PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-c11-extensions)
        target_compile_options(SerialPrograms PRIVATE -Wall -Wextra -Wpedantic -Werror -Wshorten-64-to-32)
    else()
        # Assume GCC
        target_compile_options(SerialPrograms PRIVATE -Wall -Wextra -Wpedantic -Werror -fno-strict-aliasing)
    endif()

    # Set OS-specific flags
    if (WIN32)
    elseif (APPLE)
        # on macOS, need this framework to query OS API to control display sleep and system sleep behavior
        target_link_libraries(SerialPrograms PRIVATE "-framework IOKit -framework CoreFoundation")
    elseif(UNIX)
    endif()

    IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        # Arm CPU
        #   Run-time ISA dispatching
        target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_arm64_20_M1)

    else()
        # Intel CPU
        target_compile_options(SerialPrograms PRIVATE -march=nehalem)
        set(ARCH_FLAGS_09_Nehalem       -march=nehalem)
        set(ARCH_FLAGS_13_Haswell       -march=haswell)
        set(ARCH_FLAGS_17_Skylake       -march=skylake-avx512)
        set(ARCH_FLAGS_19_IceLake       -march=icelake-client)

        #   Run-time ISA dispatching
        target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_08_Nehalem)
        target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_13_Haswell)
        target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_17_Skylake)
        target_compile_definitions(SerialPrograms PRIVATE PA_AutoDispatch_x64_19_IceLake)

    endif()
endif()



#   Run-time CPU dispatching.

if (ARCH_FLAGS_09_Nehalem)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/AudioStreamConversion/AudioStreamConversion_Core_x86_SSE41.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_SSE41.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_SSE42.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_SSE41.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_SSE.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_SSE41.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x8_x64_SSE42.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x8_x64_SSE42.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_09_Nehalem}
)
endif()
if (ARCH_FLAGS_13_Haswell)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_AVX2.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX2.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX2.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX2.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX2.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x16_x64_AVX2.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x16_x64_AVX2.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x16_x64_AVX2.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_13_Haswell}
)
endif()
if (ARCH_FLAGS_17_Skylake)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX512.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX512.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX512.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x64_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x64_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_17_Skylake}
)
endif()
if (ARCH_FLAGS_19_IceLake)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX512-VNNI.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512-GF.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_19_IceLake}
)
endif()




if (WIN32)
#copy needed dlls
#file(COPY *.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB MY_DLLS
  "../3rdPartyBinaries/*.dll"
)
file(COPY ${MY_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy discord_social_sdk DLL based on build type
add_custom_command(
    TARGET SerialPrograms
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<IF:$<CONFIG:Debug>,"${PROJECT_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win/bin/debug/discord_partner_sdk.dll","${PROJECT_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win/bin/release/discord_partner_sdk.dll">
        $<TARGET_FILE_DIR:SerialPrograms>/Binaries64/discord_partner_sdk.dll
    COMMENT "Copying Discord SDK DLL to Binaries64"
)
endif()

if (QT_MAJOR GREATER_EQUAL 6)
    qt_finalize_target(SerialPrograms)
endif()
