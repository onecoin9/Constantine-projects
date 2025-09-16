#ifndef GLOBALITEM_H
#define GLOBALITEM_H

#include <QObject>
#include <QMutex>
#include <QVariant>
#include <QMap>
#include <QString>

/**
 * @brief 全局单例类，用于存储和管理全局变量
 * 线程安全的单例实现，支持任意类型的全局变量存储
 */
class GlobalItem : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return GlobalItem& 单例引用
     */
    static GlobalItem& getInstance();

    /**
     * @brief 设置全局变量
     * @param key 变量键名
     * @param value 变量值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取全局变量
     * @param key 变量键名
     * @param defaultValue 默认值（如果键不存在）
     * @return QVariant 变量值
     */
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 检查键是否存在
     * @param key 变量键名
     * @return bool 是否存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除指定键
     * @param key 变量键名
     * @return bool 是否成功移除
     */
    bool remove(const QString& key);

    /**
     * @brief 清空所有变量
     */
    void clear();

    /**
     * @brief 获取所有键名
     * @return QStringList 所有键名列表
     */
    QStringList keys() const;

    /**
     * @brief 获取变量数量
     * @return int 变量数量
     */
    int size() const;

    // 便利方法：常用类型的直接访问
    void setString(const QString& key, const QString& value);
    QString getString(const QString& key, const QString& defaultValue = QString()) const;

    void setInt(const QString& key, int value);
    int getInt(const QString& key, int defaultValue = 0) const;

    void setBool(const QString& key, bool value);
    bool getBool(const QString& key, bool defaultValue = false) const;

    void setDouble(const QString& key, double value);
    double getDouble(const QString& key, double defaultValue = 0.0) const;

signals:
    /**
     * @brief 变量值改变时发射的信号
     * @param key 变量键名
     * @param newValue 新值
     * @param oldValue 旧值
     */
    void valueChanged(const QString& key, const QVariant& newValue, const QVariant& oldValue);

private:
    explicit GlobalItem(QObject* parent = nullptr);
    ~GlobalItem() override = default;

    // 禁用拷贝构造和赋值操作
    GlobalItem(const GlobalItem&) = delete;
    GlobalItem& operator=(const GlobalItem&) = delete;

    mutable QMutex m_mutex;           // 保护数据的互斥锁
    QMap<QString, QVariant> m_data;   // 存储全局变量的容器
};

#endif // GLOBALITEM_H

