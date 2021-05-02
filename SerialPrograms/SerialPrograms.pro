
QT += core gui
QT += serialport
QT += multimedia
QT += multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../
INCLUDEPATH += Source/

CONFIG += force_debug_info

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


win32-g++{
    CONFIG += c++14

    QMAKE_CXXFLAGS += -msse4.2
#    QMAKE_CXXFLAGS += -Wnarrowing
#    QMAKE_CXXFLAGS += -Wno-unused-parameter
#    QMAKE_CXXFLAGS += -Wno-unused-function
#    QMAKE_CXXFLAGS += -Wno-missing-field-initializers

    DEFINES += TESS_IMPORTS
    DEFINES += WIN32
    LIBS += ../SerialPrograms/libtesseractc.lib
}
win32-msvc{
    QMAKE_CXXFLAGS += /std:c++latest


    DEFINES += TESS_IMPORTS
    DEFINES += WIN32
    LIBS += ../SerialPrograms/libtesseractc.lib
}


SOURCES += \
    ../ClientSource/Connection/PABotBase.cpp \
    ../ClientSource/Connection/PABotBaseConnection.cpp \
    ../ClientSource/Libraries/Logging.cpp \
    ../ClientSource/Libraries/MessageConverter.cpp \
    ../Common/CRC32.cpp \
    ../Common/Clientside/AsyncDispatcher.cpp \
    ../Common/Clientside/PrettyPrint.cpp \
    ../Common/Clientside/Unicode.cpp \
    ../Common/PokemonSwSh/PokemonSettings.cpp \
    ../Common/PokemonSwSh/PokemonSwShAutoHosts.cpp \
    ../Common/PokemonSwSh/PokemonSwShDateSpam.cpp \
    ../Common/PokemonSwSh/PokemonSwShDaySkippers.cpp \
    ../Common/PokemonSwSh/PokemonSwShEggRoutines.cpp \
    ../Common/PokemonSwSh/PokemonSwShGameEntry.cpp \
    ../Common/PokemonSwSh/PokemonSwShMisc.cpp \
    ../Common/Qt/CodeValidator.cpp \
    ../Common/Qt/ExpressionEvaluator.cpp \
    ../Common/Qt/Options/BooleanCheckBoxOption.cpp \
    ../Common/Qt/Options/FloatingPointOption.cpp \
    ../Common/Qt/Options/FossilTableOption.cpp \
    ../Common/Qt/Options/MultiHostTableOption.cpp \
    ../Common/Qt/Options/SimpleIntegerOption.cpp \
	../Common/Qt/Options/StringOption.cpp \
    ../Common/Qt/Options/SwitchDateOption.cpp \
    ../Common/Qt/Options/TimeExpressionOption.cpp \
    ../Common/Qt/QtJsonTools.cpp \
    ../Common/SwitchFramework/FrameworkSettings.cpp \
    ../Common/SwitchFramework/Switch_PushButtons.cpp \
    ../Common/SwitchRoutines/SwitchDigitEntry.cpp \
    Source/CommonFramework/CrashDump.cpp \
    Source/CommonFramework/Globals.cpp \
    Source/CommonFramework/Inference/AnomalyDetector.cpp \
    Source/CommonFramework/Inference/BlackScreenDetector.cpp \
    Source/CommonFramework/Inference/ColorClustering.cpp \
    Source/CommonFramework/Inference/FillGeometry.cpp \
    Source/CommonFramework/Inference/FillMatrix.cpp \
    Source/CommonFramework/Inference/ImageTools.cpp \
    Source/CommonFramework/Main.cpp \
    Source/CommonFramework/Options/FixedCode.cpp \
    Source/CommonFramework/Options/RandomCode.cpp \
    Source/CommonFramework/Options/SectionDivider.cpp \
    Source/CommonFramework/Panels/RightPanel.cpp \
    Source/CommonFramework/Panels/SettingsPanel.cpp \
    Source/CommonFramework/PersistentSettings.cpp \
    Source/CommonFramework/Tools/AsyncCommandSet.cpp \
    Source/CommonFramework/Tools/BotBaseHandle.cpp \
    Source/CommonFramework/Tools/ProgramEnvironment.cpp \
    Source/CommonFramework/Tools/StatsDatabase.cpp \
    Source/CommonFramework/Tools/StatsTracking.cpp \
    Source/CommonFramework/Widgets/CameraSelector.cpp \
    Source/CommonFramework/Widgets/ProgramList.cpp \
    Source/CommonFramework/Widgets/SerialSelector.cpp \
    Source/CommonFramework/Widgets/SettingList.cpp \
    Source/CommonFramework/Widgets/VideoOverlay.cpp \
    Source/CommonFramework/Windows/ButtonDiagram.cpp \
    Source/CommonFramework/Windows/MainWindow.cpp \
    Source/CommonFramework/Windows/OutputWindow.cpp \
    Source/NintendoSwitch/Framework/MultiSwitchProgram.cpp \
    Source/NintendoSwitch/Framework/MultiSwitchSystem.cpp \
    Source/NintendoSwitch/Framework/RunnableSwitchProgram.cpp \
    Source/NintendoSwitch/Framework/SingleSwitchProgram.cpp \
    Source/NintendoSwitch/Framework/SwitchCommandRow.cpp \
    Source/NintendoSwitch/Framework/SwitchSystem.cpp \
    Source/NintendoSwitch/Framework/VirtualSwitchController.cpp \
    Source/NintendoSwitch/Framework/VirtualSwitchControllerMapping.cpp \
    Source/NintendoSwitch/FrameworkSettingsPanel.cpp \
    Source/NintendoSwitch/Options/FriendCodeList.cpp \
    Source/NintendoSwitch/Programs/FriendCodeAdder.cpp \
    Source/NintendoSwitch/Programs/FriendDelete.cpp \
    Source/NintendoSwitch/Programs/PreventSleep.cpp \
    Source/NintendoSwitch/Programs/SwitchViewer.cpp \
    Source/PanelList.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_BeamSetter.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_FishingDetector.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkFinder.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkTracker.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_OverworldMarkTracker.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_RaidLobbyReader.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.cpp \
    Source/PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.cpp \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.cpp \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.cpp \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.cpp \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareDetector.cpp \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.cpp \
    Source/PokemonSwSh/Options/Catchability.cpp \
    Source/PokemonSwSh/Options/EggStepCount.cpp \
    Source/PokemonSwSh/Options/RegiSelector.cpp \
    Source/PokemonSwSh/PokemonSwSh_SettingsPanel.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_BallThrower.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_ClothingBuyer.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_MassRelease.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_SurpriseTrade.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TradeBot.cpp \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TurboA.cpp \
    Source/PokemonSwSh/Programs/PokemonSwSh_OverworldTrajectory.cpp \
    Source/PokemonSwSh/Programs/PokemonSwSh_StartGame.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_EncounterTracker.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_MultiGameFossil.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHunt-Regi.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-BerryTree.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Fishing.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-IoATrade.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Overworld.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regi.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Whistling.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-IoATrade.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regi.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regigigas2.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.cpp \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.cpp \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.cpp \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.cpp \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.cpp \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_BeamReset.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperEU.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperUS.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_EventBeamFinder.cpp \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombined2.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcher2.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHatcher.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggDuplication.cpp \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.cpp \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.cpp \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-Rolling.cpp \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenRoller.cpp \
    Source/PokemonSwSh/Programs/PokemonSwSh_RaidItemFarmerOKHO.cpp \
    Source/PokemonSwSh/Programs/PokemonSwSh_SynchronizedSpinning.cpp \
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.cpp \
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.cpp \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.cpp \
    Source/PokemonSwSh/Programs/TestProgram.cpp \
    Source/PokemonSwSh/ShinyHuntTracker.cpp

