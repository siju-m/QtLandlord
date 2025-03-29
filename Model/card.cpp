#include "card.h"

Card::Card() {}

void Card::setPoint(CardPoint point)
{
    m_point = point;
}

void Card::setSuit(CardSuit suit)
{
    m_suit = suit;
}

Card::CardPoint Card::point()
{
    return m_point;
}

Card::CardSuit Card::suit()
{
    return m_suit;
}
