#ifndef CLIPITEMDELEGATE_H
#define CLIPITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QObject>

class ClipItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

signals:
    void updateLabel() const;

public:
    ClipItemDelegate(bool rotated = false, QObject *parent = nullptr);
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const override;

    void paint( QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const override;
public slots:
    void handleDisplayOffset(int dpo);

private:
    bool doRotate;
    int m_itemOffset{0};
};

#endif // CLIPITEMDELEGATE_H
