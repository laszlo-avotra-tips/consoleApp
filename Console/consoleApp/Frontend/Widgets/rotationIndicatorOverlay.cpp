#include "rotationIndicatorOverlay.h"
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QPointF>
#include <QGraphicsScene>

RotationIndicatorOverlay::RotationIndicatorOverlay(QGraphicsScene *scene)
    : QGraphicsTextItem( nullptr ), m_scene(scene)
{
    setTextWidth(400);
    auto font = QFont("Helvetica Neue");
    setFont(font);

    addItem();
}

RotationIndicatorOverlay::~RotationIndicatorOverlay()
{
    removeItem();
}

void RotationIndicatorOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    QFont font = painter->font();
    font.setPointSize(font.pointSize() * 2 );
    painter->setFont(font);
    painter->setPen( QPen( QBrush( QColor( 255, 215, 0 ), Qt::SolidPattern ), 2 ) );
    painter->drawText(QPointF(430,530), m_text); // " Active"
}

QString RotationIndicatorOverlay::text() const
{
    return m_text;
}

void RotationIndicatorOverlay::setText(const QString &text)
{
    m_text = text;
}

void RotationIndicatorOverlay::addItem()
{
    if(m_scene && !m_itemIsAdded){
        m_scene->addItem(this);
        setZValue(2);
        show();
        m_itemIsAdded = true;
    }
}

void RotationIndicatorOverlay::removeItem()
{
    if(m_scene && m_itemIsAdded){
        m_scene->removeItem(this);
        m_itemIsAdded = false;
    }
}
