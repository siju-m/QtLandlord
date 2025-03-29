#include "cardpanel.h"

#include <QPainter>

CardPanel::CardPanel(QWidget *parent)
    : QWidget{parent}
    , m_isFront(true)
{}

void CardPanel::setImage(QPixmap &front, QPixmap &back)
{
    m_front = front;
    m_back = back;

    setFixedSize(m_front.size());

    update();
}

QPixmap CardPanel::getImage()
{
    return m_front;
}

void CardPanel::setFrontSide(bool flag)
{
    m_isFront = flag;
}

bool CardPanel::isFrontSide()
{
    return m_isFront;
}

void CardPanel::setSelected(bool flag)
{
    m_isSelected = flag;
}

bool CardPanel::isSelected()
{
    return m_isSelected;
}

void CardPanel::setCard(Card &card)
{
    m_card = card;
}

Card CardPanel::getCard()
{
    return m_card;
}

void CardPanel::setOwner(Player *player)
{
    m_owner = player;
}

Player *CardPanel::getOwner()
{
    return m_owner;
}

void CardPanel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    if(m_isFront){
        p.drawPixmap(rect(),m_front);
    }else{
        p.drawPixmap(rect(),m_back);
    }
}

void CardPanel::mousePressEvent(QMouseEvent *event)
{

}
