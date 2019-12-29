/*
 * curvesdialog.h
 *
 * non-linear grayscale adjustment UI
 *
 * Author: Chris White
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#include "curvesdialog.h"

curvesDialog::curvesDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    connect(ui.widget, SIGNAL(notifyCurveChanged()), this, SLOT(curveChanged()));
}

curvesDialog::~curvesDialog()
{

}

QVector<unsigned char> curvesDialog::getMap(void)
{
    return(ui.widget->getMap());
}

void curvesDialog::on_buttonBox_rejected()
{
    emit rejected();
}

void curvesDialog::on_buttonBox_accepted()
{
    emit accepted();
}

void curvesDialog::curveChanged(void)
{
    emit updateCurveMap();
}
