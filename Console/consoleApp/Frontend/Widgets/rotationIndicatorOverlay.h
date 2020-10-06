#ifndef ROTATIONINDICATOROVERLAY_H
#define ROTATIONINDICATOROVERLAY_H

#include <QGraphicsTextItem>

class QGraphicsScene;

struct IRotationIndicator : public QGraphicsTextItem
{
    virtual void setText(const QString &text) = 0;
    virtual void addItem() = 0;
    virtual void removeItem() = 0;
};

class RotationIndicatorOverlay : public IRotationIndicator
//        class RotationIndicatorOverlay : public QGraphicsTextItem
{
public:
    static RotationIndicatorOverlay* instance(QGraphicsScene* scene = nullptr, QGraphicsItem* parent = nullptr);
    ~RotationIndicatorOverlay();

    QString text() const;
    void setText(const QString &text) override;
    void addItem() override;
    void removeItem() override;

private:
    RotationIndicatorOverlay(QGraphicsScene *scene, QGraphicsItem* parent);
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    QRectF boundingRect() const override;

    static RotationIndicatorOverlay* m_instance;
    QGraphicsScene* m_scene{nullptr};
    QString m_text;
    bool m_itemIsAdded{false};
};

#endif // ROTATIONINDICATOROVERLAY_H
