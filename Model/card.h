#ifndef CARD_H
#define CARD_H

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
    CardPoint point();
    CardSuit suit();
private:
    CardSuit m_suit;
    CardPoint m_point;
};

#endif // CARD_H
