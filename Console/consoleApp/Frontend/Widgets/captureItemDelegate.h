#ifndef CAPTUREITEMDELEGATE_H
#define CAPTUREITEMDELEGATE_H

#include <QObject>
#include <QAbstractItemDelegate>

/*
 * CaptureItemDelegate
 *
 * The captureItemDelegate knows how to draw individual thumbnails
 * of captures, along with any tagging or shortcut markup, for inclusion
 * in the recent captures list.
 */
class CaptureItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

signals:
    void updateLabel() const;


public:
    CaptureItemDelegate(bool rotated = false, QObject *parent = 0);
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const;
    void paint( QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const;
private:
    bool doRotate;
};


#endif // CAPTUREITEMDELEGATE_H
