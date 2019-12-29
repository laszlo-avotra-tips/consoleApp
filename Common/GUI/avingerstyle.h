#ifndef AVINGERSTYLE_H
#define AVINGERSTYLE_H

#include <QProxyStyle>

class avingerStyle : public QProxyStyle
{
    Q_OBJECT
public:
    void drawControl ( ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;

signals:

public slots:

};

#endif // AVINGERSTYLE_H
