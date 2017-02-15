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
 * Blocking EEPROM Read and Writes.
 * This program shows usage of FastArduino EEPROM API.
 * It interfaces with user through the UART console and allows:
 * - writing values to EEPROM
 * - reading values to EEPROM
 * 
 * Wiring: TODO
 * - on ATmega328P based boards (including Arduino UNO):
 * - on Arduino MEGA:
 * - on ATtinyX4 based boards:
 *   - D1: TX output connected to Serial-USB allowing traces display on a PC terminal
 */

#include <fastarduino/time.h>
#include <fastarduino/eeprom.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t INPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UART_ISR(0)
#elif defined (ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const uint8_t INPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UART_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const uint8_t INPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
constexpr const Board::DigitalPin TX = Board::DigitalPin::D1;
constexpr const Board::DigitalPin RX = Board::DigitalPin::D0;
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

using namespace eeprom;

int main()
{
	// Enable interrupts at startup time
	sei();
#if HARDWARE_UART
	UART<Board::USART::USART0> uart{input_buffer, output_buffer};
	uart.register_handler();
#else
	//TODO Handle PCI/INT enabling
	Soft::UART<RX, TX> uart{input_buffer, output_buffer};
#endif
	uart.begin(115200);

	FormattedOutput<OutputBuffer> out = uart.fout();
//	FormattedInput<InputBuffer> in = uart.fin();
	InputBuffer& in = uart.in();
	
	// Declare Analog input
	while (true)
	{
		char command;
		out << "Command (R/W): " << flush;
		command = ::get(in);
//		in >> command;
		if (command == 'W')
		{
			EEPROM::write(0, (uint8_t) 123);
			out << "\n1 byte written to EEPROM.\n";
		}
		else if (command == 'R')
		{
			uint8_t value;
			EEPROM::read(0, value);
			out << "\n1 byte read from EEPROM: " << value << '\n';
		}
	}
	return 0;
}