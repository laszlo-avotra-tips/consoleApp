#ifndef CASEREVIEWDIALOG_H
#define CASEREVIEWDIALOG_H

#include <QDialog>

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
