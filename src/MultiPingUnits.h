#ifndef _MULTIPING_UNITS_H_
#define _MULTIPING_UNITS_H_ 1

// set false to remove floating point setTemperature method if memory savings are needed
#define MULTIPING_UNITS_SET_FLOAT_TERMPERATURE 1

// define the following to use a fixed speed of sound; see struct Units::SoS_t below
//#define MULTIPING_UNITS_FIXED_SPEEDOFSOUND {330,0,0}

namespace MultiPing {



/*********************************************************************
 * Units and speed of sound related conversion class.
 * Pure static methods.
 * 
 * The speed of sound varies with the air temperature.
 * Set the current temperature using the static setTemperature 
 * method.  Subsequent conversions use the corresponding 
 * computed speed of sound for subsequent calculations.
 *********************************************************************/
 class Units {
   public:
    /**
     * Convert seconds to microseconds.
     * @param[in] s interval in seconds.
     * @return inverval in microseconds.
     */
    static inline unsigned long s2us(unsigned long s) {
        return s * 1000000ul;
    }

    /**
     * Convert milliseconds to microseconds.
     * @param[in] ms interval in milliseconds.
     * @return inverval in microseconds.
     */
    static inline unsigned long ms2us(unsigned long ms) { return ms * 1000ul; }


#ifndef MULTIPING_UNITS_FIXED_SPEEDOFSOUND
    /**
     * Set the air temperature for speed of sound dependent conversions.
     * @param[in] dC temperature in degrees C
     */
    static void setTemperature(int dC);

#if MULTIPING_UNITS_SET_FLOAT_TERMPERATURE
    /**
     * Set the air temperature for speed of sound dependent conversions.
     * @param[in] dC temperature in degrees C
     */
    static void setTemperature(float dC);
#endif 
#endif

    /**
     * Convert a one-way (half-echo) time in to distance.
     * 
     * This computation is speed-optmized using integer operations and
     * a table-lookup for the speed-of-sound.
     * 
     * @param[in] us one-way time-of-flight in microseconds.
     * @return distance in millimeters
     */
    static inline unsigned long us2mm(unsigned long us) {
        return (((unsigned long)Units::a.ms) * us) / 1000L +
               (((unsigned long)Units::a.mms) * (us / 1000L)) / 1000L;
    }

    /**
     * Convert a one-way (half-echo) time in to distance.
     * 
     * @param[in] us one-way time-of-flight in microseconds.
     * @return float distance in meters
     */
    static float us2m(unsigned long us);

    /**
     * Convert a one-way (half-echo) time in to distance.
     * 
     * @param[in] us one-way time-of-flight in microseconds.
     * @return float distance in centimeters
     */
    static float us2cm(unsigned long us);

    /**
     * Convert a one-way (half-echo) time in to distance.
     * 
     * @param[in] us one-way time-of-flight in microseconds.
     * @return float distance in feet
     */
    static float us2ft(unsigned long us);

    /**
     * Convert a one-way (half-echo) time in to distance.
     * 
     * @param[in] us one-way time-of-flight in microseconds.
     * @return float distance in inches
     */
    static float us2in(unsigned long us);

   protected:
    typedef struct {  // speed of sound table entry
        int ms;   // integer part m/s
        int mms;  // fractional part mm/s
        int dadT; // mm/s / dC evaluated at table midpoints [+2.5C]
    } SoS_t; 
#ifndef MULTIPING_UNITS_FIXED_SPEEDOFSOUND   
    static const int nSoS = 17;  // number of entries in speed of sound table 
    // indexed by T(C) / 5; T in range -30 to 50
    static const SoS_t speedOfSound[nSoS];
#endif    
    static SoS_t a;  // current speed of sound
};

};      // namespace MultiPing
#endif  // _MULTIPING_UNITS_H_