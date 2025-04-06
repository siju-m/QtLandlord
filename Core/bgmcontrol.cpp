#include "bgmcontrol.h"

#include <QAudioOutput>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

BGMControl::BGMControl(QObject *parent)
    : QObject{parent}
{
    for(int i=0;i<5;++i){
        QMediaPlayer *player = new QMediaPlayer(this);
        QAudioOutput* audioOutput = new QAudioOutput(this);
        QVector<QString> list = QVector<QString>();
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
    QAudioOutput* audioOutput = m_players[2]->audioOutput();
    audioOutput->setVolume(valume);
    m_players[2]->play();
}

void BGMControl::stopBGM()
{
    m_players[2]->stop();
}
