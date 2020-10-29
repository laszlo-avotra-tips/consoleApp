#ifndef CASEREVIEWDIALOG_H
#define CASEREVIEWDIALOG_H

#include <QDialog>
#include <QMediaPlayer>

class QVideoWidget;
class QMediaPlayer;
class QAbstractButton;
class QSlider;

namespace Ui {
class CaseReviewDialog;
}

class CaseReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CaseReviewDialog(QWidget *parent = nullptr);
    ~CaseReviewDialog();
    void setUrl(const QUrl &url);

private slots:
    void on_pushButtonOpen_clicked();
    void mediaStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void handleError();

public slots:
    void openFile();
    void play();

private:
    Ui::CaseReviewDialog *ui;
    QVideoWidget* m_videoWidget{nullptr};
    QMediaPlayer* m_mediaPlayer{nullptr};
    QAbstractButton *m_playButton{nullptr};
    QSlider *m_positionSlider{nullptr};
};

#endif // CASEREVIEWDIALOG_H
