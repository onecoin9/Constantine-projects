#ifndef ACAUTOMATICSETTING_H
#define ACAUTOMATICSETTING_H

#include <QDialog>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class ACAutomaticSetting; }
QT_END_NAMESPACE

namespace Presentation {

class ACAutomaticSetting : public QDialog
{
    Q_OBJECT

public:
    explicit ACAutomaticSetting(QWidget *parent = nullptr);
    ~ACAutomaticSetting();

private:
    void initCoordWidget();
    void InitText();
    void InitButton();
    void getCoordinfo(int type, QStringList& infoList);
    void getIpsTypeDesc(quint32 index, QString& desc);
    void packCmdMsg(QString &);
    void initIpsTypeCBox();
    bool GetLoadStatus();
    bool IsPluginLoad() { return m_bPluginLoaded; };
    void simulateHandlerEvents();
signals:
    void sgnAutoStartBurn(bool);
    void sgnAutoTellDevReady(int);
    void sgnClearLotData(int);
    void sgnCheckSitesTskStatus();
protected:
    void setLoadStatus(bool status);
public slots:
    void SetSitesTskPass(bool status) { m_bSitesTskPass = status; };
    void onSlotLoadAutoSetting();
    void onSlotTskPathSetting();
    void onSlotAutomicOver();
private:
    enum {
            IPSTYPE_5000_5800 = 0,
            IPSTYPE_3000_5200,
            IPSTYPE_7000_PHA2000,
            IPSTYPE_OTHER
    };
    /** 托盘配置方式 */
    enum {
        TRAY_CFG_DEFAULT = 0,   /// 默认
        TRAY_CFG_MANUAL,        /// 指定
        TRAY_CFG_AUTO           /// 自动
    };
    /** 托盘类型 */
    enum TRAY_TYPE {
        TRAY_TYPE_SUPPLY = 0,   /// 供给托盘
        TRAY_TYPE_PRODUCTION,   /// 良品托盘
        TRAY_TYPE_REJECT,       /// 不良品托盘
    };
private:
    Ui::ACAutomaticSetting *ui;
    QTimer* m_textChangeTimer;
    QTimer* m_timeOutTimer;
    QString m_strRecordProjPath;
    bool m_bLoading;
    bool m_bSitesTskPass;
    bool m_bPluginLoaded;
    QMap<int, int> m_trayMap;
};

} // namespace Presentation

#endif // ACAUTOMATICSETTING_H 
