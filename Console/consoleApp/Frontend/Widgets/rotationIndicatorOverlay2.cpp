#include "rotationIndicatorOverlay2.h"

#include <QGraphicsScene>
#include <QPainter>
#include "trigLookupTable.h"
#include "deviceSettings.h"
#include "logger.h"
#include "depthsetting.h"
#include "sledsupport.h"
#include "Utility/userSettings.h"
#include "livescene.h"


RotationIndicatorOverlay2::RotationIndicatorOverlay2(liveScene *scene)
: QGraphicsItem( nullptr ), m_scene(scene)
{
    addItem();
}

RotationIndicatorOverlay2::~RotationIndicatorOverlay2()
{
    removeItem();
}

void RotationIndicatorOverlay2::addItem()
{
    if(m_scene && !m_itemIsAdded){
        m_scene->addItem(this);
        setZValue(2);
        show();
        m_itemIsAdded = true;
    }
}

void RotationIndicatorOverlay2::removeItem()
{
    if(m_scene && m_itemIsAdded){
        m_scene->removeItem(this);
        m_itemIsAdded = false;
    }
}

void RotationIndicatorOverlay2::setText(const QString &text)
{
    m_text = text;
}

void RotationIndicatorOverlay2::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    // Update our reference line position
    const int x1 = 1024 / 2;
    const int y1 = 1024 / 2;

    deviceSettings &devSettings = deviceSettings::Instance();

    depthSetting &depth = depthSetting::Instance();
    int catheterEdgePosition = depth.getCatheterEdgePosition();
    QBrush cathEdgeBrush( Qt::black, Qt::SolidPattern );
    QColor reticleColor( 50, 50, 255, userSettings::Instance().reticleBrightness() );
    QPen   reticlePen( QPen( reticleColor, 3, Qt::SolidLine, Qt::RoundCap) );
    painter->setPen( reticlePen );
    painter->setBrush( cathEdgeBrush );
    painter->drawEllipse( QRect( QPoint( x1 - catheterEdgePosition, y1 - catheterEdgePosition ),
                                 QPoint( x1 + catheterEdgePosition, y1 + catheterEdgePosition ) ) );

    QPen directionPen = QPen( QColor( 0, 0, 0 ), 6, Qt::SolidLine, Qt::RoundCap );


    // Direction indicator is only drawn when live.   TBD: the sector should not care about playback or not
    if( devSettings.current()->isBiDirectional() )
    {
        painter->setPen( directionPen );

        // draw direction indicator
        const int DirectionEdge = catheterEdgePosition / 2;

        painter->drawEllipse( QRect( QPoint( x1 - DirectionEdge, y1 - DirectionEdge ),
                                     QPoint( x1 + DirectionEdge, y1 + DirectionEdge ) ) );
    }

    // Direction indicator for highspeed bidirectional devices (Ocelaris)
    if(devSettings.current()->isBiDirectional() )
    {
        QString spin;
        QFont font;
        font.setPixelSize(catheterEdgePosition / 3);
        font.setBold(true);
        painter->setFont(font);

        auto& sled = SledSupport::Instance();

        int lastRunningState = sled.lastRunningState(); //dev.current()->getRotation();
        if(lastRunningState == 3)
        {
            painter->setBrush( PassiveSpinColor );
            spin = "PASSIVE";
        }
        else if(lastRunningState == 1)
        {
            painter->setBrush( AggressiveSpinColor );
            spin = "ACTIVE";
        }
        else{
            painter->setBrush(Qt::black);
        }
        // draw direction indicator
        const int DirectionEdge = catheterEdgePosition;
        QRect center(QRect( QPoint( x1 - DirectionEdge, y1 - DirectionEdge ),
                            QPoint( x1 + DirectionEdge, y1 + DirectionEdge ) ) );

        painter->drawEllipse( center );
        painter->setPen( Qt::black );
        painter->drawText( center, Qt::AlignCenter, spin );
    }

//    setPixmap( tmpPixmap );
    painter->end();

}

QRectF RotationIndicatorOverlay2::boundingRect() const
{
    return m_scene->sceneRect();
}
