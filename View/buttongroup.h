#ifndef BUTTONGROUP_H
#define BUTTONGROUP_H

#include <QWidget>

namespace Ui {
class ButtonGroup;
}

class ButtonGroup : public QWidget
{
    Q_OBJECT

public:
    enum Panel{Start,PlayCard,PassOrPlay,CallLord,Empty};
    explicit ButtonGroup(QWidget *parent = nullptr);
    ~ButtonGroup();

    void initButtons();

    // 处理page页切换
    void selectPanel(Panel type);
signals:
    // 开始游戏
    void startGame();
    // 出牌
    void playHand();
    // 不出牌
    void pass();
    // 抢地主
    void betPoint(int point);
private:
    Ui::ButtonGroup *ui;
};

#endif // BUTTONGROUP_H
