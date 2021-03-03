#ifndef ACTIVELABEL_H
#define ACTIVELABEL_H

#include <QLabel>

class ActiveLabel : public QLabel
{
    Q_OBJECT

public:
    ActiveLabel(QWidget* parent);
    void mouseReleaseEvent(QMouseEvent*) override;
    void mark();
    void unmark();

    bool isMarked() const;
    void setIsMarked(bool isMarked);

signals:
    void labelItemSelected(ActiveLabel* label);

private:
    bool m_isMarked{false};

};

#endif // ACTIVELABEL_H
