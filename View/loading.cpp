#include "loading.h"

#include <QPainter>
#include <QTimer>
#include "gamepanel.h"

Loading::Loading(QWidget *parent)
    : QWidget{parent}
{
    // 去边框
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    m_bk.load(":/images/loading.png");
    setFixedSize(m_bk.size());

    QPixmap tmp(":/images/progress.png");
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        m_progress = tmp.copy(0, 0, m_dist, tmp.height());
        update();
        if(m_dist >= tmp.width()){
            timer->stop();
            timer->deleteLater();
            GamePanel* panel = new GamePanel();
            panel->show();
            close();
        }
        m_dist+=5;
    });
    timer->start(15);
}

void Loading::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawPixmap(rect(), m_bk);
    p.drawPixmap(62, 417, m_progress.width(), m_progress.height(), m_progress);
}
