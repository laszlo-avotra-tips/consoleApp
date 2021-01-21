#ifndef FORMPMLOGO_H
#define FORMPMLOGO_H

#include <QWidget>

namespace Ui {
class FormPmLogo;
}

class FormPmLogo : public QWidget
{
    Q_OBJECT

public:
    explicit FormPmLogo(QWidget *parent = nullptr);
    ~FormPmLogo();

private:
    Ui::FormPmLogo *ui;
};

#endif // FORMPMLOGO_H
