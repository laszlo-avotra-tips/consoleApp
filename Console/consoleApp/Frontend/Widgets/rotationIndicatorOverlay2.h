#ifndef ROTATIONINDICATOROVERLAY2_H
#define ROTATIONINDICATOROVERLAY2_H

//#include <QGraphicsItem>
#include "iRotationIndicator.h"

class QGraphicsScene;

class RotationIndicatorOverlay2 : public IRotationIndicator
{
public:
    static RotationIndicatorOverlay2* instance(QGraphicsScene* scene = nullptr, QGraphicsItem* parent = nullptr);
    ~RotationIndicatorOverlay2();

    void addItem() override;
    void removeItem() override;
    void setText(const QString &text) override;

private:
    RotationIndicatorOverlay2(QGraphicsScene *scene, QGraphicsItem*);
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;

    static RotationIndicatorOverlay2* m_instance;
    QGraphicsScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY2_H
