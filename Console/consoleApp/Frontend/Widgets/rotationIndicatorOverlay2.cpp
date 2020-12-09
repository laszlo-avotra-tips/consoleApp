#include "rotationIndicatorOverlay2.h"

#include <QGraphicsScene>
#include <QPainter>
#include "trigLookupTable.h"
#include "deviceSettings.h"
#include "logger.h"
#include "depthsetting.h"
#include "sledsupport.h"

RotationIndicatorOverlay2* RotationIndicatorOverlay2::m_instance{nullptr};


RotationIndicatorOverlay2::RotationIndicatorOverlay2(QGraphicsScene *scene, QGraphicsItem *)
    : m_scene(scene)
{
    addItem();
    hide();
}

RotationIndicatorOverlay2* RotationIndicatorOverlay2::instance(QGraphicsScene *scene, QGraphicsItem* parent)
{
    if(!m_instance){
        m_instance = new RotationIndicatorOverlay2(scene, parent);
    }
    return m_instance;
}


RotationIndicatorOverlay2::~RotationIndicatorOverlay2()
{
    removeItem();
}

void RotationIndicatorOverlay2::addItem()
{
    if(m_scene && !m_itemIsAdded){
        m_scene->addItem(this);
        setZValue(200);
        m_itemIsAdded = true;
        show();
    }
}

void RotationIndicatorOverlay2::removeItem()
{
    if(m_scene && m_itemIsAdded){
//        m_scene->removeItem(this);
//        m_itemIsAdded = false;
        hide();
    }
}

void RotationIndicatorOverlay2::setText(const QString &text)
{
    m_text = text;
}

void RotationIndicatorOverlay2::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget * /*widget*/)
{
    deviceSettings &devSettings = deviceSettings::Instance();
    depthSetting::Instance().calculateReticles();

    // Update our reference line position
    const int x1 = 512;
    const int y1 = 512;

    trigLookupTable &quickTrig = trigLookupTable::Instance();
    const double degToRad(3.1415/180.0);

    double currentAngle_deg{0.0};
    auto currentAlineLength_px = devSettings.current()->getALineLength_px();
    int catheterRadius_px     = devSettings.current()->getCatheterRadius_px();

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

//    // Direction indicator is only drawn when live.   TBD: the sector should not care about playback or not
//    if( devSettings.current()->isBiDirectional() )
//    {
//        painter->setPen( directionPen );

//        // draw direction indicator
//        const int DirectionEdge = catheterEdgePosition / 2;

//        LOG1(DirectionEdge)

//        painter->drawEllipse( QRect( QPoint( x1 - DirectionEdge, y1 - DirectionEdge ),
//                                     QPoint( x1 + DirectionEdge, y1 + DirectionEdge ) ) );
//    }

    // Direction indicator for highspeed bidirectional devices (Ocelaris)
    if(devSettings.current()->isBiDirectional() )
    {
        QString spin;
        QFont font;
        static int redrawCount = 0;
        font.setPixelSize(catheterEdgePosition / 3);
//        font.setBold(true);
        painter->setFont(font);

        auto& sled = SledSupport::Instance();

        int lastRunningState = sled.getLastRunningState(); //dev.current()->getRotation();
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

        if(redrawCount++ % 64 == 0){
            LOG1(DirectionEdge)
        }
    }

    painter->end();
}

QRectF RotationIndicatorOverlay2::boundingRect() const
{
    return m_scene->sceneRect();
}
