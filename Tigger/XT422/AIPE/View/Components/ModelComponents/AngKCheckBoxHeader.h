#ifndef CHECKBOXHEADER_H  
#define CHECKBOXHEADER_H  

#include <QHeaderView>  
#include <QStyleOptionButton>  
#include <QPainter>  
#include <QMouseEvent>  
#include <QStandardItemModel>  
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QTreeView>




class ToolTipDelegate : public QStyledItemDelegate {
public:
    explicit ToolTipDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    bool helpEvent(QHelpEvent* event,
        QAbstractItemView* view,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) override {
        if (!index.isValid())
            return false;

        // 获取显示的文本  
        QString text = index.data(Qt::DisplayRole).toString();

        if (!text.isEmpty()) {
            // 显示工具提示  
            //QToolTip::showText(event->globalPos(), text, view);
            QToolTip::showText(event->globalPos(), text, view->viewport());
            return true;
        }

        return QStyledItemDelegate::helpEvent(event, view, option, index);
    }
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {

        QStyledItemDelegate::paint(painter, option, index);
    }
};


class AngKCheckBoxHeader : public QHeaderView
{
    Q_OBJECT

public:
    explicit AngKCheckBoxHeader(Qt::Orientation orientation, QWidget* parent = nullptr);

    // 设置与全选复选框关联的模型和列  
    void setCheckBoxModel(QStandardItemModel* model, int column, const QString& str);
public slots:
    void updateCheckBoxState();
signals:
    void checkBoxClicked(bool checked);

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QStyleOptionButton m_checkBoxOption;
    QRect m_checkBoxRect;
    bool m_isChecked;
    QStandardItemModel* m_checkBoxModel;
    int m_checkBoxColumn;
    QString m_checkBoxTitle;

};

#endif // CHECKBOXHEADER_H