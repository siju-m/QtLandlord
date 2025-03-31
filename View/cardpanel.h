#ifndef CARDPANEL_H
#define CARDPANEL_H

#include <Card.h>
#include <Player.h>
#include <QWidget>
// #include <../Model/card.h>

class CardPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CardPanel(QWidget *parent = nullptr);

    //设置获取图片
    void setImage(QPixmap &front,QPixmap &back);
    QPixmap getImage();

    //扑克牌显示正反
    void setFrontSide(bool flag);
    bool isFrontSide();

    //窗口是否被选中
    void setSelected(bool flag);
    bool isSelected();

    //卡牌花色及点数
    void setCard(Card& card);
    Card getCard();

    //扑克牌所有者
    void setOwner(Player *player);
    Player *getOwner();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    QPixmap m_front;
    QPixmap m_back;
    bool m_isFront;
    bool m_isSelected;
    Card m_card;
    Player *m_owner;

signals:
};

#endif // CARDPANEL_H
