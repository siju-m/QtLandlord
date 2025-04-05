#ifndef ROBOTPLAYHAND_H
#define ROBOTPLAYHAND_H

#include "Robot.h"
#include <QObject>
#include <QThread>

class RobotPlayHand : public QThread
{
    Q_OBJECT
public:
    explicit RobotPlayHand(Robot* robot, QObject *parent = nullptr);

signals:

protected:
    void run() override;
private:
    Robot* m_robot;
};

#endif // ROBOTPLAYHAND_H
