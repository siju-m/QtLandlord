#include "scorepanel.h"
#include "View/ui_scorepanel.h"

const QString MYCOLOR[] = {"black", "white", "red", "blue", "green"};
ScorePanel::ScorePanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScorePanel)
{
    ui->setupUi(this);
    m_list << ui->meScore << ui->leftScore << ui->rightScore
           << ui->meTitle << ui->leftTitle << ui->rightTitle
           << ui->score1 << ui->score2 << ui->score3;
}

ScorePanel::~ScorePanel()
{
    delete ui;
}

void ScorePanel::setScores(int left, int right, int user)
{
    ui->leftScore->setText(QString::number(left));
    ui->rightScore->setText(QString::number(right));
    ui->meScore->setText(QString::number(user));
}

void ScorePanel::setMyFontSize(int point)
{
    QFont font("微软雅黑", point, QFont::Bold);
    for(auto &it : m_list){
        it->setFont(font);
    }
}

void ScorePanel::setMyFontColor(FontColor color)
{
    QString style = QString("QLabel{color:%1}").arg(MYCOLOR[color]);
    for(auto &it : m_list)
        it->setStyleSheet(style);
}
