#ifndef CARDS_H
#define CARDS_H

#include <Card.h>
#include <QSet>


class Cards
{
public:
    enum SortType{Asc, Desc, NoSort};
    Cards();

    //添加扑克牌
    void add(const Card &card);
    void add(const Cards &cards);

    //一次插入多个数据
    Cards& operator <<(const Card& card);
    Cards& operator <<(const Cards& cards);

    //删除扑克牌
    void remove(Card &card);
    void remove(Cards &cards);

    //扑克牌的数量
    int cardCount();
    //是否为空
    bool isEmpty();
    //清空扑克牌
    void clear();

    //最大点数
    Card::CardPoint maxPoint();
    //最小点数
    Card::CardPoint minPoint();
    //指定点数的牌的数量
    int pointCount(Card::CardPoint point);
    //某张牌是否在集合中
    bool contains(const Card &card);
    bool contains(const Cards &cards);
    //随机取出一张牌
    Card takeRandCard();

    CardList toCardList(SortType type = Desc);
private:
    QSet<Card> m_cards;
};

#endif // CARDS_H
