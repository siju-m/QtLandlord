#ifndef GAMECONTROL_H
#define GAMECONTROL_H

#include <QObject>
#include <Robot.h>
#include <UserPlayer.h>

class GameControl : public QObject
{
    Q_OBJECT
public:
    // 游戏状态
    enum GameStatus{
        DispatchCard,
        CallingLord,
        PlayingHand
    };
    // 玩家状态
    enum PlayerStatus{
        ThinkingForCallLord,
        ThinKingForplayHand,
        Winning
    };

    explicit GameControl(QObject *parent = nullptr);

    // 初始化玩家
    void playerInit();

    // 得到玩家的实例对象
    Robot* getLeftRobot();
    Robot* getRightRobot();
    UserPlayer* getUserPlayer();

    void setCurrentPlayer(Player* player);
    Player* getCurrentPlayer();

    // 得到出牌玩家和打出的牌
    Player* getPendPlayer();
    Cards getPendCards();

    // 初始化扑克牌
    void initAllcards();
    // 每次发一张牌
    Card takeOneCard();
    // 得到最后三张牌
    Cards getSurplusCards();
    // 重置卡牌数据
    void resetCardData();

    // 准备叫地主
    void startLordCard();
    // 成为地主
    void becomeLord(Player* player);
    // 清空所有玩家分数
    void clearPlayerScore();
    // 处理叫地主
    // 处理出牌
signals:

private:
    Robot* m_robotLeft;
    Robot* m_robotRight;
    UserPlayer* m_user;
    Player* m_currPlayer;
    Player* m_pendPlayer;
    Cards m_pendCards;
    Cards m_allCards;
};

#endif // GAMECONTROL_H
