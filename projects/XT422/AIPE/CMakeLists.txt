# CMakeLists.txt for Aprog project based on Aprog.vcxproj

cmake_minimum_required(VERSION 3.15)

project(Aprog LANGUAGES CXX)

# --- Add common Windows definitions to avoid macro conflicts ---
if(WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

# --- C++ Standard (from vcxproj) ---
set(CMAKE_CXX_STANDARD 14) # vcxproj specified stdcpp14
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Find Qt ---
# Modules identified from vcxproj PreprocessorDefinitions and AdditionalDependencies
# If CMake cannot find Qt, set CMAKE_PREFIX_PATH, e.g.,
# cmake .. -DCMAKE_PREFIX_PATH="C:/path/to/your/Qt/version/msvcXXXX_64"
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Svg
    Widgets
    Concurrent
    Network
    SerialPort
    WebEngine       # Might require separate handling or installation steps
    WebEngineCore   # Might require separate handling or installation steps
    WebEngineWidgets # Might require separate handling or installation steps
    UiTools         # Often used for dynamic UI loading
    # Charts          # Seen in some moc includes, add if needed
    # PropertyBrowser # This seems like a third-party lib (QtSolutions), handle separately
)
message(STATUS "Found Qt version: ${Qt5_VERSION}")

# --- Enable Qt Auto Tools ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Include Directories (from vcxproj AdditionalIncludeDirectories) ---
# Paths are relative to this CMakeLists.txt file
include_directories(
    # Directories relative to SolutionDir (this file's location)
    View/GlobalInit
    View/DlgComponents/MessageBox
    View/mainFrame
    View/mainFrame/Login
    View/mainFrame/MainWindow
    View/mainFrame/WindowComponents
    View/Components
    View/Components/ProgressComponents
    View/Components/ModelComponents
    View/Components/InterfaceComponents
    View/DlgComponents
    View/DlgComponents/eMMCComponents
    View/DlgComponents/ProjectComponents
    View/DlgComponents/ChipComponents
    View/DlgComponents/MenuBarComponents
    View/DlgComponents/BufferMapDlg
    View/DlgComponents/SettingComponents
    View/SProtocolTest
    Model/DataBase
    Model/SerializeData
    Model/DataTransform
    Model/ParserPlugins
    Model/ProjDataset
    Model/eMMC
    Model/DataBuffer
    Model/IOHeap
    Model/Device
    Controller/TransmitSignalsInstance
    Controller/MessageQueuePool
    Controller/IOHeapManager
    Controller/ACCmdHandler
    Controller/AutomaticManager
    Controller/eMMC
    Controller/Utils
    Controller/ChipManager
    Controller/LogManager
    Controller/ScanManager
    Controller/ChkManager
    Controller/EventManager
    Controller/DeviceManager
    Controller/ProjectManager
    Controller/TaskManager
    Controller/ReportManager
    Controller/RemoteServer # Added based on source file location

    # Directories relative to Aprog.vcxproj (Aprog/ subdir)
    Aprog # The '.' entry in vcxproj
    Aprog/GeneratedFiles # Usually handled by AUTOMOC/UIC/RCC, but include just in case
    Aprog/Depend/sqlite3
    Aprog/Depend/qHexedit # Assuming headers are here
    Aprog/Depend/pugixml
    Aprog/Depend/nlohmann
    Aprog/Depend/ACComlib/include
    Aprog/Depend/ACComLib/third_part
    Aprog/Depend/QtPropertyBrowser/QtPropertyBrowser/include
    Aprog/Depend/QtPropertyBrowser/QtPropertyBrowser
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${QtGui_PRIVATE_INCLUDE_DIRS}
)

# --- Library Directories (from vcxproj AdditionalLibraryDirectories) ---
# Paths are relative to this CMakeLists.txt file
link_directories(
    Aprog/Depend/sqlite3
    Aprog/Depend/qHexedit
    Aprog/Depend/pugixml
    Aprog/Depend/ACComlib/lib
    Aprog/Depend/QtPropertyBrowser/lib
)

