#ifndef FORML300_H
#define FORML300_H

#include <QWidget>
#include <sectoritem.h>
#include <QGraphicsScene>

namespace Ui {
class FormL300;
}

class FormL300 : public QWidget
{
    Q_OBJECT

public:
    explicit FormL300(QWidget *parent = nullptr);
    ~FormL300();
    QGraphicsPixmapItem *sectorHandle( void );
    QImage* sectorImage() const;
    bool isVisible() const;
    sectorItem* sector();
    QGraphicsScene* scene();

private slots:
    void on_pushButton_clicked();

private:
    Ui::FormL300 *ui;
    sectorItem* m_sector{nullptr};
    QGraphicsView* m_graphicsView{nullptr};
    QGraphicsScene* m_scene{nullptr};
};

#endif // FORML300_H
