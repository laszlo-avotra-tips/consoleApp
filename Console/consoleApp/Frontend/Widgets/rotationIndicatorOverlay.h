#ifndef ROTATIONINDICATOROVERLAY_H
#define ROTATIONINDICATOROVERLAY_H

#include <QGraphicsTextItem>

class QGraphicsScene;

class RotationIndicatorOverlay : public QGraphicsTextItem
{
public:
    static RotationIndicatorOverlay* instance(QGraphicsScene* scene, QGraphicsItem* parent);
    ~RotationIndicatorOverlay();

    QString text() const;
    void setText(const QString &text);
    void addItem();
    void removeItem();

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    RotationIndicatorOverlay(QGraphicsScene *scene, QGraphicsItem* parent);
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    static RotationIndicatorOverlay* m_instance;
    QGraphicsScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY_H
