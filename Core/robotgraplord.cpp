#include "robotgraplord.h"
#include <qdebug.h>

RobotGrapLord::RobotGrapLord(Robot *robot, QObject *parent)
    : QThread(parent)
    , m_robot(robot)
{

}

void RobotGrapLord::run()
{
    msleep(2000);
    m_robot->thinkCallLord();
}
