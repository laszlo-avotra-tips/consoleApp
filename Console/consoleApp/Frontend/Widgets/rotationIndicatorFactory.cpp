#include "rotationIndicatorFactory.h"
#include "rotationIndicatorOverlay.h"

IRotationIndicator* RotationIndicatorFactory::m_rotationIndicator{nullptr};

RotationIndicatorFactory::RotationIndicatorFactory()
{

}

IRotationIndicator *RotationIndicatorFactory::getRotationIndicator(QGraphicsScene* scene, QGraphicsItem* item)
{
    if(!m_rotationIndicator) {
        m_rotationIndicator = RotationIndicatorOverlay::instance(scene, item);
    }
    return m_rotationIndicator;
}
