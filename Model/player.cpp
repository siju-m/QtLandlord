#include "player.h"
#include <qdebug.h>

Player::Player(QObject *parent)
    : QObject{parent}
    , m_score(0)
    , m_isWin(false)
{}

Player::Player(QString name, QObject *parent)
    : Player(parent)//委托构造函数，调用上面的函数
{
    m_name = name;
}

void Player::setScore(int score)
{
    m_score = score;
}

int Player::getScore()
{
    return m_score;
}

void Player::setName(const QString &name)
{
    m_name = name;
}

QString Player::getName()
{
    return m_name;
}

void Player::setRole(const Role &role)
{
    m_role = role;
}

Player::Role Player::getRole()
{
    return m_role;
}

void Player::setSex(const Sex &sex)
{
    m_sex = sex;
}

Player::Sex Player::getSex()
{
    return m_sex;
}

void Player::setDirection(const Direction &direction)
{
    m_direction  = direction;
}

Player::Direction Player::getDirection()
{
    return m_direction;
}

void Player::setType(const Type &type)
{
    m_type = type;
}

Player::Type Player::getType()
{
    return m_type;
}

void Player::setWin(bool flag)
{
    m_isWin = flag;
}

bool Player::isWin()
{
    return m_isWin;
}

void Player::setPrevPlayer(Player *player)
{
    m_prev = player;
}

void Player::setNextPlayer(Player *player)
{
    m_next = player;
}

Player *Player::getPrevPlayer()
{
    return m_prev;
}

Player *Player::getNextPlayer()
{
    return m_next;
}

void Player::grabLordBet(int point)
{
    emit notifyGrabLordBet(this,point);
}

void Player::storeDispatchCard(Card &card)
{
    m_cards.add(card);
    Cards cs;
    cs.add(card);
    emit notifyPickCards(this, cs);
}

void Player::storeDispatchCard(Cards &cards)
{
    m_cards.add(cards);
    emit notifyPickCards(this, cards);
}

Cards Player::getCards()
{
    return m_cards;
}

void Player::clearCards()
{
    m_cards.clear();
}

void Player::playHand(Cards &cards)
{
    m_cards.remove(cards);
    emit notifyPlayHand(this, cards);
}

// void Player::setPendingInfo(Player *player, Cards &cards)
// {
//     m_pendPlayer = player;
//     m_pendCards = cards;
// }

Player *Player::getPendPlayer()
{
    return m_pendPlayer;
}

Cards Player::getPendCards()
{
    return m_pendCards;
}

void Player::onStorePendingInfo(Player *player, Cards &cards)
{
    m_pendPlayer = player;
    m_pendCards = cards;
}

bool Player::getIsNotice(int n)
{
    if(n == 1)
        return m_1isNotice;

    if(n == 2)
        return m_2isNotice;

    return true;
}

void Player::setHasNotice(int n)
{
    if(n == 1){
        m_1isNotice = true;
    }
    if(n == 2)
        m_2isNotice = true;
}

void Player::prepareCallLord()
{

}

void Player::preparePlayHand()
{

}

