#include "activeLabel.h"
#include "logger.h"

ActiveLabel::ActiveLabel(QWidget* parent) : QLabel(parent)
{

}

void ActiveLabel::mouseReleaseEvent(QMouseEvent *)
{
    const auto& name = text();
    emit labelSelected(name);
    LOG1(name);
}

void ActiveLabel::unmark()
{
    setStyleSheet("color: white");
}

void ActiveLabel::mark()
{
    setStyleSheet("color: rgb(245,196,0)");
}
