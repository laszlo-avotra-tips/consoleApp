/*
 * curvesdialog.h
 *
 * non-linear grayscale adjustment UI
 *
 * Author: Chris White
 * Copyright (c) 2009-2018 Avinger, Inc.
 */
#ifndef CURVESDIALOG_H
#define CURVESDIALOG_H

#include <QDialog>
#include <QVector>
#include "ui_curvesdialog.h"

class curvesDialog : public QDialog
{
    Q_OBJECT

public:
    curvesDialog(QWidget *parent = nullptr);
    ~curvesDialog();

    QVector<unsigned char> getMap(void);
private:
    Ui::curvesDialogClass ui;
signals:
    void updateCurveMap(void);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void curveChanged();
};

#endif // CURVESDIALOG_H
