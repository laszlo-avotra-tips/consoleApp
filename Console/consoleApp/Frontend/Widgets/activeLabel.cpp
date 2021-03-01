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