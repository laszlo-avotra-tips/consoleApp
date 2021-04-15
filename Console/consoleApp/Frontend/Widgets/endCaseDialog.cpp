#include "endCaseDialog.h"
#include "ui_endCaseDialog.h"

endCaseDialog::endCaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::endCaseDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::SplashScreen);
    setWindowFlags( windowFlags() & Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowTitleHint );

}

endCaseDialog::~endCaseDialog()
{
    delete ui;
}
