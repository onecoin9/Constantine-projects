#ifndef DUT_H
#define DUT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <QDebug>
#include <QMetaType>
namespace Domain {

/**
 * @brief 代表一个独立的被测设备 (DUT).
 * 此类是一个数据对象，用于在其在测试过程中的整个生命周期内，
 * 持有单个芯片的状态和信息。
 */
class Dut : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ getState WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString currentSite READ getCurrentSite WRITE setCurrentSite NOTIFY siteChanged)

public:
    enum class State {
        Unknown,                // 新增：未知或初始状态
        Idle,                   // 空闲，在料盘里
        SKTEnable,              // 座子使能
        AtActivationStation,    // 已被Handler放置到激活工位
        Activating,             // Tester正在调用Aprog2.exe激活
        ActivationPassed,       // 激活成功
        ActivationFailed,       // 激活失败
        AtTestStation,          // 已被Handler放置到测试工位
        Testing,                // Tester正在采集数据
        TestingPassed,          // 测试通过
        TestingFailed,          // 测试失败
        ReadyForCalibration,    // [可选] 测试通过，等待标定
        Calibrating,            // [可选] 正在调用MTCaliTest.exe
        CalibrationPassed,      // [可选] 标定成功
        CalibrationFailed,      // [可选] 标定失败
        ReadyForBinning,        // 测试完成，等待Handler取走
        Binned,                 // 已被分拣
        Error                   // 流程中出现无法恢复的错误
    };
    Q_ENUM(State)

    explicit Dut(const QString& id, QObject *parent = nullptr);
    ~Dut() override;

    QString getId() const;

    State getState() const;
    void setState(State newState);

    QDateTime getLastUpdateTime() const;

    QVariantMap getTestData() const;
    void setTestData(const QVariantMap& data);
    void updateTestData(const QVariantMap &dataToUpdate);

    QString getCurrentSite() const;
    void setCurrentSite(const QString& site);

signals:
    void stateChanged(const QString& dutId, Domain::Dut::State newState);
    void testDataChanged(const QString& dutId, const QVariantMap& testData);
    void siteChanged(const QString& dutId, const QString& newSite);

private:
    //"Site02_Dut_11" QString m_id为这样的字符串
    QString m_id;
    State m_state;
    QDateTime m_lastUpdateTime;
    QVariantMap m_testData;
    QString m_currentSite;
};

} // namespace Domain

// 必须在全局命名空间声明，以便 Qt 的元对象系统识别
Q_DECLARE_METATYPE(Domain::Dut::State)

#endif // DUT_H 
