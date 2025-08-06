#include <QStyledItemDelegate>
#include <QPushButton>
#include <QHBoxLayout>

class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ButtonDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override
    {
        QWidget* widget = new QWidget(parent);
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);

        QPushButton* resetButton = new QPushButton("重置密码", widget);
        QPushButton* deleteButton = new QPushButton("删除", widget);

        layout->addWidget(resetButton);
        layout->addWidget(deleteButton);

        connect(resetButton, &QPushButton::clicked, this, [=]() {
            emit resetPasswordClicked();
            });

        connect(deleteButton, &QPushButton::clicked, this, [=]() {
            emit deleteUserClicked();
            });

        return widget;
    }

signals:
    void resetPasswordClicked() const;
    void deleteUserClicked() const;
};