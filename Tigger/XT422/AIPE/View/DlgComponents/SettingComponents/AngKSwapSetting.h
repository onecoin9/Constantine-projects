#pragma once

#include "AngKDialog.h"
namespace Ui { class AngKSwapSetting; };

class QButtonGroup;
class AngKSwapSetting : public AngKDialog
{
	Q_OBJECT

public:
	enum SwapMode {
		NoMode = 1,
		ByteMode,
		WordMode,
		BWMode
	};

	AngKSwapSetting(QWidget *parent = Q_NULLPTR);
	~AngKSwapSetting();

	void InitText();

	SwapMode GetSwapMode() { return m_nSwapMode; }

	static QString GetSwapname(SwapMode modeName);
private:
	Ui::AngKSwapSetting *ui;
	QButtonGroup* m_swapGroup;
	SwapMode	m_nSwapMode;
};
