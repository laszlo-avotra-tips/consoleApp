#include "caseReviewModel.h"

#include <QUrl>
#include <QMediaPlaylist>

CaseReviewModel* CaseReviewModel::m_instance = nullptr;

CaseReviewModel *CaseReviewModel::instance()
{
    if(!m_instance){
        m_instance = new CaseReviewModel;
    }
    return m_instance;
}

CaseReviewModel::CaseReviewModel()
{
    m_singleSource =
        new QUrl
        (
            R"(file:///C:/Avinger_Data/7e71b349-a6ae-4c94-8d14-a1c9fe95d201/fullCase/fsequence3.ts)"
        );

    m_playlist = new QMediaPlaylist;
    m_playlist->addMedia(QUrl(R"(file:///C:/Avinger_Data/7e71b349-a6ae-4c94-8d14-a1c9fe95d201/fullCase/fsequence2.ts)"));
    m_playlist->addMedia(QUrl(R"(file:///C:/Avinger_Data/7e71b349-a6ae-4c94-8d14-a1c9fe95d201/fullCase/fsequence3.ts)"));
}

QMediaPlaylist *CaseReviewModel::getPlaylist() const
{
    return m_playlist;
}

const QUrl& CaseReviewModel::getSingleSource() const
{
    return *m_singleSource;
}
