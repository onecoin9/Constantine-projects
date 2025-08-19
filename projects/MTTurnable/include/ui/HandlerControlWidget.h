#ifndef HANDLERCONTROLWIDGET_H
#define HANDLERCONTROLWIDGET_H

#include "ui/IDeviceControlWidget.h"
#include <QJsonObject>
#include <memory>
#include <QTimer>
#include "domain/protocols/ISProtocol.h"  // 使用协议层的类型定义
#include "domain/protocols/SProtocol.h"   // 包含SProtocol以访问枚举
#include <QGridLayout>
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
    class IDevice;
}

namespace Presentation {

class HandlerControlWidget : public IDeviceControlWidget
{
    Q_OBJECT
    
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        SUCCESS,
        EVENT,
        RAW,
        JSON,
        STATUS,
        CONFIG
    };
    Q_ENUM(LogLevel)

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
    

    
    // 轴控制槽函数
    void onSendAxisMoveClicked();
    
    // 设备信号槽
    void onChipPlaced(int siteIndex, uint32_t slotEn, const QString &siteSn);
    void onContactCheckRequested(int siteIdx, const QByteArray &sktEn);
    void onRemainingCheckRequested(int siteIdx, const QByteArray &sktEn);
    void onAxisMovementRequested(const QString &axisSelect, int siteIdx, int targetAngle);
    void onAxisMovementCompleted(const QString &axisSelect, int siteIdx, int currentAngle, int result, const QString &errMsg);
    void onRawFrameReceived(const QByteArray &frame);
    void onDebugMessage(const QString &message);
    void onCommandFinished(const QJsonObject& result);
    void onCommandError(const QString& errorMsg);
    
    // 新增槽函数，用于处理站点选择变化
    void onSiteSelectionChanged(int index);
    
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
    void logFrame(const QString& direction, const QByteArray& frame);
    QJsonObject buildReadyInfo();
    
    // UI Elements
    std::shared_ptr<Domain::HandlerDevice> m_handlerDevice;
    
    QGroupBox* m_taskControlGroup;
    QLineEdit* m_taskPathEdit;
    QPushButton* m_loadTaskBtn;
    QPushButton* m_tellDevReadyBtn;
    QSpinBox* m_commVersionSpinBox;
    QPushButton* m_tellDevCommVersionBtn;
    
    // CMD3相关控件
    QSpinBox* m_productionQuantitySpinBox;
    QComboBox* m_supplyTrayCombo;
    QSpinBox* m_supplyTrayXSpinBox;
    QSpinBox* m_supplyTrayYSpinBox;
    QComboBox* m_productionTrayCombo;
    QSpinBox* m_productionTrayXSpinBox;
    QSpinBox* m_productionTrayYSpinBox;
    QComboBox* m_rejectTrayCombo;
    QSpinBox* m_rejectTrayXSpinBox;
    QSpinBox* m_rejectTrayYSpinBox;
    QSpinBox* m_reelStartPosSpinBox;

    QGroupBox* m_siteControlGroup;
    QPushButton* m_querySiteEnableBtn;
    QPushButton* m_getSiteMapBtn;
    
    // PDU 67 新UI控件
    QComboBox* m_pdu67SiteSelectCombo; // 站点选择
    QGridLayout* m_socketResultLayout; // 动态生成Socket结果的布局
    QWidget* m_socketResultWidget;     // 包含布局的Widget
    QPushButton* m_setDoneSiteBtn;
    QList<QPair<QCheckBox*, QSpinBox*>> m_socketResultControls; // 存储动态创建的控件

    // 旧的PDU67控件将被移除
    // QSpinBox* m_siteIdxSpinBox;
    // QLineEdit* m_resultEdit;

    QSpinBox* m_adapterCntSpinBox;
    QLineEdit* m_checkResultEdit;
    QPushButton* m_sendContactCheckResultBtn;
    QPushButton* m_sendRemainingCheckResultBtn;
    
    QGroupBox* m_axisControlGroup;
    QComboBox* m_axisSelectCombo;
    QSpinBox* m_axisSiteIdxSpinBox;
    QSpinBox* m_targetAngleSpinBox;
    QPushButton* m_sendAxisMoveBtn;
    
    QPushButton* m_showLogDialogBtn;
    QDialog* m_logDialog;
    QTextEdit* m_logTextEdit;
    QPushButton* m_clearLogBtn;
    QTimer* m_responseTimer;
    QStringList m_logCache;
    bool m_isSending;
    QString m_lastSentCommand;
};

} // namespace Presentation

#endif // HANDLERCONTROLWIDGET_H
