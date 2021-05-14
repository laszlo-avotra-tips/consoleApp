#ifndef DAQFACTORY_H
#define DAQFACTORY_H

class IDAQ;
class MainScreen;

class daqfactory
{
public:
    static daqfactory* instance();
    IDAQ* getdaq(MainScreen *ms);
    IDAQ* getdaq();

private:
    daqfactory();
    IDAQ* idaq;
    static daqfactory* factory;
};

#endif // DAQFACTORY_H
