#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QObject>
#include <QLineEdit>

class OctLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    OctLineEdit(QWidget* parent = nullptr);
    void mousePressEvent(QMouseEvent * e) override;

signals:
    void mousePressed();

};

#endif // MYLINEEDIT_H
