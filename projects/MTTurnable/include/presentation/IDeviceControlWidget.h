#ifndef IDEVICECONTROLWIDGET_H
#define IDEVICECONTROLWIDGET_H

#include <QWidget>
#include <memory>
#include <QString>
#include <QDebug>

namespace Domain {
class IDevice;
}

namespace Presentation {

/**
 * @brief 设备控制界面接口
 * 每种设备类型都应该实现自己的控制界面
 */
class IDeviceControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IDeviceControlWidget(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~IDeviceControlWidget() = default;
    
    /**
     * @brief 设置要控制的设备
     * @param device 设备指针
     */
    virtual void setDevice(std::shared_ptr<Domain::IDevice> device) = 0;
    
    /**
     * @brief 获取当前控制的设备
     * @return 设备指针
     */
    virtual std::shared_ptr<Domain::IDevice> getDevice() const = 0;
    
    /**
     * @brief 更新界面状态
     * 根据设备的当前状态更新UI
     */
    virtual void updateStatus() = 0;
    
    /**
     * @brief 获取设备类型名称
     * @return 设备类型的显示名称
     */
    virtual QString getDeviceTypeName() const = 0;
    
    /**
     * @brief 设置是否启用控制
     * @param enabled true启用，false禁用
     */
    virtual void setControlsEnabled(bool enabled) = 0;

signals:
    /**
     * @brief 设备状态发生变化时发出
     */
    void deviceStatusChanged();
    
    /**
     * @brief 发生错误时发出
     * @param error 错误信息
     */
    void errorOccurred(const QString &error);
};

} // namespace Presentation

#endif // IDEVICECONTROLWIDGET_H 