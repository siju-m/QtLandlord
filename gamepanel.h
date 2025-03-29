#ifndef GAMEPANEL_H
#define GAMEPANEL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class GamePanel;
}
QT_END_NAMESPACE

class GamePanel : public QMainWindow
{
    Q_OBJECT

public:
    GamePanel(QWidget *parent = nullptr);
    ~GamePanel();

private:
    Ui::GamePanel *ui;
};
#endif // GAMEPANEL_H
