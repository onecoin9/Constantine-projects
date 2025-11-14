#ifndef SERVICES_DUTMANAGER_H
#define SERVICES_DUTMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <memory>
#include <QMutex>
#include <QAtomicPointer>  // 添加原子指针头文件
#include "domain/Dut.h"
#include <QDateTime>
#include <QJsonObject>

namespace Services {

/**
 * @brief 管理所有DUT的生命周期和状态。
 * 此类充当所有Dut对象的中央存储库。它负责
 * 创建、跟踪和提供对DUT的访问，但其本身不包含
 * 任何复杂的业务流程逻辑。流程逻辑由WorkflowSteps处理，
 * WorkflowSteps使用此管理器来获取和更新DUT状态。
 */
class DutManager : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例
    static DutManager* instance();

    // 禁止拷贝和赋值
    DutManager(const DutManager&) = delete;
    DutManager& operator=(const DutManager&) = delete;
    
    ~DutManager();

    // --- DUT 管理 ---

    /**
     * @brief 创建并注册一个新的DUT，或者检索一个已存在的DUT。
     * @param dutId DUT的唯一标识符。
     * @return 指向DUT对象的共享指针。如果失败则返回nullptr。
     */
    Domain::Dut* registerDut(const QString &dutId);

    /**
     * @brief 通过ID检索一个DUT。
     * @param dutId 要检索的DUT的ID。
     * @return 指向DUT的共享指针，如果未找到则为nullptr。
     */
    Domain::Dut* getDut(const QString &dutId) const;

    /**
     * @brief 检索所有已注册的DUT ID。
     */
    QStringList getAllDutIds() const;

    /**
     * @brief 将所有DUT重置到其初始状态（例如，用于新的测试运行）。
     */
    void resetAllDuts();

    /**
     * @brief 注销一个DUT。
     * @param dutId 要注销的DUT的ID。
     * @return 如果成功则返回true，否则返回false。
     */
    bool unregisterDut(const QString &dutId);

    /**
     * @brief 更新指定 DUT 的状态。
     * @param dutId DUT ID
     * @param newState 新状态
     * @return 成功返回 true
     */
    bool setState(const QString& dutId, Domain::Dut::State newState);

    /**
     * @brief 写入测试数据到 DUT。
     * @param dutId DUT ID
     * @param data 任意键值对
     * @return 成功返回 true
     */
    bool setTestData(const QString& dutId, const QVariantMap& data);

    /**
     * @brief 更新 DUT 所在工位信息
     */
    bool setSite(const QString& dutId, const QString& site);

    /**
     * @brief 从独立的站点配置文件加载站点配置信息
     * @param siteConfigFilePath 站点配置文件路径（如 site.json）
     * @return 成功返回 true
     */
    bool loadSiteConfiguration(const QString& siteConfigFilePath);

    /**
     * @brief 站点信息结构体，包含配置信息和扫描信息
     */
    struct SiteInfo {
        // --- 从配置文件加载的静态信息 ---
        int siteIndex = -1;
        QString siteAlias;//站点别名
        QString siteSN;
        QString autoAlias;
        int socketCount = 0;
        quint64 socketEnable = 0;
        bool siteEnvInit = false;
        quint64 socketReady = 0;
        bool chipReady = false;
        quint64 chipEnable = 0;
        QString description;

        // --- 通过站点扫描动态获取的信息 ---
        bool isScanned = false;         // 标志位，表示此站点是否通过扫描发现
        QDateTime scanTime;             // 扫描发现的时间
        int chainID = -1;
        QString dpsFpgaVersion;
        QString dpsFwVersion;
        QString firmwareVersion;
        QString firmwareVersionDate;
        QString fpgaLocation;
        QString fpgaVersion;
        int hopNum = -1;
        QString ip;
        bool isLastHop = false;
        int linkNum = -1;
        QString mac;
        QJsonObject mainBoardInfo;
        QString muAppVersion;
        QString muAppVersionDate;
        QString muLocation;
        QString port;
        
        // --- 实时状态信息 ---
        bool hasChips = false;          // 当前是否有芯片
        quint64 currentChipMask = 0;    // 当前芯片位置掩码
        QDateTime lastPlacementTime;    // 最后放置芯片的时间
        QString currentWorkflowId;      // 当前正在执行的工作流
        QMap<uint16_t, QByteArray> uidMap;
        int successCount = 0;           // 成功计数
        int failCount = 0;              // 失败计数
        QByteArray currentChipStatus;
    };

    /**
     * @brief 获取所有站点信息
     */
    QMap<QString, SiteInfo> getAllSiteInfo() const;

    /**
     * @brief 获取指定站点信息
     */
    SiteInfo getSiteInfo(const QString& siteAlias) const;
    
    /**
     * @brief 根据站点索引获取站点信息
     * @param siteIndex 站点索引
     * @return 站点信息，如果未找到返回空的SiteInfo
     */
    SiteInfo getSiteInfoByIndex(int siteIndex) const;
    SiteInfo getSiteInfoByIp(const QString& ip) const;