# --- Source Files (from vcxproj ItemGroup) ---
# Paths are relative to this CMakeLists.txt file
set(PROJECT_SOURCES
    # --- ClCompile Items ---
    Controller/ACCmdHandler/ACCmdHandler.cpp
    Controller/ACCmdHandler/ACDeviceModel.cpp
    Controller/ACCmdHandler/ACNetComm.cpp
    Controller/ACCmdHandler/AngKRemoteCmdManager.cpp
    Controller/ACCmdHandler/AppConfigModel.cpp
    Controller/ACCmdHandler/AppModels.cpp
    Controller/ACCmdHandler/BufferModel.cpp
    Controller/ACCmdHandler/CmdHandler.cpp
    Controller/ACCmdHandler/CRC/Crc32_Comm.cpp
    Controller/ACCmdHandler/CRC/Crc32_Std.cpp
    Controller/ACCmdHandler/CustomMessageHandler.cpp
    Controller/ACCmdHandler/ICD.cpp
    Controller/ACCmdHandler/MT422DllHandler.cpp
    Controller/ACCmdHandler/MT422SerialHandler.cpp
    Controller/ACCmdHandler/SerialPortManager.cpp
    Controller/ACCmdHandler/SerialPortWorker.cpp
    Controller/ACCmdHandler/UplinkDataReceiver.cpp
    Controller/AutomaticManager/ACAutomaticManager.cpp
    Controller/AutomaticManager/AuthUtils.cpp
    Controller/AutomaticManager/AutoMessageServer.cpp
    Controller/AutomaticManager/ExternalServer.cpp
    Controller/ChipManager/ACChipManager.cpp
    Controller/ChkManager/AngKChkManager.cpp
    Controller/DeviceManager/ACDeviceManager.cpp
    Controller/eMMC/ACeMMCAnalyzeManager.cpp
    Controller/eMMC/ACMasterChipAnalyzeManager.cpp
    Controller/eMMC/CAcMbn.cpp
    Controller/EventManager/ACEventBuild.cpp
    Controller/EventManager/ACEventLogger.cpp
    Controller/EventManager/ACEventManager.cpp
    Controller/IOHeapManager/AngKIOHeapManager.cpp
    Controller/LogManager/AngKLogManager.cpp
    Controller/MessageQueuePool/AngKMessageHandler.cpp
    Controller/ProjectManager/ACProjManager.cpp
    Controller/RemoteServer/JsonRpcServer.cpp
    Controller/ReportManager/ACHtmlLogWriter.cpp
    Controller/ReportManager/ACReportRecord.cpp
    Controller/ScanManager/AngKScanManager.cpp
    Controller/TaskManager/ACSSDProjManager.cpp
    Controller/TaskManager/ACTaskManager.cpp
    Controller/TransmitSignalsInstance/AngKTransmitSignals.cpp
    Controller/Utils/AngKCommonTools.cpp
    Controller/Utils/AngKHelper.cpp
    Controller/Utils/AngkLogger.cpp
    Controller/Utils/AngKMisc.cpp
    Controller/Utils/AngKPathResolve.cpp
    Controller/Utils/AngKProjFile.cpp
    Controller/Utils/crc16.cpp
    Controller/Utils/crc32.c
    Controller/Utils/sha256.c
    Model/DataBase/AngKDBStandardItemModel.cpp
    Model/DataBase/ISQList.cpp
    Model/DataBuffer/AngKDataBuffer.cpp
    Model/DataTransform/DataTransform.cpp
    Model/Device/AngKDeviceModel.cpp
    Model/IOHeap/AngKIOHeapDevice.cpp
    Model/ParserPlugins/parser_global.cpp
    Model/ProjDataset/AngKProjDataset.cpp
    View/Components/InterfaceComponents/AngKControlWidget.cpp
    View/Components/InterfaceComponents/AngKShowLogArea.cpp
    View/Components/InterfaceComponents/AngKSiteWidget.cpp
    View/Components/InterfaceComponents/AngKTabProgram.cpp
    View/Components/InterfaceComponents/AngKTabProject.cpp
    View/Components/InterfaceComponents/AngKTabSite.cpp
    View/Components/InterfaceComponents/AngKTitleImg.cpp
    View/Components/InterfaceComponents/AngKTranslateSelect.cpp
    View/Components/MessageNotify/notify.cpp
    View/Components/MessageNotify/notifymanager.cpp
    View/Components/ModelComponents/AngKCheckBoxHeader.cpp
    View/Components/ModelComponents/AngKCustomDelegate.cpp
    View/Components/ModelComponents/AngKDBStandardItemModelEx.cpp
    View/Components/ModelComponents/AngKItemDelegate.cpp
    View/Components/ModelComponents/AngKProgressDelegate.cpp
    View/Components/ModelComponents/AngKSortFilterProxyModel.cpp
    View/Components/ModelComponents/AngKTableView.cpp
    View/Components/ProgressComponents/AngKProcessPathWidget.cpp
    View/Components/ProgressComponents/AngKProgressBar.cpp
    View/Components/ProgressComponents/AngKRoundProgressBar.cpp
    View/Components/ProgressComponents/AngKRoundProgressBarEx.cpp
    View/Components/ProgressComponents/CurrentProgressWgt.cpp
    View/Components/ProgressComponents/ProgressDialogSingleton.cpp
    View/Components/ProgressComponents/WarningNum.cpp
    View/Components/ProgressComponents/WarningWgt.cpp
    View/Components/ProgressComponents/YieldRateWgt.cpp
    View/DlgComponents/AngKDialog.cpp
    View/DlgComponents/BufferMapDlg/AngKBufferHexEdit.cpp
    View/DlgComponents/BufferMapDlg/AngKFillSetting.cpp
    View/DlgComponents/BufferMapDlg/AngKGotoSetting.cpp
    View/DlgComponents/BufferMapDlg/AngKHexViewEdit.cpp
    View/DlgComponents/ChipComponents/AngKChipDialog.cpp
    View/DlgComponents/ChipComponents/AngKChipSelectWidget.cpp
    View/DlgComponents/ChipComponents/AngKDeviceInfoDlg.cpp
    View/DlgComponents/eMMCComponents/ACeMMCSolution.cpp
    View/DlgComponents/eMMCComponents/AngKAddeMMCFile.cpp
    View/DlgComponents/eMMCComponents/AngKAddeMMCHuaweiFile.cpp
    View/DlgComponents/eMMCComponents/AngKemmCDriverPara.cpp
    View/DlgComponents/eMMCComponents/AngKemmcOption.cpp
    View/DlgComponents/eMMCComponents/AngKExtendCSD.cpp
    View/DlgComponents/eMMCComponents/eMMCParserManager.cpp
    View/DlgComponents/MenuBarComponents/ACAdapterCountInfomation.cpp
    View/DlgComponents/MenuBarComponents/ACAutomaticSetting.cpp
    View/DlgComponents/MenuBarComponents/ACDeviceManagerList.cpp
    View/DlgComponents/MenuBarComponents/ACMasterChipAnalyzeDlg.cpp
    View/DlgComponents/MenuBarComponents/ACProgrammerTestDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKAboutDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKAdapterInformation.cpp
    View/DlgComponents/MenuBarComponents/AngKCodeGeneratorDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKDeviceInfoTable.cpp
    View/DlgComponents/MenuBarComponents/AngKDriverParam.cpp
    View/DlgComponents/MenuBarComponents/AngKNandImageDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKPinMapSetting.cpp
    View/DlgComponents/MenuBarComponents/AngKProgrammerInfoDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKSerialNumberDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKSNCreator.cpp
    View/DlgComponents/MenuBarComponents/AngKSNCreatorDlg.cpp
    View/DlgComponents/MenuBarComponents/AngKUpdateFirmware.cpp
    View/DlgComponents/MessageBox/ACMessageBox.cpp
    View/DlgComponents/ProjectComponents/ACProjectCreateDlg.cpp
    View/DlgComponents/ProjectComponents/ACProjectProperty.cpp
    View/DlgComponents/ProjectComponents/ACTaskCreateDlg.cpp
    View/DlgComponents/ProjectComponents/ACTaskDownload.cpp
    View/DlgComponents/ProjectComponents/AngKFilesImport.cpp
    View/DlgComponents/ProjectComponents/AngKOperationTag.cpp
    View/DlgComponents/ProjectComponents/AngKProjectChipConfigWidget.cpp
    View/DlgComponents/ProjectComponents/AngKProjectChipSelect.cpp
    View/DlgComponents/ProjectComponents/AngKProjectDrvParaSetting.cpp
    View/DlgComponents/ProjectComponents/AngKProjectOperation.cpp
    View/DlgComponents/SettingComponents/AngKAdvanceSetting.cpp
    View/DlgComponents/SettingComponents/AngKChainSelectDlg.cpp
    View/DlgComponents/SettingComponents/AngKConnetDialog.cpp
    View/DlgComponents/SettingComponents/AngKDebugSetting.cpp
    View/DlgComponents/SettingComponents/AngKDemoWidget.cpp
    View/DlgComponents/SettingComponents/ACEthernetConfigWidget.cpp
    View/DlgComponents/SettingComponents/AngKSettingDlg.cpp
    View/DlgComponents/SettingComponents/AngKSwapSetting.cpp
    View/GlobalInit/AppModeType.cpp
    View/GlobalInit/DevErrCode.cpp
    View/mainFrame/agclient.cpp
    View/mainFrame/AngKGlobalInstance.cpp
    View/mainFrame/Login/AngKLoginChoose.cpp
    View/mainFrame/Login/AngKLoginDialog.cpp
    View/mainFrame/Login/AngKLoginInput.cpp
    View/mainFrame/MainWindow/AngKMainFrame.cpp
    View/mainFrame/MainWindow/AngKShadowWidget.cpp
    View/mainFrame/MainWindow/AngKSkinGridImage.cpp
    View/mainFrame/MainWindow/AngKWindowTitleBar.cpp
    View/mainFrame/WindowComponents/AngKCustomTab.cpp
    View/mainFrame/WindowComponents/AngKProgrammerWidget.cpp
    View/mainFrame/WindowComponents/AngKPropetryWidget.cpp
    View/mainFrame/WindowComponents/AngKReportWidget.cpp
    View/mainFrame/WindowComponents/AngKToolWidget.cpp
    View/mainFrame/WindowComponents/AnimWidgetManager.cpp
    Aprog/main.cpp

    # --- Headers (.h/.hpp) for AUTOMOC ---
    # Add headers that contain Q_OBJECT, Q_GADGET, Q_NAMESPACE, Q_ENUM_NS
    Controller/ACCmdHandler/ACCmdHandler.h
    Controller/ACCmdHandler/ACDeviceModel.h
    Controller/ACCmdHandler/ACNetComm.h
    Controller/ACCmdHandler/AngKRemoteCmdManager.h
    Controller/ACCmdHandler/AppConfigModel.h
    Controller/ACCmdHandler/AppModels.h
    Controller/ACCmdHandler/BufferModel.h
    Controller/ACCmdHandler/CmdHandler.h
    Controller/ACCmdHandler/CustomMessageHandler.h
    Controller/ACCmdHandler/MT422SerialHandler.h
    Controller/ACCmdHandler/SerialPortManager.h
    Controller/ACCmdHandler/SerialPortWorker.h
    Controller/ACCmdHandler/UplinkDataReceiver.h
    Controller/AutomaticManager/AutoInterface.h # Assuming Q_OBJECT/Q_PLUGIN_METADATA
    Controller/AutomaticManager/automaticplugin_global.h # Might contain Q_NAMESPACE etc.
    Controller/AutomaticManager/AutoMessageServer.h
    Controller/AutomaticManager/AuthUtils.h
    Controller/AutomaticManager/ExternalServer.h
    Controller/ChkManager/AngKChkManager.h
    Controller/DeviceManager/ACDeviceManager.h
    Controller/eMMC/ACeMMCAnalyzeManager.h
    Controller/eMMC/ACMasterChipAnalyzeManager.h
    Controller/EventManager/ACEventManager.h
    Controller/IOHeapManager/AngKIOHeapManager.h
    Controller/LogManager/AngKLogManager.h
    Controller/MessageQueuePool/AngKMessageHandler.h
    Controller/ProjectManager/ACProjManager.h
    Controller/RemoteServer/JsonRpcServer.h
    Controller/ReportManager/ACReportRecord.h
    Controller/ScanManager/AngKScanManager.h
    Controller/TaskManager/ACSSDProjManager.h
    Controller/TaskManager/ACTaskManager.h
    Controller/TransmitSignalsInstance/AngKTransmitSignals.h
    Model/DataBase/AngKDBStandardItemModel.h
    Model/DataBuffer/AngKDataBuffer.h
    Model/Device/AngKDeviceModel.h
    Model/IOHeap/AngKIOHeapDevice.h
    Model/ProjDataset/AngKProjDataset.h
    View/Components/InterfaceComponents/AngKControlWidget.h
    View/Components/InterfaceComponents/AngKShowLogArea.h
    View/Components/InterfaceComponents/AngKSiteWidget.h
    View/Components/InterfaceComponents/AngKTabProgram.h
    View/Components/InterfaceComponents/AngKTabProject.h
    View/Components/InterfaceComponents/AngKTabSite.h
    View/Components/InterfaceComponents/AngKTitleImg.h
    View/Components/InterfaceComponents/AngKTranslateSelect.h
    View/Components/MessageNotify/notify.h
    View/Components/MessageNotify/notifymanager.h
    View/Components/ModelComponents/AngKCheckBoxHeader.h
    View/Components/ModelComponents/AngKCustomDelegate.h
    View/Components/ModelComponents/AngKDBStandardItemModelEx.h
    View/Components/ModelComponents/AngKItemDelegate.h
    View/Components/ModelComponents/AngKProgressDelegate.h
    View/Components/ModelComponents/AngKSortFilterProxyModel.h
    View/Components/ModelComponents/AngKTableView.h
    View/Components/ProgressComponents/AngKProcessPathWidget.h
    View/Components/ProgressComponents/AngKProgressBar.h
    View/Components/ProgressComponents/AngKRoundProgressBar.h
    View/Components/ProgressComponents/AngKRoundProgressBarEx.h
    View/Components/ProgressComponents/CurrentProgressWgt.h
    View/Components/ProgressComponents/ProgressDialogSingleton.h
    View/Components/ProgressComponents/WarningNum.h
    View/Components/ProgressComponents/WarningWgt.h
    View/Components/ProgressComponents/YieldRateWgt.h
    View/DlgComponents/AngKDialog.h
    View/DlgComponents/BufferMapDlg/AngKBufferHexEdit.h
    View/DlgComponents/BufferMapDlg/AngKFillSetting.h
    View/DlgComponents/BufferMapDlg/AngKGotoSetting.h
    View/DlgComponents/BufferMapDlg/AngKHexViewEdit.h
    View/DlgComponents/ChipComponents/AngKChipDialog.h
    View/DlgComponents/ChipComponents/AngKChipSelectWidget.h
    View/DlgComponents/ChipComponents/AngKDeviceInfoDlg.h
    View/DlgComponents/eMMCComponents/ACeMMCSolution.h
    View/DlgComponents/eMMCComponents/AngKAddeMMCFile.h
    View/DlgComponents/eMMCComponents/AngKAddeMMCHuaweiFile.h
    View/DlgComponents/eMMCComponents/AngKemmCDriverPara.h
    View/DlgComponents/eMMCComponents/AngKemmcOption.h
    View/DlgComponents/eMMCComponents/AngKExtendCSD.h
    View/DlgComponents/eMMCComponents/eMMCParserManager.h
    View/DlgComponents/MenuBarComponents/ACAdapterCountInfomation.h
    View/DlgComponents/MenuBarComponents/ACAutomaticSetting.h
    View/DlgComponents/MenuBarComponents/ACDeviceManagerList.h
    View/DlgComponents/MenuBarComponents/ACMasterChipAnalyzeDlg.h
    View/DlgComponents/MenuBarComponents/ACProgrammerTestDlg.h
    View/DlgComponents/MenuBarComponents/AngKAboutDlg.h
    View/DlgComponents/MenuBarComponents/AngKAdapterInformation.h
    View/DlgComponents/MenuBarComponents/AngKCodeGeneratorDlg.h
    View/DlgComponents/MenuBarComponents/AngKDeviceInfoTable.h
    View/DlgComponents/MenuBarComponents/AngKDriverParam.h
    View/DlgComponents/MenuBarComponents/AngKNandImageDlg.h
    View/DlgComponents/MenuBarComponents/AngKPinMapSetting.h
    View/DlgComponents/MenuBarComponents/AngKProgrammerInfoDlg.h
    View/DlgComponents/MenuBarComponents/AngKSerialNumberDlg.h
    View/DlgComponents/MenuBarComponents/AngKSNCreator.h
    View/DlgComponents/MenuBarComponents/AngKSNCreatorDlg.h
    View/DlgComponents/MenuBarComponents/AngKUpdateFirmware.h
    View/DlgComponents/MessageBox/ACMessageBox.h
    View/DlgComponents/ProjectComponents/ACProjectCreateDlg.h
    View/DlgComponents/ProjectComponents/ACProjectProperty.h
    View/DlgComponents/ProjectComponents/ACTaskCreateDlg.h
    View/DlgComponents/ProjectComponents/ACTaskDownload.h
    View/DlgComponents/ProjectComponents/AngKFilesImport.h
    View/DlgComponents/ProjectComponents/AngKOperationTag.h
    View/DlgComponents/ProjectComponents/AngKProjectChipConfigWidget.h
    View/DlgComponents/ProjectComponents/AngKProjectChipSelect.h
    View/DlgComponents/ProjectComponents/AngKProjectDrvParaSetting.h
    View/DlgComponents/ProjectComponents/AngKProjectOperation.h
    View/DlgComponents/SettingComponents/AngKAdvanceSetting.h
    View/DlgComponents/SettingComponents/AngKChainSelectDlg.h
    View/DlgComponents/SettingComponents/AngKConnetDialog.h
    View/DlgComponents/SettingComponents/AngKDebugSetting.h
    View/DlgComponents/SettingComponents/AngKDemoWidget.h
    View/DlgComponents/SettingComponents/ACEthernetConfigWidget.h
    View/DlgComponents/SettingComponents/AngKSettingDlg.h
    View/DlgComponents/SettingComponents/AngKSwapSetting.h
    View/mainFrame/agclient.h
    View/mainFrame/AngKGlobalInstance.h
    View/mainFrame/Login/AngKLoginChoose.h
    View/mainFrame/Login/AngKLoginDialog.h
    View/mainFrame/Login/AngKLoginInput.h
    View/mainFrame/MainWindow/AngKMainFrame.h
    View/mainFrame/MainWindow/AngKShadowWidget.h
    View/mainFrame/MainWindow/AngKSkinGridImage.h
    View/mainFrame/MainWindow/AngKWindowTitleBar.h
    View/mainFrame/WindowComponents/AngKCustomTab.h
    View/mainFrame/WindowComponents/AngKProgrammerWidget.h
    View/mainFrame/WindowComponents/AngKPropetryWidget.h
    View/mainFrame/WindowComponents/AngKReportWidget.h
    View/mainFrame/WindowComponents/AngKToolWidget.h
    View/mainFrame/WindowComponents/AnimWidgetManager.h
    # Add other headers with Q_OBJECT/etc. if missed

    # --- UI Files (.ui) for AUTOUIC ---
    View/Components/InterfaceComponents/AngKControlWidget.ui
    View/Components/InterfaceComponents/AngKShowLogArea.ui
    View/Components/InterfaceComponents/AngKSiteWidget.ui
    View/Components/InterfaceComponents/AngKTabProgram.ui
    View/Components/InterfaceComponents/AngKTabProject.ui
    View/Components/InterfaceComponents/AngKTabSite.ui
    View/Components/InterfaceComponents/AngKTitleImg.ui
    View/Components/InterfaceComponents/AngKTranslateSelect.ui
    View/Components/ProgressComponents/AngKProcessPathWidget.ui
    View/Components/ProgressComponents/CurrentProgressWgt.ui
    View/Components/ProgressComponents/WarningNum.ui
    View/Components/ProgressComponents/WarningWgt.ui
    View/Components/ProgressComponents/YieldRateWgt.ui
    View/DlgComponents/AngKDialog.ui
    View/DlgComponents/BufferMapDlg/AngKBufferHexEdit.ui
    View/DlgComponents/BufferMapDlg/AngKFillSetting.ui
    View/DlgComponents/BufferMapDlg/AngKGotoSetting.ui
    View/DlgComponents/BufferMapDlg/AngKHexViewEdit.ui
    View/DlgComponents/ChipComponents/AngKChipDialog.ui
    View/DlgComponents/ChipComponents/AngKChipSelectWidget.ui
    View/DlgComponents/ChipComponents/AngKDeviceInfoDlg.ui
    View/DlgComponents/eMMCComponents/ACeMMCSolution.ui
    View/DlgComponents/eMMCComponents/AngKAddeMMCFile.ui
    View/DlgComponents/eMMCComponents/AngKAddeMMCHuaweiFile.ui
    View/DlgComponents/eMMCComponents/AngKemmCDriverPara.ui
    View/DlgComponents/eMMCComponents/AngKemmcOption.ui
    View/DlgComponents/eMMCComponents/AngKExtendCSD.ui
    View/DlgComponents/MenuBarComponents/ACAdapterCountInfomation.ui
    View/DlgComponents/MenuBarComponents/ACAutomaticSetting.ui
    View/DlgComponents/MenuBarComponents/ACDeviceManagerList.ui
    View/DlgComponents/MenuBarComponents/ACMasterChipAnalyzeDlg.ui
    View/DlgComponents/MenuBarComponents/ACProgrammerTestDlg.ui
    View/DlgComponents/MenuBarComponents/AngKAboutDlg.ui
    View/DlgComponents/MenuBarComponents/AngKAdapterInformation.ui
    View/DlgComponents/MenuBarComponents/AngKCodeGeneratorDlg.ui
    View/DlgComponents/MenuBarComponents/AngKDeviceInfoTable.ui
    View/DlgComponents/MenuBarComponents/AngKDriverParam.ui
    View/DlgComponents/MenuBarComponents/AngKNandImageDlg.ui
    View/DlgComponents/MenuBarComponents/AngKPinMapSetting.ui
    View/DlgComponents/MenuBarComponents/AngKProgrammerInfoDlg.ui
    View/DlgComponents/MenuBarComponents/AngKSerialNumberDlg.ui
    View/DlgComponents/MenuBarComponents/AngKSNCreator.ui
    View/DlgComponents/MenuBarComponents/AngKSNCreatorDlg.ui
    View/DlgComponents/MenuBarComponents/AngKUpdateFirmware.ui
    View/DlgComponents/MessageBox/ACMessageBox.ui
    View/DlgComponents/ProjectComponents/ACProjectCreateDlg.ui
    View/DlgComponents/ProjectComponents/ACProjectProperty.ui
    View/DlgComponents/ProjectComponents/ACTaskCreateDlg.ui
    View/DlgComponents/ProjectComponents/ACTaskDownload.ui
    View/DlgComponents/ProjectComponents/AngKFilesImport.ui
    View/DlgComponents/ProjectComponents/AngKOperationTag.ui
    View/DlgComponents/ProjectComponents/AngKProjectChipSelect.ui
    View/DlgComponents/ProjectComponents/AngKProjectDrvParaSetting.ui
    View/DlgComponents/ProjectComponents/AngKProjectOperation.ui
    View/DlgComponents/SettingComponents/AngKAdvanceSetting.ui
    View/DlgComponents/SettingComponents/AngKChainSelectDlg.ui
    View/DlgComponents/SettingComponents/AngKConnetDialog.ui
    View/DlgComponents/SettingComponents/AngKDebugSetting.ui
    View/DlgComponents/SettingComponents/AngKDemoWidget.ui
    View/DlgComponents/SettingComponents/ACEthernetConfigWidget.ui
    View/DlgComponents/SettingComponents/AngKSettingDlg.ui
    View/DlgComponents/SettingComponents/AngKSwapSetting.ui
    View/mainFrame/agclient.ui
    View/mainFrame/Login/AngKLoginChoose.ui
    View/mainFrame/Login/AngKLoginDialog.ui
    View/mainFrame/Login/AngKLoginInput.ui
    View/mainFrame/MainWindow/AngKMainFrame.ui
    View/mainFrame/WindowComponents/AngKProgrammerWidget.ui
    View/mainFrame/WindowComponents/AngKPropetryWidget.ui
    View/mainFrame/WindowComponents/AngKReportWidget.ui
    View/mainFrame/WindowComponents/AngKToolWidget.ui
    Aprog/QtGuiClass.ui # Assuming this exists

    # --- Resource Files (.qrc) for AUTORCC ---
    Aprog/agclient.qrc
)

