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

    // 定时器实例化
    m_timer = new QTimer(this);
    connect(m_timer,&QTimer::timeout,this,&GamePanel::onDispatchCard);

    m_animation = new AnimationWindow(this);
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

    connect(m_gameCtl,&GameControl::playerStatusChanged,this,&GamePanel::onPlayerStatusChanged);
    connect(m_gameCtl,&GameControl::notifyGrabLordBet,this,&GamePanel::onGrabLordBet);
    connect(m_gameCtl,&GameControl::gameStatusChanged,this,&GamePanel::gameStatusProcess);
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
    for(int i=0,suit = Card::Suit_Begin+1; suit<Card::Suit_End; ++suit,++i){
        for(int j=0,point = Card::Card_Begin+1; point<Card::Card_SJ; ++point,++j){
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

    connect(ui->btnGroup, &ButtonGroup::startGame, this, [=](){
        // 界面初始化
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        m_gameCtl->clearPlayerScore();
        updatePlayerScore();
        // 修改游戏状态为发牌
        gameStatusProcess(GameControl::DispatchCard);
    });
    connect(ui->btnGroup, &ButtonGroup::playHand, this, [=](){});
    connect(ui->btnGroup, &ButtonGroup::pass, this, [=](){});
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](int bet){
        m_gameCtl->getUserPlayer()->grabLordBet(bet);
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
    });
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
                           height()/2-100);
    m_baseCard->move(m_baseCardPos);
    m_moveCard->move(m_baseCardPos);

    int base = (width()-3*m_cardSize.width() - 2*10)/2;
    for(int i=0;i<3;++i){
        m_last3Card[i]->move(base+(m_cardSize.width()+10)*i,20);
    }
}

void GamePanel::gameStatusProcess(GameControl::GameStatus status)
{
    // 记录游戏状态
    m_gameStatus = status;
    // 处理游戏状态
    switch(status){
    case GameControl::DispatchCard:
        startDispatchCard();
        break;
    case GameControl::CallingLord:{
        // 取出底牌数据
        CardList last3Card = m_gameCtl->getSurplusCards().toCardList();
        // 给底牌设置图片
        for(int i=0; i<last3Card.size(); ++i){
            QPixmap front = m_cardMap[last3Card.at(i)]->getImage();
            m_last3Card[i]->setImage(front,m_cardBackImg);
            m_last3Card[i]->hide();
        }
        // 开始叫地主
        m_gameCtl->startLordCard();
        }break;
    case GameControl::PlayingHand:

        break;
    default:break;
    }
}

void GamePanel::startDispatchCard()
{
    // 重置每张卡牌的属性
    for(auto &it : m_cardMap){
        it->setSelected(false);
        it->setFrontSide(true);
        it->hide();
    }
    // 隐藏三张底牌
    for(int i=0; i<m_last3Card.size(); ++i){
        m_last3Card.at(i)->hide();
    }
    // 重置玩家的窗口上下文信息
    int index = m_playerList.indexOf(m_gameCtl->getUserPlayer());
    for(int i=0; i<m_playerList.size(); ++i){
        m_contextMap[m_playerList.at(i)].lastCards.clear();
        m_contextMap[m_playerList.at(i)].info->hide();
        m_contextMap[m_playerList.at(i)].roleImg->hide();
        m_contextMap[m_playerList.at(i)].isFrontSide = (i==index);
    }
    // 重置所有玩家手牌
    m_gameCtl->resetCardData();
    // 显示底牌
    m_baseCard->show();
    // 隐藏按钮面板
    ui->btnGroup->selectPanel(ButtonGroup::Empty);
    // 启动定时器
    m_timer->start(10);
    // 播放背景音乐
}

void GamePanel::onDispatchCard()
{
    // 记录扑克牌的位置
    static int curMovePos = 0;
    // 当前玩家
    Player* curPlayer = m_gameCtl->getCurrentPlayer();
    if(curMovePos >= 100){
        // 增加玩家手牌
        Card card = m_gameCtl->takeOneCard();
        curPlayer->storeDispatchCard(card);
        Cards cards(card);
        disposCard(curPlayer,cards);
        // 切换玩家
        m_gameCtl->setCurrentPlayer(curPlayer->getNextPlayer());
        curMovePos = 0;
        // 发牌动画
        cardMoveStep(curPlayer,curMovePos);
        // 判断牌是否发完
        if(m_gameCtl->getSurplusCards().cardCount() == 3){
            // 终止定时器
            m_timer->stop();
            // 切换游戏状态
            gameStatusProcess(GameControl::CallingLord);
            return;
        }
    }
    // 移动扑克牌
    cardMoveStep(curPlayer,curMovePos);
    curMovePos += 15;
}

