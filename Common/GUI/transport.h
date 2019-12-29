/*
 * transport.h
 *
 * Playback controls
 *
 * Author: Dennis W. Jackson, Chris White
 *
 * Copyright (c) 2011-2018 Avinger, Inc.
 */

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QWidget>

namespace Ui {
    class transport;
}

class transport : public QWidget {
    Q_OBJECT

public:
    transport(QWidget *parent = 0);
    ~transport();
    bool isRepeatChecked( void );

protected:
    void changeEvent(QEvent *e);

signals:
    void play();
    void pause();
    void advance();
    void rewind();
    void sliderReleased( int );
    void resetPlaybackRate();
    void seekRequest( qint64 );

public slots:
    void setPaused( bool );
    void handleClipName( QString name );
    void handleClipLengthChanged( qint64 );
    void handleForcePauseButtonOff() { setPaused( true ); }
    void updateClipPosition( qint64 );
    void configureLayoutForTraining( void );

private:
    Ui::transport *ui;
    bool isPlaying;
    bool isClipSelected;
    QString lengthString;

private slots:
    void on_rewindButton_clicked();
    void on_advanceButton_clicked();
    void on_playButton_clicked();
    void seekingStart();
    void seekingEnd();
};

#endif // TRANSPORT_H
