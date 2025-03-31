#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include <CardPanel.h>
#include <GameControl.h>
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
    // 定时器的处理动作
    void onDispatchCard();
    // 移动扑克牌
    void cardMoveStep(Player* player, int curPos);
    // 处理分发得到的扑克牌
    void disposCard(Player* player, Cards &cards);
    // 更新扑克牌在窗口中的显示
    void updatePlayerCards(Player* player);
protected:
    void paintEvent(QPaintEvent *event);

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
};
#endif // GAMEPANEL_H
