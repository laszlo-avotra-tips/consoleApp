#ifndef DAQFACTORY_H
#define DAQFACTORY_H

class IDAQ;

class daqfactory
{
public:
    static daqfactory* instance();
    IDAQ* getdaq();

private:
    daqfactory();
    IDAQ* idaq;
    static daqfactory* factory;
};

#endif // DAQFACTORY_H
