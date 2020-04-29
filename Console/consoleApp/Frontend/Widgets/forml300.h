#ifndef FORML300_H
#define FORML300_H

#include <QWidget>
#include <livescene.h>

namespace Ui {
class FormL300;
}

class FormL300 : public QWidget
{
    Q_OBJECT

public:
    explicit FormL300(QWidget *parent = nullptr);
    ~FormL300();

private slots:
    void on_pushButton_clicked();

private:
    Ui::FormL300 *ui;
    liveScene* m_liveScene{nullptr};
};

#endif // FORML300_H
