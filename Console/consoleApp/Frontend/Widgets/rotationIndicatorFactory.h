#ifndef ROTATIONINDICATORFACTORY_H
#define ROTATIONINDICATORFACTORY_H

#include "iRotationIndicator.h"

class QGraphicsScene;
class QGraphicsItem;

class RotationIndicatorFactory
{
public:
    RotationIndicatorFactory();
    static IRotationIndicator* getRotationIndicator(QGraphicsScene* scene, QGraphicsItem* item);
private:
    static IRotationIndicator* m_rotationIndicator;
};

#endif // ROTATIONINDICATORFACTORY_H
