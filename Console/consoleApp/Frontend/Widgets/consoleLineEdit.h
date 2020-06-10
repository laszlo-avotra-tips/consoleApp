#ifndef CONSOLELINEEDIT_H
#define CONSOLELINEEDIT_H

#include <QObject>
#include <QLineEdit>

class ConsoleLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    ConsoleLineEdit(QWidget* parent = nullptr);
    void mousePressEvent(QMouseEvent * e) override;

signals:
    void mousePressed();

};

#endif // CONSOLELINEEDIT_H
