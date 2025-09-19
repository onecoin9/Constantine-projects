#pragma once

#include "AngKDialog.h"
namespace Ui { class ACAutomaticSetting; };

class ACAutomaticSetting : public AngKDialog
{
	Q_OBJECT

public:
	ACAutomaticSetting(QWidget *parent = Q_NULLPTR);
	~ACAutomaticSetting();

	void InitText();

	void InitButton();
	bool GetLoadStatus();

	bool IsPluginLoad() { return m_bPluginLoaded; };

signals:
	void sgnAutoTellDevReady(int);
	void sgnClearLotData(int);

	void sgnCheckSitesTskStatus();

protected:
	void setLoadStatus(bool status);

public slots:
	void SetSitesTskPass(bool status) { m_bSitesTskPass = status; };

	void onSlotLoadAutoSetting();

	void onSlotTskPathSetting();

	void onSlotAutomicOver();
private:
	Ui::ACAutomaticSetting *ui;
	QString m_strRecordProjPath;

	QTimer* m_timeOutTimer;
	QTimer* m_textChangeTimer;
	bool m_bLoading;
	bool m_bPluginLoaded;

	bool m_bSitesTskPass;
};
