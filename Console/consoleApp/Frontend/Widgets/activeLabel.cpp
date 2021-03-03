#include "activeLabel.h"
#include "logger.h"

ActiveLabel::ActiveLabel(QWidget* parent) : QLabel(parent)
{

}

void ActiveLabel::mouseReleaseEvent(QMouseEvent *)
{
    const auto& name = text();
    if(!isMarked()){
        mark();
        emit labelItemSelected(this);
    } else {
        unmark();
    }
    LOG1(name);
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
    m_isMarked = true;
    setStyleSheet("color: rgb(245,196,0)");
}
