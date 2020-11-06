/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "videoplayer.h"

#include <QtWidgets>
#include <QVideoWidget>
#include <QDebug>
#include "logger.h"

//#define LOG1(x_)  qDebug() << __LINE__ << ". " << #x_ << "=" << x_

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
{
    LOG1("");
}

void VideoPlayer::init()
{
    m_mediaPlayer = new QMediaPlayer(this,QMediaPlayer::VideoSurface);
    m_videoWidget = new QVideoWidget();

    m_errorLabel = new QLabel(this);
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    if(!m_videoWidgetContainer){
        m_videoWidgetContainer = new QVBoxLayout();
        setLayout(m_videoWidgetContainer);
        qDebug() << __LINE__ << "local m_videoWidgetContainer";
        LOG1(m_videoWidgetContainer);
    }
    QVBoxLayout *layout = m_videoWidgetContainer;
    layout->addWidget(m_videoWidget);
    layout->addWidget(m_errorLabel);
    m_videoWidget->autoFillBackground();
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &VideoPlayer::handleError);
    openFile();

    emit playerInitialized();
}

VideoPlayer::~VideoPlayer()
{
    LOG1("~VideoPlayer");
    delete m_videoWidget;
    delete m_mediaPlayer;
    delete m_errorLabel;
}

void VideoPlayer::openFile()
{
//    bool useFileDialog(false);

//    if(useFileDialog){
//        QFileDialog fileDialog(this);
//        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
//        fileDialog.setWindowTitle(tr("Open Video"));
//        fileDialog.setDirectory("C:\\Avinger_Data7\\03df5cad-a401-4d99-a42c-0a79019423f4\\fullCase\\"); //fsequence7.ts
//        if (fileDialog.exec() == QDialog::Accepted)
//            setUrl(fileDialog.selectedUrls().constFirst());
//    } else
    {
        const QUrl url(R"(file:///C:/Avinger_Data/7e71b349-a6ae-4c94-8d14-a1c9fe95d201/fullCase/fsequence3.ts)");
        m_errorLabel->setText(QString());
        setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
        m_mediaPlayer->setMedia(url);
    }

}

void VideoPlayer::setUrl(const QUrl &url)
{
    m_errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_mediaPlayer->setMedia(url);
}

void VideoPlayer::play()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    default:
        m_mediaPlayer->play();
        break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
    emit updatePosition(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    emit playerInitialized();
    emit updateDuration(duration);
}

void VideoPlayer::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
{
    const QString errorString = m_mediaPlayer->errorString();
    QString errorMessage = "Error: ";
    if (errorString.isEmpty())
        errorMessage += " #" + QString::number(int(m_mediaPlayer->error()));
    else
        errorMessage += errorString;
    m_errorLabel->setText(errorMessage);
    LOG1(errorMessage)
}

void VideoPlayer::setVideoWidgetContainer(QVBoxLayout *videoWidgetContainer)
{
    m_videoWidgetContainer = videoWidgetContainer;
    LOG1(m_videoWidgetContainer);
}
