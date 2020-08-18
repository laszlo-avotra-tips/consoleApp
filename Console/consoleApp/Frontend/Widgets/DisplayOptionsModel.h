#ifndef DISPLAYOPTIONSMODEL_H
#define DISPLAYOPTIONSMODEL_H

#include <QObject>

class DisplayOptionsModel
{
public:
    static DisplayOptionsModel* instance();

private:
    DisplayOptionsModel();
    static DisplayOptionsModel* m_instance;
};

#endif // DISPLAYOPTIONSMODEL_H
