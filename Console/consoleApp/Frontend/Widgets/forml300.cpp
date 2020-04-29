#include "forml300.h"
#include "ui_forml300.h"

FormL300::FormL300(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormL300)
{
    ui->setupUi(this);
    m_sector = new sectorItem();
    m_scene = new QGraphicsScene(this);
    m_graphicsView = ui->graphicsView;
    m_scene->addItem(m_sector);
    m_graphicsView->setScene(m_scene);
}

FormL300::~FormL300()
{
    delete ui;
    delete m_sector;
}

QGraphicsPixmapItem *FormL300::sectorHandle()
{
    return m_sector;
}

QImage *FormL300::sectorImage() const
{
    return m_sector->getSectorImage();
}

bool FormL300::isVisible() const
{
    const auto& region = m_graphicsView->visibleRegion();
    return !region.isEmpty();
}

void FormL300::on_pushButton_clicked()
{
    hide();
}
