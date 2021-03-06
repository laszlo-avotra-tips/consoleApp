#include "daqfactory.h"
#include "deviceSettings.h"
#include "logger.h"
#include "daq.h"
#include "mainScreen.h"

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

IDAQ *daqfactory::getdaq(MainScreen* ms)
{
    if( !idaq )
    {
        deviceSettings &setting = deviceSettings::Instance();
        auto currentDevice = setting.current();
        auto deviceName = currentDevice->getDeviceName();

        LOG1(deviceName)

        idaq = new DAQ(ms);
        setting.setIsDeviceSimulation(false);
    }

    return idaq;
}

IDAQ *daqfactory::getdaq()
{
    return idaq;
}
