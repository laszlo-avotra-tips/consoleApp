#ifndef ACTIVELABEL_H
#define ACTIVELABEL_H

#include <QLabel>

class ActiveLabel : public QLabel
{
    Q_OBJECT

public:
    ActiveLabel(QWidget* parent);
    void mouseReleaseEvent(QMouseEvent*) override;
    void mark();
    void unmark();

signals:
    void labelSelected(const QString& name);

};

#endif // ACTIVELABEL_H
