//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * Timer compilation example.
 * Shows how to use 2 CTC Timers (not RTT) to blink 1 LED for some period of time, then stop completely.
 * 
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <fastarduino/fast_io.h>
#include <fastarduino/timer.h>
#include <fastarduino/time.h>

constexpr const Board::Timer BLINK_TIMER = Board::Timer::TIMER0;
using BLINK_TIMER_TYPE = Timer<BLINK_TIMER>;
constexpr const uint32_t BLINK_PERIOD_US = 10000;
constexpr const BLINK_TIMER_TYPE::TIMER_PRESCALER BLINK_PRESCALER = BLINK_TIMER_TYPE::prescaler(BLINK_PERIOD_US);
static_assert(BLINK_TIMER_TYPE::is_adequate(BLINK_PRESCALER, BLINK_PERIOD_US), 
		"BLINK_TIMER_TYPE::is_adequate(BLINK_PRESCALER, BLINK_PERIOD_US)");
constexpr const BLINK_TIMER_TYPE::TIMER_TYPE BLINK_COUNTER = BLINK_TIMER_TYPE::counter(BLINK_PRESCALER, BLINK_PERIOD_US);

constexpr const Board::Timer SUSPEND_TIMER = Board::Timer::TIMER1;
using SUSPEND_TIMER_TYPE = Timer<SUSPEND_TIMER>;
constexpr const uint32_t SUSPEND_PERIOD_US = 4000000;
constexpr const SUSPEND_TIMER_TYPE::TIMER_PRESCALER SUSPEND_PRESCALER = SUSPEND_TIMER_TYPE::prescaler(SUSPEND_PERIOD_US);
static_assert(SUSPEND_TIMER_TYPE::is_adequate(SUSPEND_PRESCALER, SUSPEND_PERIOD_US), 
		"SUSPEND_TIMER_TYPE::is_adequate(SUSPEND_PRESCALER, SUSPEND_PERIOD_US)");
constexpr const SUSPEND_TIMER_TYPE::TIMER_TYPE SUSPEND_COUNTER = SUSPEND_TIMER_TYPE::counter(SUSPEND_PRESCALER, SUSPEND_PERIOD_US);

class BlinkHandler
{
public:
	BlinkHandler(): _led{PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.set();
		Time::delay_us(1000);
		_led.clear();
	}
	
private:
	FastPinType<Board::DigitalPin::LED>::TYPE _led;
};

class SuspendHandler
{
public:
	SuspendHandler(BLINK_TIMER_TYPE& blink_timer):_blink_timer{blink_timer} {}
	
	void on_timer()
	{
		if (_blink_timer.is_suspended())
			_blink_timer._resume();
		else
			_blink_timer._suspend();
	}
	
private:
	BLINK_TIMER_TYPE& _blink_timer;
};

// Define vectors we need in the example
REGISTER_TIMER_ISR_METHOD(0, BlinkHandler, &BlinkHandler::on_timer)
REGISTER_TIMER_ISR_METHOD(1, SuspendHandler, &SuspendHandler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	BlinkHandler blink_handler;
	BLINK_TIMER_TYPE blink_timer;
	SuspendHandler suspend_handler{blink_timer};
	SUSPEND_TIMER_TYPE suspend_timer;
	register_handler(blink_handler);
	register_handler(suspend_handler);
	blink_timer._begin(BLINK_PRESCALER, BLINK_COUNTER);
	suspend_timer._begin(SUSPEND_PRESCALER, SUSPEND_COUNTER);
	sei();
	
	while (true) ;
}