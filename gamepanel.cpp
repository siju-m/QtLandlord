#include "gamepanel.h"
#include "./ui_gamepanel.h"
#include "PlayHand.h"
#include "endingpanel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
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

    // 倒计时窗口初始化
    initCountDown();

    // 定时器实例化
    m_timer = new QTimer(this);
    connect(m_timer,&QTimer::timeout,this,&GamePanel::onDispatchCard);

    m_animation = new AnimationWindow(this);
    m_bgm = new BGMControl(this);
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
    connect(m_gameCtl, &GameControl::notifyPlayHand, this, &GamePanel::onDisposePlayHand);

    connect(leftRobot, &Player::notifyPickCards, this, &GamePanel::disposeCard);
    connect(rightRobot, &Player::notifyPickCards, this, &GamePanel::disposeCard);
    connect(user, &Player::notifyPickCards, this, &GamePanel::disposeCard);
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

    connect(panel, &CardPanel::cardSelected, this, &GamePanel::onCardSelected);
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
        m_bgm->startBGM(0.8);
    });
    connect(ui->btnGroup, &ButtonGroup::playHand, this, &GamePanel::onUserPlayHand);
    connect(ui->btnGroup, &ButtonGroup::pass, this, &GamePanel::onUserPass);
    connect(ui->btnGroup, &ButtonGroup::betPoint, this, [=](int bet){
        m_gameCtl->getUserPlayer()->grabLordBet(bet);
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
    });
}

void GamePanel::initPlayerContext()
{
    // 放置玩家扑克牌的位置
    QRect cardsRect[]={
        QRect(90, 130, 100, height()-200),
        QRect(rect().right()-190, 130, 100, height()-200),
        QRect(250, rect().bottom()-120, width()-500, 100)
    };
    // 玩家出牌的区域
    QRect playHandRect[]={
        QRect(260, 150, 100, 100),
        QRect(rect().right()-360, 150, 100, 100),
        QRect(150, rect().bottom()-290, width()-300, 105)
    };
    // 玩家头像显示的位置
    QPoint roleImgRect[]={
        QPoint(cardsRect[0].left() - 80, cardsRect[0].height() / 2 + 20),
        QPoint(cardsRect[1].right() + 10, cardsRect[1].height() / 2 + 20),
        QPoint(cardsRect[2].right() - 10, cardsRect[2].top() - 10)
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
        // 隐藏发牌区的底牌和移动的牌
        m_baseCard->hide();
        m_moveCard->hide();
        // 取出底牌数据
        CardList last3Card = m_gameCtl->getSurplusCards().toCardList();
        // 给底牌设置图片
        for(int i=0; i<last3Card.size(); ++i){
            QPixmap front = m_cardMap[last3Card.at(i)]->getImage();
            m_last3Card[i]->setImage(front,m_cardBackImg);
            m_last3Card[i]->setFrontSide(false);
            m_last3Card[i]->show();
        }
        // 开始叫地主
        m_gameCtl->startLordCard();
        }break;
    case GameControl::PlayingHand:
        // 显示地主的三张底牌
        for(auto &it : m_last3Card){
            it->setFrontSide(true);
        }
        update();

        for(auto &it : m_playerList){
            PlayerContext &context = m_contextMap[it];
            // 隐藏各个玩家在抢地主过程中的提示信息
            context.info->hide();
            // 显示玩家头像
            QPixmap pixmap = loadRoleImage(it->getSex(), it->getDirection(), it->getRole());
            context.roleImg->setPixmap(pixmap);
            context.roleImg->show();
        }

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
    m_bgm->playAssistMusic(BGMControl::Dispatch);
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
        // disposCard(curPlayer,cards);
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
            // 终止发牌音乐
            m_bgm->stopAssistMusic();
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
        }
        break;
    case GameControl::ThinKingForplayHand:
        // 隐藏玩家上一轮打出的牌
        hidePlayerDropCards(player);
        if(player == m_gameCtl->getUserPlayer()){
            Player* pendPlayer = m_gameCtl->getPendPlayer();
            if(pendPlayer == m_gameCtl->getUserPlayer() || pendPlayer == nullptr){
                ui->btnGroup->selectPanel(ButtonGroup::PlayCard);
            }else{
                ui->btnGroup->selectPanel(ButtonGroup::PassOrPlay);
            }
        }else{
            ui->btnGroup->selectPanel(ButtonGroup::Empty);
        }
        break;
    case GameControl::Winning:
        m_bgm->stopBGM();
        m_contextMap[m_gameCtl->getLeftRobot()].isFrontSide = true;
        m_contextMap[m_gameCtl->getRightRobot()].isFrontSide = true;
        updatePlayerCards(m_gameCtl->getLeftRobot());
        updatePlayerCards(m_gameCtl->getRightRobot());
        // 更新玩家得分
        updatePlayerScore();
        m_gameCtl->setCurrentPlayer(player);
        showEndingScorePanel();
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
        // 显示叫地主的分数
        showAnimation(AnimationType::Bet, bet);
    }
    context.info->show();

    // 播放分数的背景音乐
    m_bgm->playerRolordMusic(bet, (BGMControl::RoleSex)player->getSex(), flag);
}

