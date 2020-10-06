#ifndef IROTATIONINDICATOR_H
#define IROTATIONINDICATOR_H

#include <QGraphicsItem>

class QString;

struct IRotationIndicator : public QGraphicsItem
{
    virtual void setText(const QString &text) = 0;
    virtual void addItem() = 0;
    virtual void removeItem() = 0;
};

#endif // IROTATIONINDICATOR_H
