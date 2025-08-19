#include "presentation/DeviceControlWidgetFactory.h"
#include "presentation/IDeviceControlWidget.h"
#include "presentation/TestBoardControlWidget.h"
#include "presentation/HandlerControlWidget.h"

namespace Presentation {

std::unique_ptr<IDeviceControlWidget> DeviceControlWidgetFactory::createControlWidget(
    Domain::IDevice::DeviceType deviceType, 
    QWidget *parent)
{
    switch (deviceType) {
        case Domain::IDevice::DeviceType::Handler:
            return std::make_unique<HandlerControlWidget>(parent);
                      
        case Domain::IDevice::DeviceType::TestBoard:
            return std::make_unique<TestBoardControlWidget>(parent);
            
        // 未来可以添加更多设备类型
        // case Domain::IDevice::DeviceType::PowerSupply:
        //     return std::make_unique<PowerSupplyControlWidget>(parent);
            
        default:
            return nullptr;
    }
}

bool DeviceControlWidgetFactory::isSupported(Domain::IDevice::DeviceType deviceType)
{
    switch (deviceType) {
        case Domain::IDevice::DeviceType::Handler:
        case Domain::IDevice::DeviceType::Turntable:
        case Domain::IDevice::DeviceType::TestBoard:
            return true;
        default:
            return false;
    }
}

QList<Domain::IDevice::DeviceType> DeviceControlWidgetFactory::getSupportedTypes()
{
    return {
        Domain::IDevice::DeviceType::Handler,
        Domain::IDevice::DeviceType::Turntable,
        Domain::IDevice::DeviceType::TestBoard
    };
}

} // namespace Presentation 