void GamePanel::onDisposePlayHand(Player *player, Cards &cards)
{
    // 更新玩家打出的牌
    auto it = m_contextMap.find(player);
    it->lastCards = cards;
    // 根据牌型播放游戏特效
    PlayHand hand(cards);
    PlayHand::HandType type = hand.getHandType();
    if(type == PlayHand::Hand_Plane ||
        type == PlayHand::Hand_Plane_Two_Pair ||
        type == PlayHand::Hand_Plane_Two_Single){
        showAnimation(Plane);
    }else if(type == PlayHand::Hand_Seq_Single){
        showAnimation(ShunZi);
    }else if(type == PlayHand::Hand_Seq_Pair){
        showAnimation(LianDui);
    }else if(type == PlayHand::Hand_Bomb){
        showAnimation(Bomb);
    }else if(type == PlayHand::Hand_Bomb_Jokers){
        showAnimation(JokerBomb);
    }
    // 如果玩家打出的是空牌，显示提示信息
    if(cards.isEmpty()){
        it->info->setPixmap(QPixmap(":/images/pass.png"));
        it->info->show();
        m_bgm->playPassMusic((BGMControl::RoleSex)player->getSex());
    }else{
        bool isFirst = (m_gameCtl->getPendPlayer() == player || m_gameCtl->getPendPlayer() == nullptr);
        m_bgm->playCardMusic(cards, isFirst, (BGMControl::RoleSex)player->getSex());
    }
    // 更新玩家剩余的牌
    updatePlayerCards(player);
    // 播放剩余牌提示音

    if(player->getCards().cardCount() == 2 && !player->getIsNotice(2)){
        m_bgm->playLastMusic(BGMControl::Last2, (BGMControl::RoleSex)player->getSex());
        player->setHasNotice(2);
    }else if(player->getCards().cardCount() == 1 && !player->getIsNotice(1)){
        m_bgm->playLastMusic(BGMControl::Last1, (BGMControl::RoleSex)player->getSex());
        player->setHasNotice(1);
    }
}

void GamePanel::onCardSelected(Qt::MouseButton button)
{
    // 判断是否是出牌状态
    if(m_gameStatus == GameControl::DispatchCard ||
        m_gameStatus == GameControl::CallingLord)
        return;

    // 判断发出信号的牌的所有者是不是用户玩家
    CardPanel* panel = (CardPanel*)sender();
    if(panel->getOwner() != m_gameCtl->getUserPlayer())
        return;

    // 保存当前被选中的牌的窗口对象
    m_curSelCard = panel;
    // 根据鼠标按键决定行为
    if(button == Qt::LeftButton){
        // 设置扑克牌的选中状态
        panel->setSelected(!panel->isSelected());
        // 更新扑克牌在窗口中的显示
        updatePlayerCards(panel->getOwner());
        // 保存或删除扑克牌窗口对象
        QSet<CardPanel*>::ConstIterator it =m_selectCards.find(panel);
        if(it == m_selectCards.constEnd()){
            m_selectCards.insert(panel);
        }else{
            m_selectCards.erase(it);
        }
        m_bgm->playAssistMusic(BGMControl::SelectCard);
    }else if(button == Qt::RightButton){
        // 调用出牌的槽函数
        onUserPlayHand();
    }
}

void GamePanel::onUserPlayHand()
{
    // 判断游戏状态
    if(m_gameStatus != GameControl::PlayingHand)
        return;
    // 判断玩家是不是用户玩家
    if(m_gameCtl->getCurrentPlayer() != m_gameCtl->getUserPlayer())
        return;
    // 判断要出的牌是否为空
    if(m_selectCards.isEmpty())
        return;
    // 得到要打出牌的牌型
    Cards cs;
    for(auto it = m_selectCards.begin(); it!=m_selectCards.end(); ++it){
        Card card = (*it)->getCard();
        cs.add(card);
    }
    PlayHand hand(cs);
    PlayHand::HandType type = hand.getHandType();
    if(type == PlayHand::Hand_Unknown)
        return;
    // 判断当前玩家的牌能否压住上一家的牌
    if(m_gameCtl->getPendPlayer() != m_gameCtl->getUserPlayer()){
        Cards cards = m_gameCtl->getPendCards();
        if(!hand.canBeat(PlayHand(cards))){
            return;
        }
    }
    // 停止倒计时
    m_countDown->stopCountDown();
    // 通过玩家对象出牌
    m_gameCtl->getUserPlayer()->playHand(cs);
    // 清空容器
    m_selectCards.clear();
}

