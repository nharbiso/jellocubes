#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(parent, f) {};
    explicit ClickableLabel(const QString &text, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(text, parent, f) {};

    void setActive(bool active) {
        this->active = active;
    }

signals:
    void clicked(bool active);

protected:
    bool active = false;
    void mousePressEvent(QMouseEvent* event) {
        this->active = !this->active;
        emit clicked(this->active);
    }
};

#endif // CLICKABLELABEL_H
