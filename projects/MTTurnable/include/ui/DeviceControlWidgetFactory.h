#ifndef DEVICECONTROLWIDGETFACTORY_H
#define DEVICECONTROLWIDGETFACTORY_H

#include <memory>
#include <QWidget>
#include "domain/IDevice.h"

namespace Presentation {

class IDeviceControlWidget;

/**
 * @brief 设备控制界面工厂
 * 根据设备类型创建对应的控制界面
 */
class DeviceControlWidgetFactory
{
public:
    /**
     * @brief 创建设备控制界面
     * @param deviceType 设备类型
     * @param parent 父窗口
     * @return 设备控制界面指针，如果不支持该类型则返回nullptr
     */
    static std::unique_ptr<IDeviceControlWidget> createControlWidget(
        Domain::IDevice::DeviceType deviceType, 
        QWidget *parent = nullptr);
    
    /**
     * @brief 检查是否支持该设备类型
     * @param deviceType 设备类型
     * @return true表示支持，false表示不支持
     */
    static bool isSupported(Domain::IDevice::DeviceType deviceType);
    
    /**
     * @brief 获取支持的设备类型列表
     * @return 设备类型列表
     */
    static QList<Domain::IDevice::DeviceType> getSupportedTypes();
};

} // namespace Presentation

#endif // DEVICECONTROLWIDGETFACTORY_H 