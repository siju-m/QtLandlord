#include "gamecontrol.h"

#include "PlayHand.h"

#include <QRandomGenerator>
#include <QTimer>
#include <qdebug.h>

GameControl::GameControl(QObject *parent)
    : QObject{parent}
{}

void GameControl::playerInit()
{
    m_robotLeft = new Robot("机器人A",this);
    m_robotRight = new Robot("机器人B",this);
    m_user = new UserPlayer("我自己",this);

    // 头像的显示
    m_robotLeft->setDirection(Player::left);
    m_robotRight->setDirection(Player::right);
    m_user->setDirection(Player::right);

    // 性别
    Player::Sex sex;
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotLeft->setSex(sex);
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_robotRight->setSex(sex);
    sex = (Player::Sex)QRandomGenerator::global()->bounded(2);
    m_user->setSex(sex);

    // 出牌顺序
    m_user->setPrevPlayer(m_robotLeft);
    m_user->setNextPlayer(m_robotRight);

    m_robotRight->setPrevPlayer(m_user);
    m_robotRight->setNextPlayer(m_robotLeft);

    m_robotLeft->setPrevPlayer(m_robotRight);
    m_robotLeft->setNextPlayer(m_user);

    // 指定当前玩家
    m_currPlayer = m_user;

    // 处理玩家发射的信号
    connect(m_user,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);
    connect(m_robotRight,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);
    connect(m_robotLeft,&UserPlayer::notifyGrabLordBet,this,&GameControl::onGrabBet);

    // 处理玩家出牌
    connect(m_robotLeft, &Player::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_robotRight, &Player::notifyPlayHand, this, &GameControl::onPlayHand);
    connect(m_user, &Player::notifyPlayHand, this, &GameControl::onPlayHand);

    // 传递出牌玩家对象打出的牌
    connect(this, &GameControl::pendingInfo, m_robotRight, &Player::onStorePendingInfo);
    connect(this, &GameControl::pendingInfo, m_robotLeft, &Player::onStorePendingInfo);
    connect(this, &GameControl::pendingInfo, m_user, &Player::onStorePendingInfo);
}

Robot *GameControl::getLeftRobot()
{
    return m_robotLeft;
}

Robot *GameControl::getRightRobot()
{
    return m_robotRight;
}

UserPlayer *GameControl::getUserPlayer()
{
    return m_user;
}

void GameControl::setCurrentPlayer(Player* player)
{
    m_currPlayer = player;
}

Player *GameControl::getCurrentPlayer()
{
    return m_currPlayer;
}

Player *GameControl::getPendPlayer()
{
    return m_pendPlayer;
}

Cards GameControl::getPendCards()
{
    return m_pendCards;
}

void GameControl::initAllcards()
{
    m_allCards.clear();
    for(int p = Card::Card_Begin+1; p<Card::Card_SJ; ++p){
        for(int s = Card::Suit_Begin+1; s<Card::Suit_End; ++s){
            Card c((Card::CardPoint)p,(Card::CardSuit)s);
            m_allCards.add(c);
        }
    }
    m_allCards.add(Card(Card::Card_SJ,Card::Suit_Begin));
    m_allCards.add(Card(Card::Card_BJ,Card::Suit_Begin));
}

Card GameControl::takeOneCard()
{
    return m_allCards.takeRandCard();
}

Cards GameControl::getSurplusCards()
{
    return m_allCards;
}

void GameControl::resetCardData()
{
    // 洗牌
    initAllcards();
    //清空所有玩家的牌
    m_robotLeft->clearCards();
    m_robotRight->clearCards();
    m_user->clearCards();
    //重置出牌玩家和牌
    m_pendPlayer = nullptr;
    m_pendCards.clear();
}

void GameControl::startLordCard()
{
    m_currPlayer->prepareCallLord();
    emit playerStatusChanged(m_currPlayer,ThinkingForCallLord);
}

