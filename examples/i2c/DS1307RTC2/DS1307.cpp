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

#include <fastarduino/time.h>
#include <fastarduino/i2c.h>
#include <fastarduino/devices/ds1307.h>

#if defined(ARDUINO_UNO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#else
#error "Current target is not yet supported!"
#endif

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<UART> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

const uint32_t I2C_FREQUENCY = 100000;

using devices::rtc::DS1307;
using devices::rtc::tm;
using devices::rtc::SquareWaveFrequency;
using streams::flush;

void trace_status(uint8_t expected_status, uint8_t actual_status)
{
	out << F("status expected = ") << expected_status << F(", actual = ") << actual_status << '\n' << flush;
}

void trace_time(const tm& time)
{
	out	<< F("RTC: [") 
		<< time.tm_wday << ']'
		<< time.tm_mday << '.'
		<< time.tm_mon << '.'
		<< time.tm_year << ' '
		<< time.tm_hour << ':'
		<< time.tm_min << ':'
		<< time.tm_sec << '\n'
		<< flush;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	uart.register_handler();
	uart.begin(115200);
	out.width(2);
	out.base(streams::FormatBase::Base::hex);
	out << F("Start\n") << flush;
	
	// Start TWI interface
	//====================
	i2c::I2CManager manager{trace_status};
	manager.begin();
	out << F("I2C interface started\n") << flush;
	out << F("status #1 ") << manager.error() << '\n' << flush;
	time::delay_ms(1000);
	
	DS1307 rtc{manager};
	
	// read RAM content and print out
	uint8_t data[DS1307::ram_size()];
	rtc.get_ram(0, data, sizeof data);
	out << F("status #2 ") << manager.error() << '\n' << flush;
	out << F("RAM content\n");
	for (uint8_t i = 0; i < sizeof(data); ++i)
	{
		if (!(i % 8)) out << '\n' << flush;
		out << data[i] << ' ';
	}
	out << '\n' << flush;
	
	tm time1;
	time1.tm_hour = time1.tm_min = time1.tm_sec = 0;
	time1.tm_mday = 11;
	time1.tm_mon = 6;
	time1.tm_year = 17;
	//TODO check if we can avoid this init (who knows the weekday for a given date?)
	time1.tm_wday = 1;
	
	// Initialize clock date
	//=======================
	rtc.setDateTime(time1);
	out << F("status #3 ") << manager.error() << '\n' << flush;

	time::delay_ms(2000);
	
	// Read clock
	//============
	tm time2;
	rtc.getDateTime(time2);
	out << F("status #4 ") << manager.error() << '\n' << flush;
	trace_time(time2);
	
	// Enable output clock
	//====================
	rtc.enable_output(SquareWaveFrequency::FREQ_1HZ);
	out << F("status #5 ") << manager.error() << '\n' << flush;
	
	time::delay_ms(10000);
	
	rtc.disable_output(false);
	out << F("status #6 ") << manager.error() << '\n' << flush;

	// write RAM content
	for (uint8_t i = 0; i < sizeof(data); ++i)
		data[i] = i;
	rtc.set_ram(0, data, sizeof data);
	out << F("status #7 ") << manager.error() << '\n' << flush;
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End\n") << flush;
}
