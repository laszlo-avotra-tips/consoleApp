#ifndef CASEREVIEWMODEL_H
#define CASEREVIEWMODEL_H

#include <QObject>

class QUrl;

class CaseReviewModel
{
public:
    static CaseReviewModel* instance();
    const QUrl& getSingleSource() const;

private:
    CaseReviewModel();

private:
    static CaseReviewModel* m_instance;
    const QUrl* m_singleSource{nullptr};
};

#endif // CASEREVIEWMODEL_H