void GamePanel::onUserPass()
{
    // 停止倒计时
    m_countDown->stopCountDown();
    // 判断是不是用户玩家
    Player* userPlayer = m_gameCtl->getUserPlayer();
    Player* curPlayer = m_gameCtl->getCurrentPlayer();
    if(userPlayer != curPlayer)
        return;
    // 判断当前玩家是不是上次出牌的玩家
    Player* pendPlayer = m_gameCtl->getPendPlayer();
    if(pendPlayer == userPlayer || pendPlayer == nullptr)
        return;
    // 打出一个空的Cards对象
    Cards empty;
    userPlayer->playHand(empty);
    // 清空用户选择但未打出的牌
    for(auto it = m_selectCards.begin(); it!=m_selectCards.end(); ++it){
        (*it)->setSelected(false);
    }
    m_selectCards.clear();
    // 更新玩家手牌
    updatePlayerCards(userPlayer);
}

void GamePanel::showAnimation(AnimationType type, int bet)
{
    switch(type)
    {
    case AnimationType::LianDui:
    case AnimationType::ShunZi:
        m_animation->setFixedSize(250, 150);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showSequence((AnimationWindow::Type)type);
        break;
    case AnimationType::Plane:
        m_animation->setFixedSize(800, 75);
        m_animation->move((width()-m_animation->width())/2, 200);
        m_animation->showPlane();
        break;
    case AnimationType::Bomb:
        m_animation->setFixedSize(180, 200);
        m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        m_animation->showBomb();
        break;
    case AnimationType::JokerBomb:
        m_animation->setFixedSize(250, 200);
        m_animation->move((width()-m_animation->width())/2, (height() - m_animation->height()) / 2 - 70);
        m_animation->showJokerBomb();
        break;
    case AnimationType::Bet:
        m_animation->setFixedSize(160, 98);
        m_animation->move((width()-m_animation->width())/2, (height()-m_animation->height())/2-140);
        m_animation->showBetScore(bet);
        break;
    }
    m_animation->show();
}

void GamePanel::hidePlayerDropCards(Player *player)
{
    auto it = m_contextMap.find(player);
    if(it != m_contextMap.end()){
        if(it->lastCards.isEmpty()){
            it->info->hide();
        }else{
            CardList list = it->lastCards.toCardList();
            for(const auto &it : list){
                m_cardMap[it]->hide();
            }
        }
        it->lastCards.clear();
    }
}

void GamePanel::showEndingScorePanel()
{
    bool isLord = m_gameCtl->getUserPlayer()->getRole() == Player::Lord;
    bool isWin = m_gameCtl->getUserPlayer()->isWin();
    EndingPanel* panel = new EndingPanel(isLord, isWin, this);
    panel->show();
    panel->move((width()-panel->width())/2, -panel->height());
    panel->setPlayerScore(m_gameCtl->getLeftRobot()->getScore(),
                          m_gameCtl->getRightRobot()->getScore(),
                          m_gameCtl->getUserPlayer()->getScore());

    m_bgm->playEndingMusic(isWin);

    QPropertyAnimation *animation = new QPropertyAnimation(panel, "geometry", this);
    // 动画的持续时间
    animation->setDuration(1500);
    // 窗口的起始位置和终止位置
    animation->setStartValue(QRect(panel->x(), panel->y(), panel->width(), panel->height()));
    animation->setEndValue(QRect((width()-panel->width())/2, (height()-panel->height())/2,panel->width(), panel->height()));
    // 窗口的运动曲线
    animation->setEasingCurve(QEasingCurve(QEasingCurve::OutBounce));
    // 播放动画
    animation->start();
    //处理窗口信号
    connect(panel, &EndingPanel::continueGame, this, [=](){
        panel->close();
        panel->deleteLater();
        animation->deleteLater();
        ui->btnGroup->selectPanel(ButtonGroup::Empty);
        gameStatusProcess(GameControl::DispatchCard);
        m_bgm->startBGM(0.8);
    });
}