# --- Create Executable ---
add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES}) # WIN32 creates a Windows GUI app (no console)

# --- Preprocessor Definitions (from vcxproj) ---
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    WIN32 # CMake usually defines WIN32 on Windows
    _ENABLE_EXTENDED_ALIGNED_STORAGE
    WIN64 # CMake usually defines WIN64 on 64-bit Windows
    ABSTRACTPARSER_LIB
    # QT_... definitions are usually handled by find_package(Qt5)
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    # Qt Modules
    Qt5::Core
    Qt5::Gui
    Qt5::Svg
    Qt5::Widgets
    Qt5::Concurrent
    Qt5::Network
    Qt5::SerialPort
    Qt5::WebEngine       # Check Qt documentation for linking WebEngine
    Qt5::WebEngineCore
    Qt5::WebEngineWidgets
    Qt5::UiTools
    Qt5::CorePrivate     # Added to access private headers like qzipreader_p.h

    # Other Libraries (from vcxproj AdditionalDependencies)
    # Using names without 'd' suffix - CMake might find debug/release automatically
    # If not, use generator expressions: $<IF:$<CONFIG:Debug>,pugixmld,pugixml>
    sqlite3
    AcHexViewEdit # Or AcHexViewEditd
    pugixml       # Or pugixmld
    QtSolutions_PropertyBrowser-head # Or QtSolutions_PropertyBrowser-headd
    ACComLib_x64
    $<IF:$<CONFIG:Debug>,AcHexViewEditd,AcHexViewEdit>
    $<IF:$<CONFIG:Debug>,pugixmld,pugixml>
    $<IF:$<CONFIG:Debug>,QtSolutions_PropertyBrowser-headd,QtSolutions_PropertyBrowser-head>
)

# --- MSVC Specific Settings ---
if(MSVC)
    # Add /utf-8 compile option (from vcxproj AdditionalOptions)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    # Link against qtmain(d).lib (needed for WinMain entry point with MSVC)
    # This is often handled automatically by Qt's CMake config, but can be explicit
    # target_link_libraries(${PROJECT_NAME} PRIVATE $<IF:$<CONFIG:Debug>,qtmaind,qtmain>)
endif()

# --- (Optional) Set Output Directory ---
# Match the vcxproj output: $(SolutionDir)Build\
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Build)
# You might want separate Debug/Release folders within Build:
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Build/${CMAKE_BUILD_TYPE})

message(STATUS "CMake configuration for Aprog generated from vcxproj.")
message(STATUS "Please verify source file list, Qt modules, include/link paths, and library names.")
message(STATUS "You may need to set CMAKE_PREFIX_PATH if Qt is not found automatically.")
