#ifndef ROTATIONINDICATOROVERLAY2_H
#define ROTATIONINDICATOROVERLAY2_H

#include <QGraphicsItem>

class QGraphicsScene;

class RotationIndicatorOverlay2 : QGraphicsItem
{
public:
    RotationIndicatorOverlay2(QGraphicsScene *scene);
    ~RotationIndicatorOverlay2();

    void addItem();
    void removeItem();

private:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    QGraphicsScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY2_H
