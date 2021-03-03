#include "activeLabel.h"
#include "logger.h"

ActiveLabel::ActiveLabel(QWidget* parent) : QLabel(parent)
{

}

void ActiveLabel::mouseReleaseEvent(QMouseEvent *)
{
    const auto& name = text();
    emit labelSelected(name);
    emit labelItemSelected(this);
    LOG1(name);
}

void ActiveLabel::unmark()
{
    setStyleSheet("color: white");
}

bool ActiveLabel::isSelected() const
{
    return m_isSelected;
}

void ActiveLabel::setIsSelected(bool isSelected)
{
    m_isSelected = isSelected;
}

void ActiveLabel::mark()
{
    setStyleSheet("color: rgb(245,196,0)");
}
