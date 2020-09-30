#ifndef ROTATIONINDICATOROVERLAY2_H
#define ROTATIONINDICATOROVERLAY2_H

#include <QGraphicsItem>

class liveScene;

class RotationIndicatorOverlay2 : QGraphicsItem
{
public:
    RotationIndicatorOverlay2(liveScene *scene);
    ~RotationIndicatorOverlay2();

    void addItem();
    void removeItem();
    void setText(const QString &text);
    void showItem(bool);

private:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;

    liveScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY2_H
