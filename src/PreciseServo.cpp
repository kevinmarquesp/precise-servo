#include "PreciseServo.h"

#ifdef PRECISE_SERVO_DEBUG_FLAG
/** HELPER FUNCTION - print a large number in the serial monitor */
void debug_print64(i64 num)
{
    char buff[24];
    utoa(num, buff, 10);
    Serial.println(String(buff));
}
#endif

/** HELPER FUNCTION - ajust the deg value and tells if the sleep is needed to be considered */
bool local_isRedundant(i8& min, i8& max, i8& deg, i8 sleep)
{
    deg = deg < min ? min : deg; // use the min value if it is lesser than that
    deg = deg > max ? max : deg; // use the max value if it is greater than that

    // it isn't different from a regular write() if the speed is 0
    return sleep < 1;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * _BaseServo METHOD DEFFINITIONS
 */

/** attach the object to the pin and set the min and max deggre values */
void _BaseServo::config(i8 pin, i8 min=0, i8 max=180)
{
    this->attach(pin);
    this->write(min); // ... also, it resets to the min value on config
    this->min = min;
    this->max = max;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * PreciseServo METHOD DEFFINITIONS
 */ 

/** delayed write - sleep x milliseconds each deggre movement to reach the deg position */
void PreciseServo::move(i8 deg, i8 sleep=0)
{
    if (local_isRedundant(this->min, this->max, deg, sleep))
        return this->write(deg); 

    i8 curr = this->read();

    // while curr != deg, it will approach the target position waiting the sleep time each deggre
    while (curr != deg)
    {
        deg > curr ? ++curr : --curr; // ... iterator

        this->write(curr);
        delay(curr == deg ? 0 : sleep); // the last sleep is irrelevant

        debug_log("PreciseServo-move/2", curr);
    }
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AdvancedServo METHOD DEFFINITIONS
 */

/** AdvancedServo - constructor to set the boolean values to false by default */
AdvancedServo::AdvancedServo(void)
{
    this->moveId = 0;
    this->moving = false;
    this->finished = false;
}

/** movement core - backbone of the movement validation */
AdvancedServo* AdvancedServo::move(bool cond, i8 deg, i8 sleep)
{
    // ...
    if (!cond)
        return this;

    // validade the values and finisht if is ok to do that
    if (local_isRedundant(this->min, this->max, deg, sleep))
    {
        this->write(deg); 
        _done();
    }

    // when it is ready to move but hasen't started yet
    if (!this->moving && !this->finished)
    {
        debug_log("AdvancedServo-move/3", "staring the movement process");

        _scheduler = millis();
        this->moving = true;
    }

    // there is no need to continue if the value is already setted, mark as done
    if (deg == this->read())
        _done();

    // when already is moving, and every thing is ok, just update the position
    else if (this->moving && !this->finished)
        _update(deg, sleep);

    return this;
}

/** shorthand to call the real move function, that has a condition to start */
AdvancedServo* AdvancedServo::move(i8 deg, i8 sleep)
{
    return this->move(this->is(0), deg, sleep);
}

/** instructions that mark this servo object as done */
void AdvancedServo::_done(void)
{
    debug_log("AdvancedServo-_done/0", "this servo is already in the target position");

    this->finished = true;
    ++this->moveId;
}

/** update loop - it will count the milliseconds and write the deg based on that */
void AdvancedServo::_update(i8 deg, i8 sleep)
{
    debug_log("AdvancedServo-_update/2", this->read());

    // count the millis() to check if it is able to make a single movement
    if (millis() - _scheduler >= sleep)
    {
        i8 curr = this->read();

        if (deg > curr)
            this->write(this->read() + 1);
        else
            this->write(this->read() - 1);
    }
}

/** user id validation - check if the current motor is the expected id by the user */
bool AdvancedServo::is(i8 id)
{
    return id == this->moveId;
}
