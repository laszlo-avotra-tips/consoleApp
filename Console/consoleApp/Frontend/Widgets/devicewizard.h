/*
 * devicewizard.h
 *
 * The devicewizard is a customized qt wizard. It walks
 * the user through the device selection, configuration, and verification
 * process prior to the start of a case.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc
 */
#ifndef DEVICEWIZARD_H
#define DEVICEWIZARD_H

#include <QWizard>
#include "livescene.h"

namespace Ui {
    class deviceWizard;
}

class deviceWizard : public QWizard {
    Q_OBJECT

public:
    deviceWizard(QWidget *parent = nullptr);
    ~deviceWizard();
    void init( void );

public slots:

signals:

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *);

private slots:

private:
    Ui::deviceWizard *ui;
};

#endif // DEVICEWIZARD_H
