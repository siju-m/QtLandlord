#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include "player.h"

class Robot : public Player
{
    Q_OBJECT
public:
    using Player::Player;
    explicit Robot(QObject* parent = nullptr);

    void prepareCallLord() override;
    void preparePlayHand() override;

    // 考虑叫地主
    void thinkCallLord();
    // 考虑出牌
    void thinkPlayHand();
};

#endif // ROBOT_H
