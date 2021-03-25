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
#include <vector>

//#define LOG1(x_)  qDebug() << __LINE__ << ". " << #x_ << "=" << x_

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
{
    LOG1("");
}

void VideoPlayer::init()
{
    m_mediaPlayer = new QMediaPlayer(this,QMediaPlayer::VideoSurface);
    m_pmMediaPlayer = new QMediaPlayer(this,QMediaPlayer::VideoSurface);

    m_videoWidget = new QVideoWidget();
    m_videoWidget->setFixedSize(1500,1500);

    m_pmVideoWidget = new QVideoWidget();
    m_pmVideoWidget->setFixedSize(980,980);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_videoWidgetContainer->addWidget(m_videoWidget);
    m_pmVideoWidgetContainer->addWidget(m_pmVideoWidget);

    m_videoWidgetContainer->addWidget(m_errorLabel);

    m_videoWidget->autoFillBackground();
    m_pmVideoWidget->autoFillBackground();

    m_mediaPlayer->setVideoOutput(m_videoWidget);   
    m_pmMediaPlayer->setVideoOutput(m_pmVideoWidget);

    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &VideoPlayer::handleError);

    emit playerInitialized();
}

bool VideoPlayer::isPlaying() const
{
    return m_mediaPlayer->state() == QMediaPlayer::PlayingState;
}

VideoPlayer::~VideoPlayer()
{
    LOG1("~VideoPlayer");
    delete m_videoWidget;
    delete m_pmVideoWidget;
    delete m_mediaPlayer;
    delete m_pmMediaPlayer;
    delete m_errorLabel;
}


void VideoPlayer::setUrl(const QUrl &url)
{
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_mediaPlayer->setMedia(url);
    m_pmMediaPlayer->setMedia(url);
}

void VideoPlayer::play()
{
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        m_pmMediaPlayer->pause();
        break;
    default:
        m_mediaPlayer->play();
        m_pmMediaPlayer->play();
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
    m_pmMediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
{
    const std::vector<QString> errorLut
    {
        "NoError",
        "ResourceError",
        "FormatError",
        "NetworkError",
        "AccessDeniedError",
        "ServiceMissingError",
        "MediaIsPlaylist"
    };

    const QString& errorString = m_mediaPlayer->errorString();
    const QMediaPlayer::Error& errorNumber = m_mediaPlayer->error();

    if(uint(errorNumber) < 7){
        const QString& errorDecode = errorLut[int(errorNumber)];
        LOG3(errorNumber, errorDecode, errorString)
    } else {
        LOG2(errorNumber, errorString)
    }
}

void VideoPlayer::setVideoWidgetContainer(QVBoxLayout *videoWidgetContainer)
{
    m_videoWidgetContainer = videoWidgetContainer;
    LOG1(m_videoWidgetContainer);
}

void VideoPlayer::setPmVideoWidgetContainer(QVBoxLayout *videoWidgetContainer)
{
    m_pmVideoWidgetContainer = videoWidgetContainer;
    LOG1(m_videoWidgetContainer);
}
