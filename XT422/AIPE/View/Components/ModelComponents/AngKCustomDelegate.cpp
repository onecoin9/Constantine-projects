#include "AngKCustomDelegate.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QStandardItemModel>
#include "ACMessageBox.h"
AngKCustomDelegate::AngKCustomDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
	, checkboxColumn(-1)
	, editColumn(-1)
	, bCheck(true)
	, extFlag(false)
{

}

AngKCustomDelegate::~AngKCustomDelegate()
{

}

QWidget* AngKCustomDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QWidget* editor = nullptr;
	if(editColumn == index.column())
	{
		editor = new QLineEdit(parent);
	}
	//else 
	//{
	//	return QItemDelegate::createEditor(parent, option, index);
	//}

	return editor;
}

void AngKCustomDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (editColumn == index.column()) {
		QString value = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
		if (!extFlag)
		{
			QRegExp hexRegexADDR("^[0-9A-Fa-f]+$");
			lineEdit->setValidator(new QRegExpValidator(hexRegexADDR, lineEdit));
		}
		else {
			if (index.row() == 12) {
				QRegExp hexRegexADDR("^[0-9A-Fa-f]{1,4}");
				lineEdit->setValidator(new QRegExpValidator(hexRegexADDR, lineEdit));
			}
			else {
				QRegExp hexRegexADDR("^[0-9A-Fa-f]{1,2}");
				lineEdit->setValidator(new QRegExpValidator(hexRegexADDR, lineEdit));
			}
		}
		lineEdit->setText(value);
	}
	//else 
	//{
	//	QItemDelegate::setEditorData(editor, index);
	//}
}

void AngKCustomDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	if (editColumn == index.column())
	{
		QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
		lineEdit->setAlignment(Qt::AlignCenter);

		if (model->property("CommonPara").toString() == "CommonPara" ||
			model->property("SelfPara").toString() == "SelfPara") {
			QStandardItemModel* m_pModel = qobject_cast<QStandardItemModel*>(model);
			QStringList rangeList = m_pModel->data(m_pModel->index(index.row(), index.column() - 1)).toString().split("-");
			int minRange = rangeList[0].toInt();
			int maxRange = rangeList[1].toInt();
			int editText = lineEdit->text().toInt();
			if (editText >= minRange && editText <= maxRange) {
				model->setData(index, lineEdit->text(), Qt::EditRole);
			}
			else {
				ACMessageBox::showError(editor, tr("Para Error"), tr("Input parameters are out of range"));
			}
		}
		else if (model->property("ExtCSDData").toString() == "ExtCSDData") {
			model->setData(index, lineEdit->text(), Qt::EditRole);
		}
	}
	else
	{
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}

void AngKCustomDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}

void AngKCustomDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (checkboxColumn == index.column())
	{
		bool value = index.data(Qt::UserRole).toInt();
		QStyleOptionButton checkBoxOption;
		QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxOption);
		checkBoxOption.rect = option.rect;
		checkBoxOption.rect.setLeft(option.rect.left() + (option.rect.width() - checkBoxRect.width()) / 2);
		checkBoxOption.rect.setTop(option.rect.top() + (option.rect.height() - checkBoxRect.height()) / 2);
		checkBoxOption.state = value ? QStyle::State_On : QStyle::State_Off;
		checkBoxOption.state |= QStyle::State_Enabled;
		QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
	}
	else
	{
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);
		opt.displayAlignment = Qt::AlignCenter; // 设置文本居中对齐
		QStyledItemDelegate::paint(painter, opt, index); // Let the base class handle other columns
	}
}

bool AngKCustomDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (bCheck)
	{
		if (event->type() == QEvent::MouseButtonDblClick && editColumn == index.column())//禁止双击编辑
		{
			bool checked = model->data(model->index(index.row(), 0), Qt::UserRole).toBool();
			if (!checked)
				return true;
		}
		//过滤鼠标松开
		if (event->type() == QEvent::MouseButtonRelease) {
			return false;
		}
		if (event->type() == QEvent::MouseButtonPress && checkboxColumn == index.column())
		{
			bool checked = index.data(Qt::UserRole).toBool();
			model->setData(index, !checked, Qt::UserRole);
		}
	}

	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void AngKCustomDelegate::setCheckBoxColumn(int _column)
{
	checkboxColumn = _column;
}

void AngKCustomDelegate::setEditColumn(int _column)
{
	editColumn = _column;
}

void AngKCustomDelegate::setCheckEnable(bool _check)
{
	bCheck = _check;
}

void AngKCustomDelegate::setExtCSDFlag(bool _bFlag)
{
	extFlag = _bFlag;
}
