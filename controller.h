#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <chrono>
#include <exception>
#include "angles.h"
#include "interface.h"

class ConfigFileException : public std::exception
{
public:
   ConfigFileException(const std::string& what) : message(what) {}
   inline const char* what() { return message.c_str(); }
   const std::string message;
};


struct ControllerParams
{
   ControllerParams() = default;
   ControllerParams(const char* filename);

    // motor parameters
   unsigned short minDuty = 10;
   unsigned short maxDuty = 100;
   bool invertMotorPolarity = false;
   std::chrono::milliseconds stallCheckPeriod{1000};
   degrees stallThreshold = 0;
   unsigned short destallDuty = 0;
   std::chrono::milliseconds destallDuration{0};

   // movement parameters
   CookedAngle parkPosition = CookedAngle(0);
   degrees accelAngle = 20.0;
   degrees tolerance = 0.1;

   // control loop parameters
   std::chrono::milliseconds loopDelay{10};
   enum class IndicatorStyle { Bar, Percent } indicatorStyle = IndicatorStyle::Bar;
};


class Controller
{
public:
   Controller(ControllerParams initialParams);
   RawAngle getRawAngle() const;
   CookedAngle getCookedAngle() const;
   UserAngle getUserAngle() const;
   void slew(CookedAngle targetAngle);

private:
   enum class MotorStatus { Undetermined, OK, Stalled, WrongDirection };

   void beginMotorMonitoring(const CookedAngle currentAngle);
   MotorStatus checkMotor(const CookedAngle currentAngle,
                          const float wantedDirection);

   ControllerParams params;
   Motor* motor;
   Sensor* sensor;

   CookedAngle stallCheckAngle{0};
   std::chrono::steady_clock::time_point stallCheckTime;
};

#endif // CONTROLLER_H
