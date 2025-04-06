#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include "animationwindow.h"
#include "bgmcontrol.h"
#include "countdown.h"
#include "gamecontrol.h"
#include "cardpanel.h"
#include <QMainWindow>
#include <QLabel>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class GamePanel;
}
QT_END_NAMESPACE

class GamePanel : public QMainWindow
{
    Q_OBJECT

public:
    GamePanel(QWidget *parent = nullptr);
    ~GamePanel();
    enum AnimationType{ShunZi, LianDui, Plane, JokerBomb, Bomb, Bet};
    // 初始化游戏控制类信息
    void gameControlInit();
    // 更新玩家分数
    void updatePlayerScore();
    // 切割并存储图片
    void initCardMap();
    // 裁剪图片
    void cropImage(QPixmap &pix, int x, int y, Card &card);
    // 初始化游戏按钮组
    void initButtonGroup();
    // 初始化上下文
    void initPlayerContext();
    // 初始化游戏场景
    void initGameScene();
    // 处理游戏状态
    void gameStatusProcess(GameControl::GameStatus status);
    // 发牌
    void startDispatchCard();
    // 移动扑克牌
    void cardMoveStep(Player* player, int curPos);
    // 处理分发得到的扑克牌
    void disposeCard(Player* player, const Cards &cards);
    // 更新扑克牌在窗口中的显示
    void updatePlayerCards(Player* player);
    // 加载玩家头像
    QPixmap loadRoleImage(Player::Sex sex, Player::Direction direct, Player::Role role);

    // 定时器的处理发牌动作
    void onDispatchCard();
    // 处理玩家状态变化
    void onPlayerStatusChanged(Player* player, GameControl::PlayerStatus status);
    // 处理玩家抢地主
    void onGrabLordBet(Player* player, int bet, bool flag);
    // 处理玩家出牌
    void onDisposePlayHand(Player* player, Cards &cards);
    // 处理玩家选牌
    void onCardSelected(Qt::MouseButton button);
    // 处理用户玩家出牌
    void onUserPlayHand();
    // 用户玩家放弃出牌
    void onUserPass();

    // 显示特效动画
    void showAnimation(AnimationType type, int bet = 0);
    // 隐藏玩家打出的牌
    void hidePlayerDropCards(Player* player);
    // 显示玩家的最终得分
    void showEndingScorePanel();
    // 初始化闹钟倒计时
    void initCountDown();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    enum CardAlign{Horizontal,Vertical};
    struct PlayerContext{
        // 玩家扑克牌显示区域
        QRect cardRect;
        // 出牌区域
        QRect playHandRect;
        // 扑克牌对齐方式
        CardAlign align;
        // 扑克牌正反
        bool isFrontSide;
        // 游戏过程的提示信息
        QLabel* info;
        // 玩家的头像
        QLabel* roleImg;
        // 玩家刚打出的牌
        Cards lastCards;
    };
    Ui::GamePanel *ui;
    QPixmap m_bkImage;
    GameControl *m_gameCtl;
    QVector<Player*> m_playerList;
    QMap<Card,CardPanel*> m_cardMap;
    QSize m_cardSize;
    QPixmap m_cardBackImg;
    QMap<Player* ,PlayerContext> m_contextMap;
    CardPanel* m_baseCard;
    QPoint m_baseCardPos;
    CardPanel* m_moveCard;
    QVector<CardPanel*> m_last3Card;
    GameControl::GameStatus m_gameStatus;
    QTimer* m_timer;
    AnimationWindow* m_animation;
    CardPanel* m_curSelCard;
    QSet<CardPanel*> m_selectCards;
    QRect m_cardsRect;
    QHash<CardPanel*, QRect> m_userCards;
    CountDown* m_countDown;
    BGMControl* m_bgm;
};
#endif // GAMEPANEL_H