    bool updateSiteInfoByIp(const QString& ip, const SiteInfo& siteInfo);

    bool updateSiteChipStatusByIndex(int siteId, const QByteArray& chipStatus);

    /**
     * @brief 从站点扫描结果更新站点信息
     * @param siteData 从DeviceDiscovered通知收到的JSON对象
     * @return 成功返回 true
     */
    bool updateSiteFromScan(const QJsonObject& siteData);
    bool updateSktFromDownLoadProject(const QString& data);
    /**
     * @brief 更新站点的芯片放置状态
     * @param siteIndex 站点索引
     * @param chipMask 芯片位置掩码
     * @param siteSn 站点序列号（可选）
     * @return 成功返回 true
     */
    bool updateSiteChipPlacement(int siteIndex, quint64 chipMask, const QString& siteSn = QString());


    bool updateSiteChipStatus(const QString& ip, const QByteArray& chipStatus);
    
    /**
     * @brief 更新站点的测试结果
     * @param siteIndex 站点索引
     * @param success 是否成功
     * @return 成功返回 true
     */
    bool updateSiteTestResult(int siteIndex, bool success);
    
    /**
     * @brief 清除站点的芯片状态
     * @param siteIndex 站点索引
     * @return 成功返回 true
     */
    bool clearSiteChips(int siteIndex);
    
    /**
     * @brief 获取站点的当前状态信息（用于条件判断）
     * @param siteIndex 站点索引
     * @return 包含状态信息的QVariantMap
     */
    QVariantMap getSiteStatus(int siteIndex) const;

signals:
    /**
     * @brief 当一个新的DUT被注册时发射。
     * @param dutId 新注册的DUT的ID。
     */
    void dutRegistered(const QString& dutId);

    /**
     * @brief 当一个DUT被注销时发射。
     * @param dutId 被注销的DUT的ID。
     */
    void dutUnregistered(const QString& dutId);

    /**
     * @brief 当一个DUT的状态改变时发射。
     * 这是一个方便的信号，它将单个Dut的stateChanged信号冒泡向上传递。
     * @param dutId 状态已改变的DUT的ID。
     * @param newState DUT的新状态。
     */
    void dutStateChanged(const QString& dutId, Domain::Dut::State newState);

    /**
     * @brief 当一个DUT的工位改变时发射。
     * @param dutId 工位已改变的DUT的ID。
     * @param newSite DUT的新工位。
     */
    void dutSiteChanged(const QString& dutId, const QString& newSite);

    /**
     * @brief 当站点配置加载完成时发射
     * @param siteCount 加载的站点数量
     * @param enabledSiteCount 启用的站点数量
     */
    void siteConfigurationLoaded(int siteCount, int enabledSiteCount);

    /**
     * @brief 当站点配置加载失败时发射
     * @param errorMessage 错误信息
     */
    void siteConfigurationFailed(const QString& errorMessage);

    /**
     * @brief 当一个站点的动态信息（例如，通过扫描）更新时发射
     * @param siteAlias 更新的站点别名
     * @param ip 更新的站点ip
     */
    void siteUpdated(const QString& siteAlias,const QString& ip);
    
    /**
     * @brief 当站点的芯片放置状态改变时发射
     * @param siteIndex 站点索引
     * @param chipMask 芯片位置掩码
     */
    void siteChipPlacementChanged(int siteIndex, quint64 chipMask);
    
    /**
     * @brief 当站点的测试结果更新时发射
     * @param siteIndex 站点索引
     * @param success 是否成功
     */
    void siteTestResultUpdated(int siteIndex, bool success);

private slots:
    // 修正：槽的签名必须与Dut发出的信号完全匹配
    void onDutStateChanged(const QString& dutId, Domain::Dut::State newState);
    void onDutSiteChanged(const QString& dutId, const QString& newSite);

private:
    // 将构造函数设为私有
    explicit DutManager(QObject* parent = nullptr);

    // 单例相关的静态成员
    static QMutex s_mutex;             // 用于双重检查锁定的互斥锁
    static QAtomicPointer<DutManager> s_instance;  // 原子指针，存储单例实例
    /**
     * @brief 内部版本的registerDut，假设调用方已获取互斥锁
     * @param dutId DUT的唯一标识符
     * @param emitSignal 是否立即发射dutRegistered信号
     * @param connectSignals 是否立即连接DUT信号
     * @return 指向DUT对象的共享指针
     */
    Domain::Dut* registerDutInternal(const QString &dutId, bool emitSignal = true, bool connectSignals = true);
    QMap<QString, Domain::Dut*> m_duts;
    QMap<QString, SiteInfo> m_siteInfoMap;  // siteAlias -> SiteInfo
    mutable QMutex m_mutex;
};

} //namespace  Services

#endif // SERVICES_DUTMANAGER_H 
