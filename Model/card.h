#ifndef CARD_H
#define CARD_H

#include <QVector>


class Card
{
public:
    //花色
    enum CardSuit{
        Suit_Begin,
        Diamond,
        Club,
        Heart,
        Spade,
        Suit_End
    };
    //点数
    enum CardPoint{
        Card_Begin,
        Card_3,
        Card_4,
        Card_5,
        Card_6,
        Card_7,
        Card_8,
        Card_9,
        Card_10,
        Card_J,
        Card_Q,
        Card_K,
        Card_A,
        Card_2,
        Card_SJ,
        Card_BJ,
        Card_End
    };

    Card();

    void setPoint(CardPoint point);
    void setSuit(CardSuit suit);
    CardPoint point() const;
    CardSuit suit() const;
private:
    CardSuit m_suit;
    CardPoint m_point;
};

//对象比较
bool lessSort(const Card &c1,const Card &c2);
bool greaterSort(const Card &c1,const Card &c2);

/*下面两个重载和重写是由于QSet默认只能使用基础数据类型*/
// 操作符重载 (==)
bool operator ==(const Card &left ,const Card &right);

//重写全局函数 qHash
uint qHash(const Card& card);

using CardList = QVector<Card>;

#endif // CARD_H