HEADERS += \
    ../ClientSource/Connection/BotBase.h \
    ../ClientSource/Connection/PABotBase.h \
    ../ClientSource/Connection/PABotBaseConnection.h \
    ../ClientSource/Connection/SerialConnection.h \
    ../ClientSource/Connection/SerialConnectionPOSIX.h \
    ../ClientSource/Connection/SerialConnectionWinAPI.h \
    ../ClientSource/Connection/StreamInterface.h \
    ../ClientSource/Libraries/Compiler.h \
    ../ClientSource/Libraries/Logging.h \
    ../ClientSource/Libraries/MessageConverter.h \
    ../Common/CRC32.h \
    ../Common/Clientside/AsyncDispatcher.h \
    ../Common/Clientside/FixedLimitVector.h \
    ../Common/Clientside/PrettyPrint.h \
    ../Common/Clientside/SpinLock.h \
    ../Common/Clientside/Unicode.h \
    ../Common/Compiler.h \
    ../Common/MessageProtocol.h \
    ../Common/PokemonSwSh/PokemonProgramIDs.h \
    ../Common/PokemonSwSh/PokemonSettings.h \
    ../Common/PokemonSwSh/PokemonSwShAutoHosts.h \
    ../Common/PokemonSwSh/PokemonSwShDateSpam.h \
    ../Common/PokemonSwSh/PokemonSwShDaySkippers.h \
    ../Common/PokemonSwSh/PokemonSwShEggRoutines.h \
    ../Common/PokemonSwSh/PokemonSwShGameEntry.h \
    ../Common/PokemonSwSh/PokemonSwShMisc.h \
    ../Common/Qt/CodeValidator.h \
    ../Common/Qt/ExpressionEvaluator.h \
    ../Common/Qt/Options/BooleanCheckBoxOption.h \
    ../Common/Qt/Options/FloatingPointOption.h \
    ../Common/Qt/Options/FossilTableOption.h \
    ../Common/Qt/Options/MultiHostTableOption.h \
    ../Common/Qt/Options/SimpleIntegerOption.h \
	../Common/Qt/Options/StringOption.h \
    ../Common/Qt/Options/SwitchDateOption.h \
    ../Common/Qt/Options/TimeExpressionOption.h \
    ../Common/Qt/QtJsonTools.h \
    ../Common/Qt/StringException.h \
    ../Common/SwitchFramework/FrameworkSettings.h \
    ../Common/SwitchFramework/Switch_PushButtons.h \
    ../Common/SwitchFramework/SwitchControllerDefs.h \
    ../Common/SwitchFramework/Switch_PushButtons.h \
    ../Common/SwitchRoutines/SwitchDigitEntry.h \
    Source/CommonFramework/CrashDump.h \
    Source/CommonFramework/Globals.h \
    Source/CommonFramework/Inference/AnomalyDetector.h \
    Source/CommonFramework/Inference/BlackScreenDetector.h \
    Source/CommonFramework/Inference/ColorClustering.h \
    Source/CommonFramework/Inference/FillGeometry.h \
    Source/CommonFramework/Inference/FillMatrix.h \
    Source/CommonFramework/Inference/FloatPixel.h \
    Source/CommonFramework/Inference/ImageTools.h \
    Source/CommonFramework/Inference/InferenceThrottler.h \
    Source/CommonFramework/Inference/InferenceTypes.h \
    Source/CommonFramework/Inference/StatAccumulator.h \
    Source/CommonFramework/Inference/TimeWindowStatTracker.h \
    Source/CommonFramework/Options/BooleanCheckBox.h \
    Source/CommonFramework/Options/ConfigOption.h \
    Source/CommonFramework/Options/FixedCode.h \
    Source/CommonFramework/Options/FloatingPoint.h \
    Source/CommonFramework/Options/RandomCode.h \
    Source/CommonFramework/Options/SectionDivider.h \
    Source/CommonFramework/Options/SimpleInteger.h \
    Source/CommonFramework/Panels/RightPanel.h \
    Source/CommonFramework/Panels/SettingsPanel.h \
    Source/CommonFramework/PersistentSettings.h \
    Source/CommonFramework/Tesseract/capi.h \
    Source/CommonFramework/Tesseract/platform.h \
    Source/CommonFramework/Tools/AsyncCommandSet.h \
    Source/CommonFramework/Tools/ConsoleHandle.h \
    Source/CommonFramework/Tools/Logger.h \
    Source/CommonFramework/Tools/ProgramEnvironment.h \
    Source/CommonFramework/Tools/StatsDatabase.h \
    Source/CommonFramework/Tools/StatsTracking.h \
    Source/CommonFramework/Tools/VideoFeed.h \
    Source/CommonFramework/Tools/BotBaseHandle.h \
    Source/CommonFramework/Widgets/CameraSelector.h \
    Source/CommonFramework/Widgets/ProgramList.h \
    Source/CommonFramework/Widgets/SerialSelector.h \
    Source/CommonFramework/Widgets/SettingList.h \
    Source/CommonFramework/Widgets/VideoOverlay.h \
    Source/CommonFramework/Windows/ButtonDiagram.h \
    Source/CommonFramework/Windows/MainWindow.h \
    Source/CommonFramework/Windows/OutputWindow.h \
    Source/NintendoSwitch/FixedInterval.h \
    Source/NintendoSwitch/Framework/MultiSwitchProgram.h \
    Source/NintendoSwitch/Framework/MultiSwitchSystem.h \
    Source/NintendoSwitch/Framework/RunnableSwitchProgram.h \
    Source/NintendoSwitch/Framework/SingleSwitchProgram.h \
    Source/NintendoSwitch/Framework/SwitchCommandRow.h \
    Source/NintendoSwitch/Framework/SwitchSetup.h \
    Source/NintendoSwitch/Framework/SwitchSystem.h \
    Source/NintendoSwitch/Framework/VirtualSwitchController.h \
    Source/NintendoSwitch/Framework/VirtualSwitchControllerMapping.h \
    Source/NintendoSwitch/FrameworkSettingsPanel.h \
    Source/NintendoSwitch/Options/FriendCodeList.h \
    Source/NintendoSwitch/Options/SwitchDate.h \
    Source/NintendoSwitch/Options/TimeExpression.h \
    Source/NintendoSwitch/Programs/FriendCodeAdder.h \
    Source/NintendoSwitch/Programs/FriendDelete.h \
    Source/NintendoSwitch/Programs/PreventSleep.h \
    Source/NintendoSwitch/Programs/SwitchViewer.h \
    Source/NintendoSwitch/Programs/VirtualConsole.h \
    Source/PanelList.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_BeamSetter.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_MarkTracker.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_OverworldMarkTracker.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_RaidLobbyReader.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h \
    Source/PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareDetector.h \
    Source/PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.h \
    Source/PokemonSwSh/Options/Catchability.h \
    Source/PokemonSwSh/Options/EggStepCount.h \
    Source/PokemonSwSh/Options/FossilTable.h \
    Source/PokemonSwSh/Options/MultiHostTable.h \
    Source/PokemonSwSh/Options/RegiSelector.h \
    Source/PokemonSwSh/PokemonSwSh_SettingsPanel.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_BallThrower.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_ClothingBuyer.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_MassRelease.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_SurpriseTrade.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TradeBot.h \
    Source/PokemonSwSh/Programs/BasicPrograms/PokemonSwSh_TurboA.h \
    Source/PokemonSwSh/Programs/PokemonSwSh_OverworldTrajectory.h \
    Source/PokemonSwSh/Programs/PokemonSwSh_StartGame.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_EncounterTracker.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_MultiGameFossil.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHunt-Regi.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-BerryTree.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Fishing.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-IoATrade.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Overworld.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regi.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-StrongSpawn.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-SwordsOfJustice.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntAutonomous-Whistling.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntTools.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-IoATrade.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regi.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-Regigigas2.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-StrongSpawn.h \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-BerryFarmer.h \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-DailyHighlightFarmer.h \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-LotoFarmer.h \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-StowOnSideFarmer.h \
    Source/PokemonSwSh/Programs/DateSpamFarmers/PokemonSwSh_DateSpam-WattFarmer.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_BeamReset.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperEU.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN-7.8k.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperJPN.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_DaySkipperUS.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_EventBeamFinder.h \
    Source/PokemonSwSh/Programs/DenHunting/PokemonSwSh_PurpleBeamFinder.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombined2.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggCombinedShared.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggFetcher2.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHatcher.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggHelpers.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_EggSuperCombined2.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggDuplication.h \
    Source/PokemonSwSh/Programs/EggPrograms/PokemonSwSh_GodEggItemDupe.h \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-MultiGame.h \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_AutoHost-Rolling.h \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenRoller.h \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h \
    Source/PokemonSwSh/Programs/Hosting/PokemonSwSh_LobbyWait.h \
    Source/PokemonSwSh/Programs/PokemonSwSh_RaidItemFarmerOKHO.h \
    Source/PokemonSwSh/Programs/PokemonSwSh_SynchronizedSpinning.h \
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FastCodeEntry.h \
    Source/PokemonSwSh/Programs/QoLMacros/PokemonSwSh_FriendSearchDisconnect.h \
    Source/PokemonSwSh/Programs/ReleaseHelpers.h \
    Source/PokemonSwSh/Programs/ShinyHunting/PokemonSwSh_ShinyHuntUnattended-SwordsOfJustice.h \
    Source/PokemonSwSh/Programs/TestProgram.h \
    Source/PokemonSwSh/ShinyHuntTracker.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