void GameControl::becomeLord(Player* player, int bet)
{
    m_curBet = bet;
    player->setRole(Player::Lord);
    player->getPrevPlayer()->setRole(Player::Farmer);
    player->getNextPlayer()->setRole(Player::Farmer);

    m_currPlayer = player;
    player->storeDispatchCard(m_allCards);

    QTimer::singleShot(1000, this, [=](){
        emit gameStatusChanged(PlayingHand);
        emit playerStatusChanged(player,ThinKingForplayHand);
        m_currPlayer->preparePlayHand();
    });

}

void GameControl::clearPlayerScore()
{
    m_robotLeft->setScore(0);
    m_robotRight->setScore(0);
    m_user->setScore(0);

}

int GameControl::getPlayerMaxBet()
{
    return m_betRecord.bet;
}

void GameControl::onGrabBet(Player *player, int bet)
{
    // 通知主界面玩家叫地主了（更新信息提示）
    if(bet==0 || m_betRecord.bet>bet){
        emit notifyGrabLordBet(player, 0, false);
    }else if(bet>0 && m_betRecord.bet==0){
        // 第一个抢地主的玩家
        emit notifyGrabLordBet(player, bet, true);
    }else{
        emit notifyGrabLordBet(player, bet, false);
    }

    qDebug() << "curent player name: " << player->getName() << ", 下注分数: "
             << bet<< ", m_betRecord.times: " << m_betRecord.times;

    // 判断玩家下注是不是三分，如果是则抢地主结束
    if(bet == 3){
        becomeLord(player, bet);
        m_betRecord.reset();
        return;
    }
    // 下注不够三分，对玩家分数进行比较，则分数高者为地主
    if(m_betRecord.bet < bet){
        m_betRecord.bet = bet;
        m_betRecord.player = player;
    }
    ++m_betRecord.times;
    // 如果每个玩家都抢过一次地主，抢地主结束
    if(m_betRecord.times == 3){
        if(m_betRecord.bet == 0){
            emit gameStatusChanged(DispatchCard);
        }else{
            becomeLord(m_betRecord.player, m_betRecord.bet);
        }
        m_betRecord.reset();
        return;
    }
    // 切换玩家，通知下一个玩家继续抢地主
    m_currPlayer = player->getNextPlayer();
    // 发送信号给主界面，告知当前状态为抢地主
    emit playerStatusChanged(m_currPlayer,ThinkingForCallLord);
    m_currPlayer->prepareCallLord();
}

void GameControl::onPlayHand(Player *player, Cards &cards)
{
    // 将玩家出牌信号转发给主界面
    emit notifyPlayHand(player, cards);

    if(!cards.isEmpty()){
        m_pendPlayer = player;
        m_pendCards = cards;
        emit pendingInfo(m_pendPlayer, m_pendCards);
    }
    // 如果有炸弹，底分翻倍
    PlayHand::HandType type = PlayHand(cards).getHandType();
    if(type == PlayHand::Hand_Bomb || type == PlayHand::Hand_Bomb_Jokers){
        m_curBet*=2;
    }
    // 如果玩家的牌出完了，计算本局游戏的总分
    if(player->getCards().isEmpty()){
        Player* prev = player->getPrevPlayer();
        Player* next = player->getNextPlayer();
        if(player->getRole() == Player::Lord){
            player->setScore(player->getScore() + 2*m_curBet);
            prev->setScore(prev->getScore() - m_curBet);
            next->setScore(next->getScore() - m_curBet);
            player->setWin(true);
            prev->setWin(false);
            next->setWin(false);
        }else{
            player->setWin(true);
            player->setScore(player->getScore() + m_curBet);
            if(prev->getRole() == Player::Lord){
                prev->setScore(prev->getScore() - 2*m_curBet);
                next->setScore(next->getScore() + m_curBet);
                prev->setWin(false);
                next->setWin(true);
            }else{
                prev->setScore(prev->getScore() + m_curBet);
                next->setScore(next->getScore() - 2 * m_curBet);
                prev->setWin(true);
                next->setWin(false);
            }
        }
        emit playerStatusChanged(player, Winning);
        return;
    }
    // 牌没有出完，下一个玩家继续出牌
    m_currPlayer = player->getNextPlayer();
    m_currPlayer->preparePlayHand();
    emit playerStatusChanged(m_currPlayer, ThinKingForplayHand);
}
