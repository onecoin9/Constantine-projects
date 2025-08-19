#pragma once

#include <QDialog>
#include <QToolButton>
namespace Ui { class AngKSettingDlg; };


namespace AcroView
{

	struct ProgramInfo {
		int sktModeIdx;
		QString sktPercent;
		QString sktAbsolute;
	};


	class AngKSettingDlg : public QDialog
	{
		Q_OBJECT

	public:
		AngKSettingDlg(QWidget* parent = Q_NULLPTR);
		~AngKSettingDlg();

		void InitText();

		void InitButton();

		void InitPage();

		void OpenActionPage(int nIndex);

		void InitProgramPage();

		void InitFactoryMode();

		void InitLanguage();

		void InitLoginPassword();

		void InitTaskProjectPage();

		void ReBackfillSetting();
	protected:
		virtual void mousePressEvent(QMouseEvent* event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);

	private:
		int calEnableFactory();

		void setEnableFactory(int calValue);

		void saveProgramInfo();
	public slots:
		void onSlotToolButtonClick(bool state);

		void onSlotSaveConfig();

		void onSlotProgramSetting(int, int);

		void onSlotSktComboBoxSwitch(int);
	private:
		Ui::AngKSettingDlg* ui;
		std::vector<QToolButton*>	m_vecToolButton;
		int							m_curClickButton;
		QButtonGroup*				m_screenGroup;
		QButtonGroup*				m_languageGroup;
		QPoint						m_clickPos;
		int							m_nDraggableHeight;   // 可拖动区域的高度
		bool						m_bDragging; // 拖动状态标志
		int							m_curSktMode;
		int							m_curSktValue;

		ProgramInfo					m_programInfo;
	};
}