#ifndef PROPOS_H
#define PROPOS_H

#include <QDialog>

namespace Ui {
    class Propos;
}

class Propos : public QDialog
{
    Q_OBJECT

public:
    explicit Propos(QWidget* parent = nullptr);
    ~Propos();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::Propos* ui;
    int m_clickCount; // Counter for label clicks
};

#endif // PROPOS_H
