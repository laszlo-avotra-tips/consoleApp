#include "forml300.h"
#include "ui_forml300.h"

FormL300::FormL300(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormL300)
{
    ui->setupUi(this);
    m_liveScene = new liveScene(parent);
}

FormL300::~FormL300()
{
    delete ui;
    delete m_liveScene;
}

void FormL300::on_pushButton_clicked()
{
    hide();
}
