#ifndef ROTATIONINDICATOROVERLAY_H
#define ROTATIONINDICATOROVERLAY_H

#include <QGraphicsTextItem>

class QGraphicsScene;

class RotationIndicatorOverlay : public QGraphicsTextItem
{
public:
    RotationIndicatorOverlay(QGraphicsScene *scene);
    ~RotationIndicatorOverlay();

    QString text() const;
    void setText(const QString &text);
    void addItem();
    void removeItem();

private:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;

    QGraphicsScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY_H
