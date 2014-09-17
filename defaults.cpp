#include "controller.h"

ControllerParams cparams = [] {
   // motor
   cparams.accelAngle = 20.0;
   cparams.minDuty = 15;
   cparams.maxDuty = 30;
   cparams.invertMotorPolarity = false;

   // angle conversions
   CookedAngle::hardwareOrigin = RawAngle(250 - 20);
   CookedAngle::inverted = false;
   UserAngle::userOrigin = CookedAngle(20);

   // angle parameters
   cparams.minimumSafeAngle = CookedAngle(335);
   cparams.maximumSafeAngle = CookedAngle(315);
   cparams.tolerance = 0.1;

   // control loop parameters
   cparams.loopDelay = std::chrono::milliseconds(10);
   
   return cparams;
}();