void GamePanel::onPlayerStatusChanged(Player *player, GameControl::PlayerStatus status)
{
    switch (status) {
    case GameControl::ThinkingForCallLord:
        if(player == m_gameCtl->getUserPlayer()){
            ui->btnGroup->selectPanel(ButtonGroup::CallLord,m_gameCtl->getPlayerMaxBet());
            m_baseCard->hide();
            m_moveCard->hide();
        }
        break;
    case GameControl::ThinKingForplayHand:

        break;
    case GameControl::Winning:

        break;
    default:
        break;
    }
}

void GamePanel::onGrabLordBet(Player *player, int bet, bool flag)
{
    // 显示抢地主的信息提示
    PlayerContext context = m_contextMap[player];
    if(bet == 0){
        context.info->setPixmap(QPixmap(":/images/buqiang.png"));
    }else{
        if(flag){
            context.info->setPixmap(QPixmap(":/images/jiaodizhu.png"));
        }else{
            context.info->setPixmap(QPixmap(":/images/qiangdizhu.png"));
        }
    }
    context.info->show();

    // 显示叫地主的分数
    showAnimation(AnimationType::Bet, bet);
    // 播放分数的背景音乐
}

void GamePanel::showAnimation(AnimationType type, int bet)
{
    switch(type)
    {
    case AnimationType::LianDui:
    case AnimationType::ShunZi:
        // m_animation->setFixedSize(250, 150);
        // m_animation->move((width()-m_animation->width())/2, 200);
        // m_animation->showSequence((AnimationWindow::Type)type);
        break;
    case AnimationType::Plane:
        // m_animation->setFixedSize(800, 75);
        // m_animation->move((width()-m_animation->width())/2, 200);
        // m_animation->showPlane();
        break;
    case AnimationType::Bomb:
        // m_animation->setFixedSize(180, 200);
        // m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        // m_animation->showBomb();
        break;
    case AnimationType::JokerBomb:
        // m_animation->setFixedSize(250, 200);
        // m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        // m_animation->showJokerBomb();
        break;
    case AnimationType::Bet:
        m_animation->setFixedSize(160, 98);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2-140);
        m_animation->showBetScore(bet);
        break;
    }
    m_animation->show();
}

void GamePanel::cardMoveStep(Player *player, int curPos)
{
    // 得到每个玩家的扑克牌展示区域
    QRect cardRect = m_contextMap[player].cardRect;
    // 每个玩家的单元步长
    int unit[] = {
        (m_baseCardPos.x() - cardRect.right())/100,
        (cardRect.left() - m_baseCardPos.x())/100,
        (cardRect.top() - m_baseCardPos.y())/100
    };
    // 每次窗口移动时每个玩家对应的牌的实时位置
    QPoint pos[] = {
        QPoint(m_baseCardPos.x() - curPos * unit[0], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x() + curPos * unit[1], m_baseCardPos.y()),
        QPoint(m_baseCardPos.x(), m_baseCardPos.y() + curPos * unit[2])
    };
    // 移动扑克牌
    int index = m_playerList.indexOf(player);
    m_moveCard->move(pos[index]);

    // 临界状态处理
    if(curPos == 0){
        m_moveCard->show();
    }
    if(curPos == 100){
        m_moveCard->hide();
    }
}

void GamePanel::disposCard(Player *player, Cards &cards)
{
    Cards& myCard = const_cast<Cards&>(cards);
    CardList list = myCard.toCardList();
    for(int i=0; i<list.size(); ++i){
        CardPanel* panel = m_cardMap[list.at(i)];
        if(panel)
            panel->setOwner(player);
    }
    updatePlayerCards(player);
}

void GamePanel::updatePlayerCards(Player *player)
{
    Cards cards = player->getCards();
    CardList cardList = cards.toCardList();
    // 取出展示扑克牌的区域
    int cardSpace = 20;
    QRect cardRect = m_contextMap[player].cardRect;
    for(int i=0; i<cardList.size(); ++i){
        CardPanel* panel = m_cardMap[cardList.at(i)];
        panel->show();
        panel->raise();
        panel->setFrontSide(m_contextMap[player].isFrontSide);

        // ui->test1->addWidget(panel);
        // 水平或垂直展示
        if(m_contextMap[player].align == Horizontal){
            int leftX = cardRect.left() + (cardRect.width() - (cardList.size() - 1) * cardSpace - panel->width()) / 2;
            int topY = cardRect.top() + (cardRect.height() - m_cardSize.height()) /2;
            if(panel->isSelected()){
                topY-=10;
            }
            panel->move(leftX + cardSpace*i, topY);
        }else{
            int leftX = cardRect.left() + (cardRect.width() - m_cardSize.width()) / 2;
            int topY = cardRect.top() + (cardRect.height() - (cardList.size() - 1) * cardSpace - panel->height()) /2;
            panel->move(leftX, topY+ i * cardSpace);
        }

    }
}

void GamePanel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_bkImage);
}

