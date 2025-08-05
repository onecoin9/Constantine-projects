#pragma once

#include <QtWidgets/QWidget>
#include "GlobalDefine.h"
namespace Ui { class AngKTranslateSelect; };

namespace AcroView
{
	class AngKTranslateSelect : public QWidget
	{
		Q_OBJECT

	public:
		AngKTranslateSelect(QWidget* parent = Q_NULLPTR, bool bDark = false);
		~AngKTranslateSelect();

		void InitShadow();

		void setCheckState(TranslateLanguage nType);
	signals:
		void sgnRestartClient();

	public slots:
		void onSlotChangeCheckState(bool);
	private:
		Ui::AngKTranslateSelect* ui;
		TranslateLanguage	m_curSelectLanguage;
	};
}