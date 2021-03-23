#include "activeLabel.h"
#include "logger.h"

ActiveLabel::ActiveLabel(QWidget* parent) : QLabel(parent)
{

}

void ActiveLabel::mouseReleaseEvent(QMouseEvent *)
{
    if(!isMarked()){
        mark();
        emit labelItemMarked(this);
     } else {
        unmark();
        emit labelItemUnmarked();
    }
}

void ActiveLabel::unmark()
{
    m_isMarked = false;
    setStyleSheet("color: white");
}

bool ActiveLabel::isMarked() const
{
    return m_isMarked;
}

void ActiveLabel::setIsMarked(bool isMarked)
{
    m_isMarked = isMarked;
}

void ActiveLabel::mark()
{
    if(!text().isEmpty()){
        m_isMarked = true;
        setStyleSheet("color: rgb(245,196,0)");
    }
}
