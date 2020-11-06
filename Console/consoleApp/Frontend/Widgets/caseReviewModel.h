#ifndef CASEREVIEWMODEL_H
#define CASEREVIEWMODEL_H

#include <QObject>

class QUrl;
class QMediaPlaylist;

class CaseReviewModel
{
public:
    static CaseReviewModel* instance();
    const QUrl& getSingleSource() const;

    QMediaPlaylist *getPlaylist() const;

private:
    CaseReviewModel();

private:
    static CaseReviewModel* m_instance;
    const QUrl* m_singleSource{nullptr};
    QMediaPlaylist* m_playlist{nullptr};
};

#endif // CASEREVIEWMODEL_H
