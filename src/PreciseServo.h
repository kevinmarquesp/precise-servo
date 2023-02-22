#ifndef PRECISESERVO_H
#define PRECISESERVO_H

#include <Arduino.h>
#include <Servo.h>

#define u8  uint8_t
#define u64 uint64_t

// DEVELOPING TIP - (un)comment the line above to activate/deactvate the debbuger printer
// #define PRECISE_SERVO_DEBUG

#ifdef PRECISE_SERVO_DEBUG
// HELPER FUNCTION - print a large number in the serial monitor
void debug_print64(u64 num);

// list of commands to show a log line in the serial port
#define debug_log(flag,msg)   \
    Serial.print(flag);       \
    Serial.print(":");        \
    Serial.print(__LINE__);   \
    Serial.print(" :: ");     \
    Serial.println(msg)

// list of commands to do the same thing, but to show a large number
#define debug_log64(flag,num) \
    Serial.print(flag);       \
    Serial.print(":");        \
    Serial.print(__LINE__);   \
    Serial.print(" :: ");     \
    debug_print64(num)
#else
#define debug_log(flag, msg)
#define debug_log64(flag, msg)
#endif

/** base attributes/methods that every servo class has in this library */
class _BaseServo: public Servo
{
  public:
    void config(u8 pin, u8 min=0, u8 max=180);
    u8 min, max; 
};

/** delayed servo - class to handle the servo speed with delay() function */
class PreciseServo: public _BaseServo
{
  public:
    void move(u8 deg, u8 sleep=0);
};

/** millis servo - same as PreciseServo but with millis() instead of delay() */
class AdvancedServo: public _BaseServo
{
  public:
    AdvancedServo(void);

    AdvancedServo* move(bool cond, u8 deg, u8 sleep);
    AdvancedServo* move(u8 deg, u8 sleep);

    bool is(u8 id);
    bool isDone(void);
    bool isDone(u8 id);

    void lock(void);
    void unlock(void);
    void reset(void);
    void whenDone(void fn(void));

  private:
    bool _isLocked;
    bool _isMoving;
    bool _isDone;
    u8   _movementId;
    u64  _scheduler;

    void _update(u8 deg, u8 sleep);
    void _markAsDone(void);
};

/** helper functions to work with multiple servos in a single array */
namespace ArrayHelpers
{
    bool isAllDone(AdvancedServo* servos, u8 size);
    bool isAll(AdvancedServo* servos, u8 size, u8 id);
    void resetAll(AdvancedServo* servos, u8 size);
}

#endif
