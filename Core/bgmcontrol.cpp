#include "bgmcontrol.h"
#include "PlayHand.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <qtimer.h>

// 使用QAudioOutput输出音频时，如果扬声器没有设置空间音效就无法播放
// 可能是6.8把QAudioOutput底层改了，6.5能正常播放，最好不要使用6.8
BGMControl::BGMControl(QObject *parent)
    : QObject{parent}
{
    for(int i=0;i<5;++i){
        QMediaPlayer *player = new QMediaPlayer(this);
        QAudioOutput* audioOutput = new QAudioOutput(this);
        QVector<QString> list = QVector<QString>();

        connect(player, &QMediaPlayer::errorOccurred, [](QMediaPlayer::Error error){
            qDebug() << "Media error:" << error;
        });
        if(i<2 || i == 4){
            player->setLoops(1);// 只播放一次
        }else if(i==2){
            player->setLoops(-1);// 循环播放
        }
        audioOutput->setVolume(1);
        player->setAudioOutput(audioOutput);
        m_players.push_back(player);
        m_lists.push_back(list);
    }

    initPlayList();
}

void BGMControl::initPlayList()
{
    QStringList list;
    list << "Man" << "Woman" << "BGM" << "Other" << "Ending";
    // 读取json配置文件
    QFile file(":/conf/playList.json");
    file.open(QFile::ReadOnly);
    QByteArray json = file.readAll();
    file.close();
    // 解析从文件读取的url路径
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject obj = doc.object();
    // 初始化播放列表
    for(int i=0; i<list.size(); ++i){
        QString prefix = list.at(i);
        QJsonArray array = obj.value(prefix).toArray();
        for(int j=0; j<array.size(); ++j){
            m_lists[i].push_back(array[j].toString());
        }
    }
}

void BGMControl::startBGM(float valume)
{
    m_players[2]->setSource(QUrl(m_lists[2][0]));
    m_players[2]->audioOutput()->setVolume(valume);
    m_players[2]->play();
}

void BGMControl::stopBGM()
{
    m_players[2]->stop();
}

// 玩家下注了没有?
// 玩家的性别?
// 什么时候播放什么样的音频文件
void BGMControl::playerRolordMusic(int point, RoleSex sex, bool isFirst)
{
    int index = sex == Man ? 0:1;
    QUrl source;
    if(isFirst && point>0){
        source = QUrl(m_lists[index].at(Order));
    }else if(point==0){
        if(isFirst){
            source = QUrl(m_lists[index].at(NoOrder));
        }else{
            source = QUrl(m_lists[index].at(NoRob));
        }
    }else if(point == 2){
        source = QUrl(m_lists[index].at(Rob1));
    }else if(point == 3){
        source = QUrl(m_lists[index].at(Rob2));
    }
    m_players[index]->setSource(source);
    m_players[index]->play();
}

void BGMControl::playCardMusic(Cards cards, bool isFirst, RoleSex sex)
{
    int index = sex == Man ? 0:1;
    Card::CardPoint pt = Card::CardPoint::Card_Begin;
    // 取出牌型进行判断
    PlayHand hand(cards);
    PlayHand::HandType type = hand.getHandType();
    if(type == PlayHand::Hand_Single || type == PlayHand::Hand_Pair || type == PlayHand::Hand_Triple)
    {
        pt = cards.takeRandomCard().point();
    }
    int number = 0;
    switch(type)
    {
    // 单牌
    case PlayHand::Hand_Single:
        number = pt - 1;
        break;
    // 对牌
    case PlayHand::Hand_Pair:
        number = pt - 1 + 15;
        break;
    // 三张点数相同的牌
    case PlayHand::Hand_Triple:
        number = pt - 1 + 15 + 13;
        break;
    // 三带一
    case PlayHand::Hand_Triple_Single:
        number = ThreeBindOne;
        break;
    // 三带二
    case PlayHand::Hand_Triple_Pair:
        number = ThreeBindPair;
        break;
    // 飞机
    case PlayHand::Hand_Plane:
    // 飞机带两个单
    case PlayHand::Hand_Plane_Two_Single:
    // 飞机带两个对儿
    case PlayHand::Hand_Plane_Two_Pair:
        number = Plane;
        break;
    // 连对
    case PlayHand::Hand_Seq_Pair:
        number = SequencePair;
        break;
    // 顺子
    case PlayHand::Hand_Seq_Single:
        number = Sequence;
        break;
    // 炸弹
    case PlayHand::Hand_Bomb:
        number = Bomb;
        break;
    // 王炸
    case PlayHand::Hand_Bomb_Jokers:
        number = JokerBomb;
        break;
    // 炸弹带一对儿
    case PlayHand::Hand_Bomb_Pair:
    // 炸弹带两单
    case PlayHand::Hand_Bomb_Two_Single:
    // 王炸带一对儿
    case PlayHand::Hand_Bomb_Jokers_Pair:
    // 王炸带两单
    case PlayHand::Hand_Bomb_Jokers_Two_Single:
        number = FourBindTwo;

    default:
        break;
    }

    QUrl url;
    if(!isFirst && (number>Plane && number<FourBindTwo)){
        url = QUrl(m_lists[index].at(MoreBiger1 + QRandomGenerator::global()->bounded(2)));
    }else{
        url = QUrl(m_lists[index].at(number));
    }

    if (m_players[index]->playbackState() == QMediaPlayer::PlayingState) {
        m_players[index]->stop(); // 可选：等待一帧后再 setSource()
    }
    m_players[index]->setSource(url);
    m_players[index]->play();

    if(number == Bomb || number == JokerBomb)
        playAssistMusic(BombVoice);
    if(number == Plane)
        playAssistMusic(PlaneVoice);
}

void BGMControl::playPassMusic(RoleSex sex)
{
    int index = sex == Man ? 0:1;

    int random = QRandomGenerator::global()->bounded(4);
    QUrl url(m_lists[index].at(Pass1 + random));

    if (m_players[index]->playbackState() == QMediaPlayer::PlayingState) {
        m_players[index]->stop(); // 可选：等待一帧后再 setSource()
    }
    m_players[index]->setSource(url);
    m_players[index]->play();
}

void BGMControl::playAssistMusic(AssistMusic type)
{
    // todo 选牌音效有bug
    if(type == Dispatch){
        // 循环播放
        m_players[3]->setLoops(-1);
    }else{
        // 只播放一次
        m_players[3]->setLoops(1);
    }
    QUrl url(m_lists[3].at(type));

    if (m_players[3]->playbackState() == QMediaPlayer::PlayingState) {
        m_players[3]->stop();
    }
    m_players[3]->setSource(url);
    m_players[3]->play();

}

void BGMControl::stopAssistMusic()
{
    m_players[3]->stop();
}

void BGMControl::playEndingMusic(bool isWin)
{
    QUrl url;
    if(isWin){
        url = QUrl(m_lists[4].at(0));
    }else{
        url = QUrl(m_lists[4].at(1));
    }
    m_players[4]->setSource(url);
    m_players[4]->play();
}

void BGMControl::playLastMusic(CardType type, RoleSex sex)
{
    // 和出牌使用同一个输出的话会被打断，用一下结束bgm的输出
    int index = sex == Man ? 0:1;
    QTimer::singleShot(1500, this, [=](){
        QUrl url(m_lists[index].at(type));
        m_players[4]->setSource(url);
        m_players[4]->play();
    });

}

