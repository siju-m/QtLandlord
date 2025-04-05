#ifndef PLAYER_H
#define PLAYER_H

#include <Cards.h>
#include <QObject>

class Player : public QObject
{
    Q_OBJECT
public:
    enum Role{Lord,Farmer};     // 角色
    enum Sex{Man,Woman};
    enum Direction{left,right};     // 方向
    enum Type{Robot,User,Unknow};   //玩家类型

    explicit Player(QObject *parent = nullptr);
    explicit Player(QString name, QObject *parent = nullptr);

    void setScore(int score);
    int getScore();

    // 名字
    void setName(const QString &name);
    QString getName();

    // 角色
    void setRole(const Role &role);
    Role getRole();

    //性别
    void setSex(const Sex &sex);
    Sex getSex();

    //方向
    void setDirection(const Direction &direction);
    Direction getDirection();

    //玩家类别
    void setType(const Type &type);
    Type getType();

    // 游戏结果
    void setWin(bool flag);
    bool isWin();

    // 当前玩家的上家下家对象
    void setPrevPlayer(Player *player);
    void setNextPlayer(Player *player);
    Player* getPrevPlayer();
    Player* getNextPlayer();

    // 叫地主/抢地主
    void grabLordBet(int point);

    // 存储扑克牌
    void storeDispatchCard(Card& card);
    void storeDispatchCard(Cards& cards);

    // 得到所有牌
    Cards getCards();
    // 清空所有牌
    void clearCards();
    // 出牌
    void playHand(Cards &cards);

    // 设置出牌的玩家已经待处理的牌
    void setPendingInfo(Player* player, Cards& cards);
    Player* getPendPlayer();
    Cards getPendCards();

    // 存储出牌玩家打出的牌
    void onStorePendingInfo(Player* player, Cards& cards);

    virtual void prepareCallLord();
    virtual void preparePlayHand();
signals:
    // 通知已经叫地主
    void notifyGrabLordBet(Player* player, int bet);
    // 通知已经出牌
    void notifyPlayHand(Player* player, Cards& cards);
    // 通知已经发牌了
    void notifyPickCards(Player* player, const Cards& cards);
protected:
    int m_score;
    QString m_name;
    Role m_role;
    Sex m_sex;
    Direction m_direction;
    Type m_type;
    bool m_isWin;
    Player* m_prev;
    Player* m_next;
    Cards m_cards;//玩家手牌
    Cards m_pendCards;
    Player* m_pendPlayer = nullptr;
};

#endif // PLAYER_H
