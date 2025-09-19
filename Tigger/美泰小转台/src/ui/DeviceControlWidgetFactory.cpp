#include "ui/DeviceControlWidgetFactory.h"
#include "ui/IDeviceControlWidget.h"
#include "ui/TestBoardControlWidget.h"
#include "ui/HandlerControlWidget.h"
#include "ui/BurnControlWidget.h"

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
            
        case Domain::IDevice::DeviceType::Burn:
            return std::make_unique<BurnControlWidget>(parent);
            
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
        case Domain::IDevice::DeviceType::Burn:
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
        Domain::IDevice::DeviceType::TestBoard,
        Domain::IDevice::DeviceType::Burn
    };
}

} // namespace Presentation 
