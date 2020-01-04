/*
 * lagwizard.h
 *
 * The lagwizard is a customized qt wizard. It walks
 * the user through the lag correction process, taking
 * reference images and allowing alignment and verification.
 *
 * Author: Chris White
 *
 * Copyright (c) 2010-2018 Avinger, Inc.
 */
#ifndef LAGWIZARD_H
#define LAGWIZARD_H

#include <QWizard>
#include <QGraphicsScene>
#include "livescene.h"

namespace Ui {
    class lagWizard;
}

class lagWizard : public QWizard {
    Q_OBJECT
public:
    lagWizard(QWidget *parent = nullptr);
    ~lagWizard();

    void setScene( QGraphicsScene *scene );
    double getAngle( void ) { return angle; }

public slots:
    void handleFullRotation( void );
    void handleDirectionChange( void );

signals:
    void goToNext( void );
    void resetIntegrationAngle( void );

protected:
    void changeEvent(QEvent *e);

private:
    liveScene *theScene;
    Ui::lagWizard *ui;
    double angle;
    QPainter *p;
    QImage *leftImage;
    QImage *rightImage;
    void initializePage(int id);
    bool seenDirectionChange;
    bool seenFirstRotation;
    bool seenSecondRotation;
    bool waitingForFirstCapture;

private slots:
    void on_angleSlider_valueChanged(int value);
};

#endif // LAGWIZARD_H
