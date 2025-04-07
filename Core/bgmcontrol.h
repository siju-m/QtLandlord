#ifndef BGMCONTROL_H
#define BGMCONTROL_H

#include "Cards.h"
#include <QMediaPlayer>
#include <QObject>

class BGMControl : public QObject
{
    Q_OBJECT
public:
    enum RoleSex{Man, Woman};
    enum CardType
    {
        // 单张牌
        Three,
        Foue,
        Five,
        Six,
        Seven,
        Eight,
        Nine,
        Ten,
        Jack,
        Queen,
        King,
        Ace,
        Tow,
        SmallJoker,
        BigJoker,
        // 两张牌
        Three_Double,
        Foue_Double,
        Five_Double,
        Six_Double,
        Seven_Double,
        Eight_Double,
        Nine_Double,
        Ten_Double,
        Jack_Double,
        Queen_Double,
        King_Double,
        Ace_Double ,
        Tow_Double,
        // 三张牌
        Three_Triple,
        Foue_Triple,
        Five_Triple,
        Six_Triple,
        Seven_Triple,
        Eight_Triple,
        Nine_Triple,
        Ten_Triple,
        Jack_Triple,
        Queen_Triple,
        King_Triple,
        Ace_Triple,
        Tow_Triple,
        // 其他组合
        Plane,          // 飞机
        SequencePair,   // 连对
        ThreeBindOne,   // 三带一
        ThreeBindPair,  // 三带一对
        Sequence,       // 顺子
        FourBindTwo,    // 四带二(单张)
        FourBind2Pair,  // 四带两对
        Bomb,           // 炸弹
        JokerBomb,      // 王炸
        Pass1,          // 过
        Pass2,
        Pass3,
        Pass4,
        MoreBiger1,     // 大你
        MoreBiger2,
        Biggest,        // 压死
        // 抢地主
        NoOrder,        // 不叫
        NoRob,          // 不抢
        Order,          // 叫地主
        Rob1,           // 抢地主
        Rob2,
        Last1,          // 只剩1张牌
        Last2           // 只剩2张牌
    };
    enum AssistMusic{
        Dispatch,   // 发牌
        SelectCard, // 选牌
        PlaneVoice, // 飞机
        BombVoice,  // 炸弹
        Alert,      // 提醒
    };

    explicit BGMControl(QObject *parent = nullptr);

    // 初始化播放列表
    void initPlayList();
    // 背景音乐
    void startBGM(float valume);//音量大小从0到1
    void stopBGM();
    // 抢地主音乐
    void playerRolordMusic(int point, RoleSex sex, bool isFirst);
    // 出牌背景音乐
    void playCardMusic(Cards cards, bool isFirst, RoleSex sex);
    // 不出牌的背景音乐
    void playPassMusic(RoleSex sex);
    // 播放辅助音乐
    void playAssistMusic(AssistMusic type);
    void stopAssistMusic();
    // 播放结束音乐
    void playEndingMusic(bool isWin);
    // 播放牌快打完的提示音
    void playLastMusic(CardType type, RoleSex sex);
signals:

private:

private:
    // man woman bgm 辅助音乐 结束音乐
    QVector<QMediaPlayer*> m_players;
    QVector<QVector<QString>> m_lists;
};

#endif // BGMCONTROL_H
