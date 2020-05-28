#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H

#include <QString>


class QWidget;

class PageFactory
{
    QWidget* getPage(const QString& name);

public:
    PageFactory();

};

#endif // PAGEFACTORY_H
