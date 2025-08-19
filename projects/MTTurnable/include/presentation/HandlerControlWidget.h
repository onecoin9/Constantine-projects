#ifndef HANDLERCONTROLWIDGET_H
#define HANDLERCONTROLWIDGET_H

#include "presentation/IDeviceControlWidget.h"
#include <QJsonObject>
#include <memory>
#include <QTimer>
#include "domain/protocols/ISProtocol.h"  // 使用协议层的类型定义
#include "domain/protocols/SProtocol.h"   // 包含SProtocol以访问枚举

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QComboBox;
class QSpinBox;
class QTableWidget;
class QTextEdit;
class QGroupBox;
class QCheckBox;
class QDialog;
QT_END_NAMESPACE

namespace Domain {
    class HandlerDevice;
}

namespace Presentation {

class HandlerControlWidget : public IDeviceControlWidget
{
    Q_OBJECT
    
public:
    explicit HandlerControlWidget(QWidget *parent = nullptr);
    ~HandlerControlWidget() override;
    
    // IDeviceControlWidget interface
    void setDevice(std::shared_ptr<Domain::IDevice> device) override;
    std::shared_ptr<Domain::IDevice> getDevice() const override;
    void updateStatus() override;
    void setControlsEnabled(bool enabled) override;
    QString getDeviceTypeName() const override;
    
private slots:
    // 任务控制槽函数
    void onLoadTaskClicked();
    void onTellDevReadyClicked();
    void onTellDevCommVersionClicked();
    
    // 站点控制槽函数
    void onQuerySiteEnableClicked();
    void onSetDoneSiteClicked();
    void onGetSiteMapClicked();
    void onSendContactCheckResultClicked();
    void onSendRemainingCheckResultClicked();
    
    // 新增：站点配置管理槽函数
    void onRefreshSiteConfigClicked();
    void onSaveSiteConfigClicked();
    void onApplySiteConfigClicked();
    void onConfigSiteChanged();
    
    // 新增：Socket快速设置槽函数
    void onEnableAllSocketsClicked();
    void onDisableAllSocketsClicked();
    void onEnableFirstHalfClicked();
    void onEnableSecondHalfClicked();
    
    // 轴控制槽函数
    void onSendAxisMoveClicked();
    
    // Device signals
    void onChipPlaced(int siteIndex, uint32_t slotEn, const QString &siteSn);
    void onContactCheckRequested(int siteIdx, const QByteArray &sktEn);
    void onRemainingCheckRequested(int siteIdx, const QByteArray &sktEn);
    void onProgramResultReceived(bool success, int errCode, const QString &errMsg);
    void onAxisMovementRequested(const QString &axisSelect, int siteIdx, int targetAngle);
    void onAxisMovementCompleted(const QString &axisSelect, int siteIdx, int currentAngle, int result, const QString &errMsg);
    void onRawFrameSent(const QByteArray &frame);
    void onRawFrameReceived(const QByteArray &frame);
    void onDebugMessage(const QString &message);
    
    // Asynchronous command handling slots
    void onCommandFinished(const QJsonObject& result);
    void onCommandError(const QString& errorMsg);
    
private:
    void setupUi();
    void createSiteControlGroup();
    void createTaskControlGroup();
    void createAxisControlGroup();
    void createBottomControlArea();
    void createLogDialog();
    void connectSignals();
    void updateDeviceStatus();
    void updateSiteTable();
    void log(const QString &message, const QString &category = "INFO");
    QString formatHexData(const QByteArray &data);
    QString getPduDescription(uint8_t pdu);
    QJsonObject buildReadyInfo();
    void initTrayCoordWidgets();
    void getCoordinfo(Domain::Protocols::SProtocol::TrayType type, QStringList& infoList);
    
