#ifndef CHIP_QUERY_WIDGET_H
#define CHIP_QUERY_WIDGET_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
    class ChipQueryWidget;
}

class ChipQueryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChipQueryWidget(QWidget* parent = nullptr);
    ~ChipQueryWidget();

private slots:
    void on_searchButton_clicked();
    void on_resetButton_clicked();
    void on_pageSizeComboBox_currentIndexChanged(int index);

private:
    Ui::ChipQueryWidget* ui;
    QStandardItemModel* model;
    void setupModel();
    void populateModelWithTestData();
};

#endif // CHIP_QUERY_WIDGET_H
