#ifndef PRODUCTINFOMODEL_H
#define PRODUCTINFOMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QPair>

class ProductInfoModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ProductInfoModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 更新数据的接口
    void updateValue(const QString& key, const QString& value);
    void updateLeakageRate();  // 预留的计算接口
    void updateOSRate();       // 预留的计算接口
    void updateProgRate();     // 预留的计算接口

private:
    QList<QPair<QString, QString>> m_data;  // 存储键值对
    void initializeData();                   // 初始化数据
    ProductInfoModel* productModel;
};

#endif // PRODUCTINFOMODEL_H 