    // 新增：站点配置管理辅助方法
    void loadSiteConfiguration();                          // 从设备加载站点配置
    void updateSiteConfigDisplay();                        // 更新站点配置显示
    void refreshSiteComboBox();                           // 刷新站点选择下拉框
    void displaySiteConfig(int siteIndex);               // 显示指定站点的配置
    bool validateSiteConfig();                            // 验证站点配置
    void applySiteConfigToDevice(int siteIndex);         // 应用站点配置到设备
    QString generateSocketEnableHex(int socketCount, const QString& pattern); // 生成Socket使能值
    void updateSiteTableRow(int row, const QJsonObject& siteData);           // 更新站点表格行
    void logFrame(const QString& direction, const QByteArray& frame);        // [新增] 智能记录数据帧
    
    // UI Elements
    std::shared_ptr<Domain::HandlerDevice> m_handlerDevice;
    
    QGroupBox* m_taskControlGroup;
    QLineEdit* m_taskPathEdit;
    QPushButton* m_browseTaskBtn;
    QPushButton* m_loadTaskBtn;
    QPushButton* m_tellDevReadyBtn;
    QSpinBox* m_commVersionSpinBox;
    QPushButton* m_tellDevCommVersionBtn;
    QLineEdit* m_passlotEdit;
    QLineEdit* m_reelStartPosEdit;
    // [改进] 托盘坐标配置控件
    QComboBox* m_ipsTypeCombo;
    QComboBox* m_supplyTrayTypeCombo;
    QComboBox* m_productionTrayTypeCombo;
    QComboBox* m_rejectTrayTypeCombo;
    QLineEdit* m_supplyTrayXEdit;
    QLineEdit* m_supplyTrayYEdit;
    QLineEdit* m_productionTrayXEdit;
    QLineEdit* m_productionTrayYEdit;
    QLineEdit* m_rejectTrayXEdit;
    QLineEdit* m_rejectTrayYEdit;

    QGroupBox* m_siteControlGroup;
    QTableWidget* m_siteTable;
    QPushButton* m_querySiteEnableBtn;
    QPushButton* m_getSiteMapBtn;
    QPushButton* m_refreshSiteConfigBtn;
    QPushButton* m_saveSiteConfigBtn;
    
    QComboBox* m_configSiteCombo;
    QSpinBox* m_socketCountSpinBox;
    QLineEdit* m_socketEnableEdit;
    QLineEdit* m_siteAliasEdit;
    QLineEdit* m_siteSNEdit;
    QCheckBox* m_siteInitCheckBox;
    QPushButton* m_applySiteConfigBtn;
    
    QPushButton* m_enableAllSocketsBtn;
    QPushButton* m_disableAllSocketsBtn;
    QPushButton* m_enableFirstHalfBtn;
    QPushButton* m_enableSecondHalfBtn;
    
    QSpinBox* m_siteIdxSpinBox;
    QLineEdit* m_resultEdit;
    QLineEdit* m_maskEdit;
    QPushButton* m_setDoneSiteBtn;
    
    // IC检查结果控件
    QSpinBox* m_adapterCntSpinBox;
    QLineEdit* m_checkResultEdit;
    QPushButton* m_sendContactCheckResultBtn;
    QPushButton* m_sendRemainingCheckResultBtn;
    
    QGroupBox* m_axisControlGroup;
    QComboBox* m_axisSelectCombo;
    QSpinBox* m_axisSiteIdxSpinBox;
    QSpinBox* m_targetAngleSpinBox;
    QPushButton* m_sendAxisMoveBtn;
    
    // [已删除] 原始帧调试相关控件

    // [改进] 日志弹窗相关
    QPushButton* m_showLogDialogBtn;
    QDialog* m_logDialog;
    QTextEdit* m_logTextEdit;
    QPushButton* m_clearLogBtn;
    
    QJsonObject m_lastReadyInfo;
    QJsonObject m_siteMapCache;
    QTimer* m_responseTimer;
    
    // [新增] 日志缓存
    QStringList m_logCache;
    
    // [新增] 发送状态管理
    bool m_isSending;
    QString m_lastSentCommand;
};

} // namespace Presentation

#endif // HANDLERCONTROLWIDGET_H