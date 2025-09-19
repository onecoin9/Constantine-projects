#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <QObject>
#include <QQueue>
#include <mutex>

#include "notify.h"

class NotifyManager : public QObject
{
    Q_OBJECT
public:
	static NotifyManager& instance()
	{
		if (!s_instance)
		{
			std::call_once(once_flag, []() { s_instance = new NotifyManager(); });
		}
		return *s_instance;
	}

    void notify(const QString &title, const QString &body, const QString &icon = "Skin/Light/Common/message.png", const QString url = "");
    void setMaxCount(int count);
    void setDisplayTime(int ms);

private:
    explicit NotifyManager(QObject* parent = 0);
	static NotifyManager* s_instance;
	static std::once_flag once_flag;

private:
    class NotifyData {
    public:
        NotifyData(const QString &icon, const QString &title, const QString &body, const QString url):
          icon(icon),
          title(title),
          body(body),
          url(url)
        {
        }

        QString icon;
        QString title;
        QString body;
        QString url;
    };

    void rearrange();
    void showNext();

    QQueue<NotifyData> dataQueue;
    QList<Notify*> notifyList;
    int maxCount;
    int displayTime;
};

#endif // NOTIFYMANAGER_H
