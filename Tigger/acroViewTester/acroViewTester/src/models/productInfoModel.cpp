#include "productInfoModel.h"

ProductInfoModel::ProductInfoModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initializeData();
}

void ProductInfoModel::initializeData()
{
    m_data = {
        {"工单", "ABCD123"},
        {"数量", "1000"},
        {"良品", "950"},
        {"不良", "50"},
        {"OS Fail", "10"},
        {"Program Fail", "10"},
        {"OS通过率", "80%"},
        {"Prog通过率", "80%"},
        {"模式", "MES模式"},
        {"自动", "自动模式"},
        {"UPH", "8000"},
        {"用户", "admin"}
    };
}

int ProductInfoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}

QVariant ProductInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &pair = m_data[index.row()];
        return QString("%1: %2").arg(pair.first).arg(pair.second);
    }

    return QVariant();
}

void ProductInfoModel::updateValue(const QString& key, const QString& value)
{
    for (int i = 0; i < m_data.size(); ++i) {
        if (m_data[i].first == key) {
            m_data[i].second = value;
            emit dataChanged(index(i), index(i));
            break;
        }
    }
}

void ProductInfoModel::updateLeakageRate()
{
    int total = 1000;  // 总数
    int leakageFail = 10;  // Leakage失败数
    double rate = ((total - leakageFail) * 100.0) / total;
    updateValue("Leakage通过率", QString::number(rate, 'f', 1) + "%");
}

void ProductInfoModel::updateOSRate()
{
    // 预留的OS通过率计算逻辑
    // TODO: 实现实际的计算逻辑
}

void ProductInfoModel::updateProgRate()
{
    // 预留的Program通过率计算逻辑
    // TODO: 实现实际的计算逻辑
} 