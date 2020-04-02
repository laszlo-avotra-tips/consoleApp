#include "daqfactory.h"
#include "deviceSettings.h"
#include "daqSettings.h"
#include "filedaq.h"
#include "logger.h"

daqfactory* daqfactory::factory(nullptr);

daqfactory::daqfactory():idaq(nullptr)
{

}

daqfactory *daqfactory::instance()
{
    if(!factory){
        factory = new daqfactory();
    }
    return factory;
}

IDAQ *daqfactory::getdaq()
{
    if( !idaq )
    {
        deviceSettings &setting = deviceSettings::Instance();
        auto currentDevice = setting.current();
        auto deviceName = currentDevice->getDeviceName();

        LOG1(deviceName)

        if( setting.current()->isHighSpeed() )
        {
            if(deviceName == "Simulation"){
                idaq = new FileDaq();
            }
        }
    }

    return idaq;
}
