#ifndef IDAQPROXY_H
#define IDAQPROXY_H

#include <idaq.h>


class IDaqProxy : public IDAQ
{

public:
    IDaqProxy();

    void setIDaq(IDAQ* obj);

    void stop() override;
    void pause() override;
    void resume() override;
    void init() override;

    QString getDaqLevel() override;

    long getRecordLength() const override;

    void enableAuxTriggerAsTriggerEnable( bool ) override; //  * R&D only

     IDAQ* getSignalSource() override { return m_idaq;}

     bool configure( void ) override;
     bool getData( void ) override;

private:
    IDAQ* m_idaq;
};

#endif // IDAQPROXY_H
