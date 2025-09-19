#include "GlobalItem.h"
#include <QMutexLocker>

GlobalItem& GlobalItem::getInstance()
{
    // C++11保证静态变量初始化的线程安全性
    static GlobalItem instance;
    return instance;
}

GlobalItem::GlobalItem(QObject* parent)
    : QObject(parent)
{
    // 构造函数为私有，确保只能通过getInstance()访问
}

void GlobalItem::setValue(const QString& key, const QVariant& value)
{
    QVariant oldValue;
    bool shouldEmitSignal = false;
    
    {
        QMutexLocker locker(&m_mutex);
        oldValue = m_data.value(key);
        
        // 只有当值真正改变时才更新和发射信号
        if (oldValue != value) {
            m_data[key] = value;
            shouldEmitSignal = true;
        }
    }
    
    // 在锁外发射信号，避免死锁
    if (shouldEmitSignal) {
        emit valueChanged(key, value, oldValue);
    }
}

QVariant GlobalItem::getValue(const QString& key, const QVariant& defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_data.value(key, defaultValue);
}

bool GlobalItem::contains(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    return m_data.contains(key);
}

bool GlobalItem::remove(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    return m_data.remove(key) > 0;
}

void GlobalItem::clear()
{
    QMutexLocker locker(&m_mutex);
    m_data.clear();
}

QStringList GlobalItem::keys() const
{
    QMutexLocker locker(&m_mutex);
    return m_data.keys();
}

int GlobalItem::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_data.size();
}

// 便利方法实现
void GlobalItem::setString(const QString& key, const QString& value)
{
    setValue(key, value);
}

QString GlobalItem::getString(const QString& key, const QString& defaultValue) const
{
    return getValue(key, defaultValue).toString();
}

void GlobalItem::setInt(const QString& key, int value)
{
    setValue(key, value);
}

int GlobalItem::getInt(const QString& key, int defaultValue) const
{
    return getValue(key, defaultValue).toInt();
}

void GlobalItem::setBool(const QString& key, bool value)
{
    setValue(key, value);
}

bool GlobalItem::getBool(const QString& key, bool defaultValue) const
{
    return getValue(key, defaultValue).toBool();
}

void GlobalItem::setDouble(const QString& key, double value)
{
    setValue(key, value);
}

double GlobalItem::getDouble(const QString& key, double defaultValue) const
{
    return getValue(key, defaultValue).toDouble();
}
