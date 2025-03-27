#include "propos.h"
#include "ui_propos.h"
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>

Propos::Propos(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Propos),
    m_clickCount(0)
{
    ui->setupUi(this);
    setWindowTitle("A propos");

    ui->label->installEventFilter(this);
}

Propos::~Propos()
{
    delete ui;
}

bool Propos::eventFilter(QObject* obj, QEvent* event)
{

    if (obj == ui->label && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_clickCount++;
            qDebug() << "Label clicked" << m_clickCount << "times";

            if (m_clickCount >= 3) {
                m_clickCount = 0;

                QPixmap newPixmap(":/icons/icons/dino.png"); 
                if (!newPixmap.isNull()) {
                    newPixmap = newPixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    ui->label->setPixmap(newPixmap);
                   

                    bool started = QProcess::startDetached("cmd.exe", { "/c", "start", "", "https://wayou.github.io/t-rex-runner/" });

                    qDebug() << "Chrome launched:" << started;
                }
                else {
                    qDebug() << "New image not found!";
                }
            }
        }
        return true; 
    }
    return QDialog::eventFilter(obj, event);
}
