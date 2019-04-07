//   Copyright 2016-2018 Jean-Francois Poilpret
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

#ifndef PULSE_TIMER_HH
#define PULSE_TIMER_HH

#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include <stddef.h>
#include "interrupts.h"
#include "utilities.h"
#include "timer.h"
#include "gpio.h"

//TODO Add documentation!

//TODO try to remove internal REGISTER macros and replace directly in actual API REGISTER macros
#define REGISTER_PULSE_TIMER_OVF2_ISR_(TIMER_NUM, PRESCALER, PIN_A, COM_A, PIN_B, COM_B)	\
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))													\
	{																						\
		static constexpr board::Timer NTIMER = (board::Timer) TIMER_NUM;					\
		timer::isr_handler_pulse_timer_overflow<											\
			NTIMER, PRESCALER, PIN_A, COM_A, PIN_B, COM_B>();								\
	}

#define REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN, COM_NUM)				\
	ISR(CAT3(TIMER, TIMER_NUM, _OVF_vect))												\
	{																					\
		static constexpr board::Timer NTIMER = (board::Timer) TIMER_NUM;				\
		timer::isr_handler_pulse_timer_overflow<NTIMER, PRESCALER, PIN, COM_NUM>();		\
	}

#define REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, COM_NUM, COMP, PIN)		\
	ISR(CAT3(TIMER, TIMER_NUM, COMP))										\
	{																		\
		timer::isr_handler_pulse_timer_compare<TIMER_NUM, COM_NUM, PIN>();	\
	}

// Macros to register ISR for PWM on PulseTimer8
//==============================================
#define REGISTER_PULSE_TIMER8_AB_ISR(TIMER_NUM, PRESCALER, PIN_A, PIN_B)     \
	REGISTER_PULSE_TIMER_OVF2_ISR_(TIMER_NUM, PRESCALER, PIN_A, 0, PIN_B, 1) \
	REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A)         \
	REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B)

#define REGISTER_PULSE_TIMER8_A_ISR(TIMER_NUM, PRESCALER, PIN_A)     \
	REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_A, 0)   \
	REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 0, _COMPA_vect, PIN_A) \
	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPB_vect))

#define REGISTER_PULSE_TIMER8_B_ISR(TIMER_NUM, PRESCALER, PIN_B)     \
	REGISTER_PULSE_TIMER_OVF1_ISR_(TIMER_NUM, PRESCALER, PIN_B, 1)   \
	REGISTER_PULSE_TIMER_COMP_ISR_(TIMER_NUM, 1, _COMPB_vect, PIN_B) \
	EMPTY_INTERRUPT(CAT3(TIMER, TIMER_NUM, _COMPA_vect))

namespace timer
{
	// Timer specialized in emitting pulses with accurate width, according to a slow frequency; this is typically
	// useful for controlling servos, which need a pulse with a width range from ~1000us to ~2000us, send every
	// 20ms, ie with a 50Hz frequency.
	// This implementation ensures a good pulse width precision for 16-bits timer.
	template<board::Timer NTIMER_, typename Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer16 : public Timer<NTIMER_>
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using PARENT = Timer<NTIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(TRAIT::IS_16BITS, "TIMER must be a 16 bits timer");

	public:
		using CALCULATOR = Calculator<NTIMER>;
		using TPRESCALER = typename CALCULATOR::PRESCALER;
		static constexpr const TPRESCALER PRESCALER = PRESCALER_;

		PulseTimer16(uint16_t pulse_frequency) : Timer<NTIMER>{TCCRA_MASK(), TCCRB_MASK()}
		{
			TRAIT::ICR = CALCULATOR::PWM_ICR_counter(PRESCALER, pulse_frequency);
		}

