/*
 *    mcontrol, declination axis control for PAART, the radiotelescope of
 *              Astronomical Society Vega - Ljubljana
 *
 *    Copyright (C) 2014 Andrej Lajovic <andrej.lajovic@ad-vega.si>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ANGLES_H
#define ANGLES_H

#include <vector>

/* Explanation: raw, cooked and user angles:
 *
 * Raw angles are readouts that come directly from the sensor. These lay in
 * the range [0:360]
 *
 * Cooked angles are linearized and possibly inverted raw angles with the
 * origin positioned so that values around zero and 360 never occur (we are
 * relying on hardware end switches to prevent that). Still in the range
 * [0:360]. This is the type the controller uses in its calculations.
 *
 * User angles are cooked angles shifted by a user-defined value (to
 * reposition the origin). Values lie in the range [-origin:360-origin].
 * User angles are meant to be used purely within the context of interaction
 * with the user and not for other internal calculations.
*/

class RawAngle;
class CookedAngle;
class UserAngle;

typedef float degrees;

degrees mod360(degrees value);

/* RawAngle is not a subclass of Angle (declared below) because it does not
 * represent a true angle due to the possible nonlinearities in the
 * measurement. No arithmetic, then: just values.
*/

class RawAngle
{
public:
   explicit RawAngle(degrees value) : val(value) {}
   degrees val;

   // RawAngle arithmetic should ensure that the result never goes outside the
   // range [0:360).
   inline RawAngle operator+(const degrees& deg) const
      { return RawAngle(mod360(val + deg)); }
};


/* A general angle class.
 *
 * We use a template to keep all operators in one place but prevent
 * operations between incompatible derived angle types. I.e. calculating
 * (UserAngle(degrees) - CookedAngle(degrees)) makes no sense and should
 * therefore throw a compile-time error.
*/

template <class DerivedAngle>
class Angle
{
public:
   Angle() = default;
   explicit Angle(degrees value) : val(value) {}

   // comparison operators
   inline bool operator>(const DerivedAngle& other) const { return val > other.val; }
   inline bool operator<(const DerivedAngle& other) const { return val < other.val; }
   inline bool operator>=(const DerivedAngle& other) const { return val >= other.val; }
   inline bool operator<=(const DerivedAngle& other) const { return val <= other.val; }

   // difference between two angles in degrees
   inline degrees operator-(const DerivedAngle& other) const { return val - other.val; }

   inline DerivedAngle operator+(const degrees& deg) const { return DerivedAngle(val + deg); }
   inline DerivedAngle operator-(const degrees& deg) const { return DerivedAngle(val - deg); }

   degrees val;
};


class CookedAngle : public Angle<CookedAngle>
{
public:
   explicit CookedAngle(degrees value) : Angle(value) {};
   explicit CookedAngle(const RawAngle raw);
   explicit CookedAngle(const UserAngle user);

   /* Angle linearization.
    * Linearization is performed according to the formula
    *
    * linearized = raw - k(1,1)*cos(1*raw) - k(1,2)*sin(1*raw)
    *                  - k(2,1)*cos(2*raw) - k(2,2)*sin(2*raw)
    *                  - ...
    *
    * The coefficients are stored in a vector in the order k(1,1), k(1,2),
    * k(2,1), k(2,2), ...
   */
   static void setLinearization(const std::vector<float>& coefficients);

   // Set the origin of the cooked angle scale. This must be set somewhere
   // within the range of raw values that will never be reached due to hardware
   // restrictions.
   static void setOrigin(const RawAngle origin);

   // Invert the sense of the cooked angles with respect to the raw angles
   // (cooked angles increase when raw angles decrease and vice versa).
   static void setInverted(const bool set);

   // Set safe slew limits.
   static void setSafeLimits(const CookedAngle min, const CookedAngle max);

   // Report safe slew limits.
   inline static CookedAngle getMinimum() { return minimumSafeAngle; };
   inline static CookedAngle getMaximum() { return maximumSafeAngle; };
   bool isSafe() const;

private:
   static degrees linearize(degrees val);

   static std::vector<float> linCoeffs;
   static RawAngle hardwareOrigin;
   static degrees offset;
   static bool inverted;
   static CookedAngle minimumSafeAngle;
   static CookedAngle maximumSafeAngle;
};


class UserAngle : public Angle<UserAngle>
{
public:
   explicit UserAngle(degrees value) : Angle(value) {};
   explicit UserAngle(const CookedAngle cooked);

   // Set the origin of the user scale, i.e., the point where the user scale
   // will read zero.
   static void setOrigin(const CookedAngle origin);

   // Check whether this angle is within the safe slew zone.
   bool isSafe() const;

private:
   static CookedAngle userOrigin;

   // allow CookedAngle to access userOrigin
   friend class CookedAngle;
};

#endif // ANGLES_H
