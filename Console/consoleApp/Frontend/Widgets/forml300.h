#ifndef FORML300_H
#define FORML300_H

#include <QWidget>
#include <sectoritem.h>
#include <livescene.h>
#include <QParallelAnimationGroup>
#include <vector>


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
    liveScene* scene();

private slots:
    void on_pushButton_clicked();
    void on_pushButtonAvinger_toggled(bool checked);

    void on_pushButtonZoomOut_clicked();

    void on_pushButtonZoomIn_clicked();

    void on_horizontalSliderZoom_valueChanged(int value);

private:
    void animateHide(QWidget* wid, QParallelAnimationGroup* group);
    void animateShow(QWidget* wid, QParallelAnimationGroup* group, float opacity=0.5f);
    void hideButtons();

private:
    Ui::FormL300 *ui;
    sectorItem* m_sector{nullptr};
    QGraphicsView* m_graphicsView{nullptr};
    liveScene* m_scene{nullptr};
    const std::vector<int> m_imagingDepth{600,525,450,375,300};
};

#endif // FORML300_H
