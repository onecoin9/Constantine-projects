#include "AngKCheckBoxHeader.h"  

AngKCheckBoxHeader::AngKCheckBoxHeader(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent),
    m_isChecked(false),
    m_checkBoxModel(nullptr),
    m_checkBoxColumn(0)
{
    // 初始化复选框样式
    m_checkBoxOption.state |= QStyle::State_Enabled;
    m_checkBoxOption.state |= QStyle::State_Off;
    setSectionsClickable(true);
}

void AngKCheckBoxHeader::setCheckBoxModel(QStandardItemModel* model, int column, const QString& str)
{
    m_checkBoxModel = model;
    m_checkBoxColumn = column;
    m_checkBoxTitle = str;

    // 连接模型的信号以同步复选框状态
    connect(m_checkBoxModel, &QStandardItemModel::itemChanged, this, &AngKCheckBoxHeader::updateCheckBoxState);

    updateCheckBoxState();
}

void AngKCheckBoxHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0)
    {
        QStyleOptionButton option;
        option.state = QStyle::State_Enabled | QStyle::State_Active;
        option.state |= m_isChecked ? QStyle::State_On : QStyle::State_Off;

        int checkboxSize = style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, this);
        QRect checkboxRect = QRect(
            rect.x() + 3, // 左边距
            rect.y() + (rect.height() - checkboxSize) / 2, // 垂直居中
            checkboxSize,
            checkboxSize
        );
        option.rect = checkboxRect;

        const_cast<AngKCheckBoxHeader*>(this)->m_checkBoxRect = checkboxRect;
        // 复选框
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, this);


        // 文本
        QRect textRect = QRect(
            checkboxRect.right() + 5, // 复选框右边距
            rect.y(),
            rect.width() - checkboxRect.width() - 10, // 留出右边距
            rect.height()
        );

        QStyleOptionHeader headerOption;
        initStyleOption(&headerOption);
        headerOption.rect = textRect;
        headerOption.text = m_checkBoxTitle;
        headerOption.position = QStyleOptionHeader::Middle;
        headerOption.textAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
        style()->drawControl(QStyle::CE_HeaderLabel, &headerOption, painter, this);
    }
}

void AngKCheckBoxHeader::mousePressEvent(QMouseEvent* event)
{

    if (m_checkBoxModel == nullptr || m_checkBoxColumn < 0) {
        QHeaderView::mousePressEvent(event);
        return;
    }
    if (m_checkBoxRect.contains(event->pos())) {
        // 切换复选框状态
        m_isChecked = !m_isChecked;
        emit checkBoxClicked(m_isChecked);


        int total = m_checkBoxModel->rowCount();
        Qt::CheckState state = m_isChecked ? Qt::Checked : Qt::Unchecked;
        for (int row = 0; row < total; ++row) {
            QStandardItem* item = m_checkBoxModel->item(row, m_checkBoxColumn);
            if (item) {
                item->setCheckState(state);
            }
        }


        // 更新复选框显示
        viewport()->update();

        event->accept();
        return;
    }

    QHeaderView::mousePressEvent(event);
}

void AngKCheckBoxHeader::updateCheckBoxState()
{
    if (m_checkBoxModel == nullptr || m_checkBoxColumn < 0)
        return;

    int checkedCount = 0;
    int total = m_checkBoxModel->rowCount();

    for (int row = 0; row < total; ++row) {
        QStandardItem* item = m_checkBoxModel->item(row, m_checkBoxColumn);
        if (item && item->checkState() == Qt::Checked) {
            checkedCount++;
        }
    }

    if (checkedCount == total && total > 0) {
        m_isChecked = true;
    }
    else {
        m_isChecked = false;
    }

    // 重绘表头以反映复选框状态
    viewport()->update();
}