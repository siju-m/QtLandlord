#ifndef BGMCONTROL_H
#define BGMCONTROL_H

#include <QMediaPlayer>
#include <QObject>

class BGMControl : public QObject
{
    Q_OBJECT
public:
    explicit BGMControl(QObject *parent = nullptr);

    // 初始化播放列表
    void initPlayList();
    // 背景音乐
    void startBGM(float valume);//音量大小从0到1
    void stopBGM();
    // 抢地主音乐

    // 出牌背景音乐

    // 不出牌的背景音乐

    // 播放辅助音乐

    // 播放结束音乐
signals:

private:
    // man woman bgm 辅助音乐 结束音乐
    QVector<QMediaPlayer*> m_players;
    QVector<QVector<QString>> m_lists;
};

#endif // BGMCONTROL_H
