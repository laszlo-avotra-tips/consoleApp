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

    bool isSelected() const;
    void setIsSelected(bool isSelected);

signals:
    void labelSelected(const QString& name);
    void labelItemSelected(ActiveLabel* label);

private:
    bool m_isSelected{false};

};

#endif // ACTIVELABEL_H