void GamePanel::initCountDown()
{
    m_countDown = new CountDown(this);
    m_countDown->move((width()-m_countDown->width())/2, (height()-m_countDown->height())/2);
    connect(m_countDown, &CountDown::notMuchTime, this, [=](){
        //播放提示音乐
        m_bgm->playAssistMusic(BGMControl::Alert);
    });
    connect(m_countDown, &CountDown::timeout, this, &GamePanel::onUserPass);
    UserPlayer* userPlayer = m_gameCtl->getUserPlayer();
    connect(userPlayer,&UserPlayer::startCountDown, this, [=](){
        if(m_gameCtl->getPendPlayer() != userPlayer && m_gameCtl->getPendPlayer() != nullptr)
            m_countDown->showCountDown();
    });
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

void GamePanel::disposeCard(Player *player, const Cards &cards)
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

    m_cardsRect = QRect();
    m_userCards.clear();
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

            m_cardsRect = QRect(leftX, topY,cardSpace * i + m_cardSize.width(), m_cardSize.height());
            int curWidth = 0;
            if(cardList.size() - 1 == i){
                curWidth = m_cardSize.width();
            }else{
                curWidth = cardSpace;
            }
            QRect cardRect(leftX + cardSpace * i, topY, curWidth, m_cardSize.height());
            m_userCards.insert(panel, cardRect);

        }else{
            int leftX = cardRect.left() + (cardRect.width() - m_cardSize.width()) / 2;
            int topY = cardRect.top() + (cardRect.height() - (cardList.size() - 1) * cardSpace - panel->height()) /2;
            panel->move(leftX, topY+ i * cardSpace);
        }
    }


    // 显示玩家打出的牌
    // 得到当前玩家的出牌区域以及本轮打出的牌
    QRect playCardRect = m_contextMap[player].playHandRect;
    Cards lastCards = m_contextMap[player].lastCards;
    if(!lastCards.isEmpty())
    {
        int playSpacing = 24;
        CardList lastCardList = lastCards.toCardList();
        CardList::ConstIterator itplayed = lastCardList.constBegin();
        for(int i=0; itplayed != lastCardList.constEnd(); ++itplayed, i++)
        {
            CardPanel* panel = m_cardMap[*itplayed];
            panel->setFrontSide(true);
            panel->raise();
            // 将打出的牌移动到出牌区域
            if(m_contextMap[player].align == Horizontal)
            {
                int leftBase = playCardRect.left() +
                               (playCardRect.width() - (lastCardList.size() - 1) * playSpacing - panel->width()) / 2;
                int top = playCardRect.top() + (playCardRect.height() - panel->height()) /2 ;
                panel->move(leftBase + i * playSpacing, top);
            }
            else
            {
                int left = playCardRect.left() + (playCardRect.width() - panel->width()) / 2;
                int top = playCardRect.top() ;
                panel->move(left, top + i * playSpacing);
            }
            panel->show();
        }
    }
}

QPixmap GamePanel::loadRoleImage(Player::Sex sex, Player::Direction direct, Player::Role role)
{
    QVector<QString> lordMan;
    QVector<QString> lordWoman;
    QVector<QString> famerMan;
    QVector<QString> famerWoman;
    lordMan << ":/images/lord_man_1.png" << ":/images/lord_man_2.png";
    lordWoman << ":/images/lord_woman_1.png" << ":/images/lord_woman_2.png";
    famerMan << ":/images/farmer_man_1.png" << ":/images/farmer_man_2.png";
    famerWoman << ":/images/farmer_woman_1.png" << ":/images/farmer_woman_2.png";
    QImage image;//只有QImage可以对图片进行方向转换
    int random = QRandomGenerator::global()->bounded(2);
    if(sex == Player::Man && role == Player::Lord){
        image.load(lordMan.at(random));
    }else if(sex == Player::Man && role == Player::Farmer){
        image.load(famerMan.at(random));
    }else if(sex == Player::Woman && role == Player::Lord){
        image.load(lordWoman.at(random));
    }else if(sex == Player::Woman && role == Player::Farmer){
        image.load(famerWoman.at(random));
    }
    QPixmap pixmap;
    if(direct == Player::left){
        pixmap = QPixmap::fromImage(image);
    }else{
        pixmap = QPixmap::fromImage(image.mirrored(true, false));
    }
    return pixmap;
}

void GamePanel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.drawPixmap(rect(),m_bkImage);
}

void GamePanel::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton){
        QPoint pt = event->pos();
        if(!m_cardsRect.contains(pt)){
            m_curSelCard = nullptr;
        }else{
            QList<CardPanel*> list = m_userCards.keys();
            for(const auto &it : list){
                if(m_userCards[it].contains(pt) && m_curSelCard != it){
                    it->clicked();
                    m_curSelCard = it;
                }
            }
        }
    }
}

