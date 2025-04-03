#ifndef ROBOTGRAPLORD_H
#define ROBOTGRAPLORD_H

#include "Robot.h"
#include <QObject>
#include <QThread>

class RobotGrapLord : public QThread
{
    Q_OBJECT
public:
    explicit RobotGrapLord(Robot* robot, QObject *parent = nullptr);

protected:
    void run();
private:
    Robot* m_robot;
};

#endif // ROBOTGRAPLORD_H