	private:
		static constexpr uint8_t TCCRA_MASK()
		{
			// If 16 bits, use ICR1 FastPWM
			return TRAIT::F_PWM_ICR_TCCRA;
		}
		static constexpr uint8_t TCCRB_MASK()
		{
			// If 16 bits, use ICR1 FastPWM and prescaler forced to best fit all pulse frequency
			return TRAIT::F_PWM_ICR_TCCRB | TRAIT::TCCRB_prescaler(PRESCALER);
		}
	};

	// Timer specialized in emitting pulses with accurate width, according to a slow frequency; this is typically
	// useful for controlling servos, which need a pulse with a width range from ~1000us to ~2000us, send every
	// 20ms, ie with a 50Hz frequency.
	// This implementation ensures a good pulse width precision for 8-bits timers.
	template<board::Timer NTIMER_, typename Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer8 : public Timer<NTIMER_>
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using PARENT = Timer<NTIMER>;
		using TRAIT = typename PARENT::TRAIT;
		static_assert(!TRAIT::IS_16BITS, "TIMER must be an 8 bits timer");

	public:
		using CALCULATOR = Calculator<NTIMER>;
		using TPRESCALER = typename CALCULATOR::PRESCALER;
		static constexpr const TPRESCALER PRESCALER = PRESCALER_;

		PulseTimer8(uint16_t pulse_frequency)
			: Timer<NTIMER>{TCCRA_MASK(), TCCRB_MASK(), TIMSK_INT_MASK()}, MAX{OVERFLOW_COUNTER(pulse_frequency)}
		{
			// If 8 bits timer, then we need ISR on Overflow and Compare A/B
			interrupt::register_handler(*this);
		}

	private:
		bool overflow()
		{
			if (++count_ == MAX) count_ = 0;
			return !count_;
		}

		static constexpr uint8_t TCCRA_MASK()
		{
			// If 8 bits, use CTC/TOV ISR
			return 0;
		}
		static constexpr uint8_t TCCRB_MASK()
		{
			// If 8 bits, use CTC/TOV ISR with prescaler forced best fit max pulse width
			return TRAIT::TCCRB_prescaler(PRESCALER);
		}
		static constexpr uint8_t TIMSK_INT_MASK()
		{
			return TRAIT::TIMSK_INT_MASK(uint8_t(TimerInterrupt::OVERFLOW | TimerInterrupt::OUTPUT_COMPARE_A |
											 TimerInterrupt::OUTPUT_COMPARE_B));
		}
		static constexpr uint8_t OVERFLOW_COUNTER(uint16_t pulse_frequency)
		{
			return F_CPU / 256UL / _BV(uint8_t(PRESCALER)) / pulse_frequency;
		}

	private:
		const uint8_t MAX;
		uint8_t count_;

		template<board::Timer T_, typename timer::Calculator<T_>::PRESCALER, board::DigitalPin, uint8_t>
		friend void isr_handler_pulse_timer_overflow();
		template<board::Timer T_, typename timer::Calculator<T_>::PRESCALER, 
			board::DigitalPin, uint8_t, board::DigitalPin, uint8_t>
		friend void isr_handler_pulse_timer_overflow();
	};

	// Unified API for PulseTimer whatever the timer bits size (no need to use PulseTimer8 or PulseTimer16)
	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER,
			 typename T = typename board_traits::Timer_trait<NTIMER_>::TYPE>
	class PulseTimer : public timer::Timer<NTIMER_>
	{
	public:
		static constexpr const board::Timer NTIMER = NTIMER_;

		PulseTimer(UNUSED uint16_t pulse_frequency) : timer::Timer<NTIMER>{0, 0}
		{
		}
		inline void begin()
		{
		}
		inline void begin_()
		{
		}
	};

	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer<NTIMER_, PRESCALER_, uint8_t> : public timer::PulseTimer8<NTIMER_, PRESCALER_>
	{
	public:
		PulseTimer(uint16_t pulse_frequency) : timer::PulseTimer8<NTIMER_, PRESCALER_>{pulse_frequency}
		{
		}
	};

	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_>
	class PulseTimer<NTIMER_, PRESCALER_, uint16_t> : public timer::PulseTimer16<NTIMER_, PRESCALER_>
	{
	public:
		PulseTimer(uint16_t pulse_frequency) : timer::PulseTimer16<NTIMER_, PRESCALER_>{pulse_frequency}
		{
		}
	};

	/// @cond notdocumented

	// All PCI-related methods called by pre-defined ISR are defined here
	//====================================================================
	template<board::Timer NTIMER_, board::DigitalPin PIN_, uint8_t COM_NUM_>
	void isr_handler_pulse_timer_check()
	{
		using TT = board_traits::Timer_trait<NTIMER_>;
		static_assert(!TT::IS_16BITS, "TIMER_NUM must be an 8 bits Timer");
		using PINT = board_traits::Timer_COM_trait<NTIMER_, COM_NUM_>;
		static_assert(PIN_ == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");
	}

	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_, 
		board::DigitalPin PIN_, uint8_t COM_NUM_>
	void isr_handler_pulse_timer_overflow()
	{
		isr_handler_pulse_timer_check<NTIMER_, PIN_, COM_NUM_>();
		using PT = timer::PulseTimer8<NTIMER_, PRESCALER_>;
		if (interrupt::HandlerHolder<PT>::handler()->overflow())
			gpio::FastPinType<PIN_>::set();
	}

	template<board::Timer NTIMER_, typename timer::Calculator<NTIMER_>::PRESCALER PRESCALER_, 
		board::DigitalPin PINA_, uint8_t COMA_NUM_, board::DigitalPin PINB_, uint8_t COMB_NUM_>
	void isr_handler_pulse_timer_overflow()
	{
		isr_handler_pulse_timer_check<NTIMER_, PINA_, COMA_NUM_>();
		isr_handler_pulse_timer_check<NTIMER_, PINB_, COMB_NUM_>();
		using PT = timer::PulseTimer8<NTIMER_, PRESCALER_>;
		if (interrupt::HandlerHolder<PT>::handler()->overflow())
		{
			using PINTA = board_traits::Timer_COM_trait<NTIMER_, COMA_NUM_>;
			using PINTB = board_traits::Timer_COM_trait<NTIMER_, COMB_NUM_>;
			if (PINTA::OCR != 0) gpio::FastPinType<PINA_>::set();
			if (PINTB::OCR != 0) gpio::FastPinType<PINB_>::set();
		}
	}

	template<uint8_t TIMER_NUM_, uint8_t COM_NUM_, board::DigitalPin PIN_>
	void isr_handler_pulse_timer_compare()
	{
		static constexpr board::Timer NTIMER = (board::Timer) TIMER_NUM_;
		using PINT = board_traits::Timer_COM_trait<NTIMER, COM_NUM_>;
		static_assert(PIN_ == PINT::PIN_OCR, "PIN must be connected to TIMER_NUM OCxA/OCxB");
		gpio::FastPinType<PIN_>::clear();
	}

	/// @endcond
}

#endif /* PULSE_TIMER_HH */
