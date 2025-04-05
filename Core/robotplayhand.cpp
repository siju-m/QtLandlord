#include "robotplayhand.h"

RobotPlayHand::RobotPlayHand(Robot* robot, QObject *parent)
    : QThread{parent}
    , m_robot(robot)
{}

void RobotPlayHand::run()
{
    msleep(2000);
    m_robot->thinkPlayHand();
}
