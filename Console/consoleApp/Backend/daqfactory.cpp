#include "daqfactory.h"
#include "deviceSettings.h"
#include "daqSettings.h"
#include "filedaq.h"
#include "logger.h"
#include "daq.h"

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

        if(deviceName == "Simulation"){
            idaq = new FileDaq();
            setting.setIsSimulation(true);
        } else {
            idaq = new DAQ();
            setting.setIsSimulation(false);
        }
    }

    return idaq;
}
