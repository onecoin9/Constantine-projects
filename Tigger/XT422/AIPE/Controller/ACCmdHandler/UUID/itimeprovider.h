#ifndef ITIMEPROVIDER_H
#define ITIMEPROVIDER_H

#include <QDate>

class ITimeProvider {
public:
    virtual ~ITimeProvider() = default;
    virtual QDate getCurrentDate() const = 0;
};

// 实时的具体实施
class RealTimeProvider : public ITimeProvider {
public:
    QDate getCurrentDate() const override {
        return QDate::currentDate();
    }
};

// 用于测试的模拟实施
class MockTimeProvider : public ITimeProvider {
private:
    QDate mockDate;
public:
    MockTimeProvider(const QDate& date) : mockDate(date) {}
    void setMockDate(const QDate& date) {
        mockDate = date;
    }
    QDate getCurrentDate() const override {
        return mockDate;
    }
};

#endif // ITIMEPROVIDER_H
