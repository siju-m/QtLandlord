#include "gamepanel.h"
#include "./ui_gamepanel.h"

#include <QPainter>
#include <QRandomGenerator>

GamePanel::GamePanel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GamePanel)
{
    ui->setupUi(this);

    // 背景图
    int num = QRandomGenerator::global()->bounded(10);
    QString path = QString(":/images/background-%1.png").arg(num+1);
    m_bkImage.load(path);

    // 窗口的标题、大小
    this->setWindowTitle("欢乐斗地主");
    this->setFixedSize(1000,650);

    // 实例化游戏控制类对象
    gameControlInit();

    // 更新玩家得分
    updatePlayerScore();

    // 切割游戏图片
    initCardMap();

    // 初始化游戏按钮组
    initButtonGroup();

    // 初始化玩家在窗口的上下文环境
    initPlayerContext();

    // 扑克牌场景初始化
    initGameScene();
}

GamePanel::~GamePanel()
{
    delete ui;
}

void GamePanel::gameControlInit()
{
    m_gameCtl = new GameControl(this);
    m_gameCtl->playerInit();
    // 得到三个玩家的实例对象
    Robot* leftRobot  = m_gameCtl->getLeftRobot();
    Robot* rightRobot = m_gameCtl->getRightRobot();
    UserPlayer* user = m_gameCtl->getUserPlayer();
    m_playerList<< leftRobot<< rightRobot<< user;
}

void GamePanel::updatePlayerScore()
{
    ui->scorePanel->setScores(m_playerList[0]->getScore(),
                              m_playerList[1]->getScore(),
                              m_playerList[2]->getScore());
}

void GamePanel::initCardMap()
{
    QPixmap pixmap(":/images/card.png");
    // 单张扑克牌大小
    m_cardSize.setHeight(pixmap.height()/5);
    m_cardSize.setWidth(pixmap.width()/13);
    // 背面图
    m_cardBackImg = pixmap.copy(m_cardSize.width()*2, m_cardSize.height()*4, m_cardSize.width(), m_cardSize.height());
    //普通牌
    for(int i=0,suit = Card::Suit_Begin+1;suit<Card::Suit_End;++suit,++i){
        for(int j=0,point = Card::Card_Begin+1;point<Card::Card_SJ;++point,++j){
            Card card((Card::CardPoint)point, (Card::CardSuit)suit);
            cropImage(pixmap, j*m_cardSize.width(), i*m_cardSize.height(), card);
        }
    }
    //大小王
    Card c;
    c.setPoint(Card::Card_SJ);
    c.setSuit(Card::Suit_Begin);
    cropImage(pixmap, 0, 4*m_cardSize.height(), c);

    c.setPoint(Card::Card_BJ);
    cropImage(pixmap, m_cardSize.width(), 4*m_cardSize.height(), c);
}

void GamePanel::cropImage(QPixmap &pix, int x, int y, Card &card)
{
    QPixmap p = pix.copy(x, y, m_cardSize.width(),m_cardSize.height());
    CardPanel* panel = new CardPanel(this);
    panel->setImage(p, m_cardBackImg);
    panel->setCard(card);
    panel->hide();
    m_cardMap.insert(card, panel);
}

void GamePanel::initButtonGroup()
{
    ui->btnGroup->initButtons();
    ui->btnGroup->selectPanel(ButtonGroup::Start);

    connect(ui->btnGroup, &ButtonGroup::startGame, this, [=](){});
    connect(ui->btnGroup, &ButtonGroup::playHand, this, [=](){});
    connect(ui->btnGroup, &ButtonGroup::pass, this, [=](){});
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](){});
}

void GamePanel::initPlayerContext()
{
    // 放置玩家扑克牌的位置
    QRect cardsRect[]={
        QRect(90,130,100,height()-200),
        QRect(rect().right()-190,130,100,height()-200),
        QRect(250,rect().bottom()-120,width()-500,100)
    };
    // 玩家出牌的区域
    QRect playHandRect[]={
        QRect(260,150,100,100),
        QRect(rect().right()-360,150,100,100),
        QRect(150,rect().bottom()-290,width()-300,100)
    };
    // 玩家头像显示的位置
    QPoint roleImgRect[]={
        QPoint(cardsRect[0].left()-80,cardsRect[0].right()/2+20),
        QPoint(cardsRect[1].right()+10,cardsRect[1].right()/2+20),
        QPoint(cardsRect[2].right()-10,cardsRect[2].top()-10)
    };

    int index = m_playerList.indexOf(m_gameCtl->getUserPlayer());
    for(int i=0; i<m_playerList.size(); ++i){
        PlayerContext context;
        context.align = i==index ? Horizontal:Vertical;
        context.isFrontSide = i==index;
        context.cardRect = cardsRect[i];
        context.playHandRect = playHandRect[i];
        // 提示信息
        context.info = new QLabel(this);
        context.info->resize(160,98);
        context.info->hide();
        // 显示到出牌区域的中心位置
        QRect rect = playHandRect[i];
        QPoint pt(rect.left()+(rect.width() - context.info->width())/2,
                  rect.top()+(rect.height() - context.info->height())/2);
        context.info->move(pt);
        // 玩家的头像
        context.roleImg = new QLabel(this);
        context.roleImg->resize(84,120);
        context.roleImg->hide();
        context.roleImg->move(roleImgRect[i]);

        m_contextMap.insert(m_playerList.at(i),context);
    }
}

void GamePanel::initGameScene()
{
    // 发牌区域的扑克牌
    m_baseCard = new CardPanel(this);
    m_baseCard->setImage(m_cardBackImg,m_cardBackImg);
    // 发牌过程移动的扑克牌
    m_moveCard = new CardPanel(this);
    m_moveCard->setImage(m_cardBackImg,m_cardBackImg);
    // 最后三张底牌
    for(int i=0; i<3; ++i){
        CardPanel* panel = new CardPanel(this);
        panel->setImage(m_cardBackImg,m_cardBackImg);
        panel->hide();
        m_last3Card.push_back(panel);
    }
    // 扑克牌的位置
    m_baseCardPos = QPoint((width()-m_cardSize.width())/2,
                           (height()-m_cardSize.height())/2-100);
    m_baseCard->move(m_baseCardPos);
    m_moveCard->move(m_baseCardPos);

    int base = (width()-3*m_cardSize.width() - 2*10)/2;
    for(int i=0;i<3;++i){
        m_last3Card[i]->move(base+(m_cardSize.width()+10)*i,20);
    }
}

void GamePanel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_bkImage);
}

