#ifndef CONSOLELABEL_H
#define CONSOLELABEL_H

#include <QObject>
#include <QLabel>

class ConsoleLabel : public QLabel
{
    Q_OBJECT

public:
    ConsoleLabel(QWidget* parent = nullptr);
    void mousePressEvent(QMouseEvent * e) override;

signals:
    void mousePressed();

};

#endif // CONSOLELABEL_H
