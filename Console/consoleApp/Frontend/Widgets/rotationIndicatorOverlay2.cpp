#include "rotationIndicatorOverlay2.h"

#include <QGraphicsScene>
#include <QPainter>
#include "trigLookupTable.h"
#include "deviceSettings.h"
#include "logger.h"
#include "depthsetting.h"
#include "sledsupport.h"


RotationIndicatorOverlay2::RotationIndicatorOverlay2(QGraphicsScene *scene)
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

void RotationIndicatorOverlay2::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    trigLookupTable &quickTrig = trigLookupTable::Instance();

    // Update our reference line position
    const int x1 = 1080;
    const int y1 = 1080;
    const double degToRad(3.1415/180.0);

    double currentAngle_deg{0.0};
    deviceSettings &devSettings = deviceSettings::Instance();
    auto currentAlineLength_px = devSettings.current()->getALineLengthNormal_px();
    auto catheterRadius_px     = devSettings.current()->getCatheterRadius_px();

    // Draw the laser reference line for low speed devices
    if( devSettings.current()->isBiDirectional() )
    {
        double tmpAngle_rad = degToRad * currentAngle_deg;
        LOG1(tmpAngle_rad)

        float cosTheta = quickTrig.lookupCos( (double)tmpAngle_rad );
        float sinTheta = quickTrig.lookupSin( (double)tmpAngle_rad );

        // calculate the endpoint of the marker line
        int x = x1 + floor_int( ( cosTheta * (double)( currentAlineLength_px + catheterRadius_px ) + 0.5 ) );
        int y = y1 + floor_int( ( sinTheta * (double)( currentAlineLength_px + catheterRadius_px ) + 0.5 ) );

        // Draw the reference line
        painter->drawLine( x1, y1, x, y );
    }
    QPen directionPen = QPen( QColor( 0, 0, 0 ), 6, Qt::SolidLine, Qt::RoundCap );

    depthSetting &depth = depthSetting::Instance();
    int catheterEdgePosition = depth.getCatheterEdgePosition();

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
