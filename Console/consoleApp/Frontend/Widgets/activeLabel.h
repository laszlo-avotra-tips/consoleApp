#ifndef ACTIVELABEL_H
#define ACTIVELABEL_H

#include <QLabel>

class ActiveLabel : public QLabel
{
    Q_OBJECT

public:
    ActiveLabel(QWidget* parent);
    void mouseReleaseEvent(QMouseEvent*) override;

signals:
    void labelSelected(const QString& name);

};

#endif // ACTIVELABEL_H
