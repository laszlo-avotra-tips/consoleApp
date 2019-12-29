#ifndef DAQFACTORY_H
#define DAQFACTORY_H

class IDAQ;

class daqfactory
{
public:
    static daqfactory* instance();
    IDAQ* getdaq();
    IDAQ *getProxy();

private:
    daqfactory();
    IDAQ* idaq;
    IDAQ* proxy;
    static daqfactory* factory;
};

#endif // DAQFACTORY_H
