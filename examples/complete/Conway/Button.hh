#ifndef BUTTON_HH
#define BUTTON_HH

#include <fastarduino/FastIO.hh>

//TODO Possibly use same header for both classes Buttons and Button
//TODO Some more refactoring is possible (AbstractButton<type> to use also with Buttons)
class AbstractButton
{
protected:
	AbstractButton():_latest_state(true), _pending_state{}, _changed{}, _count() {}

	bool _state(bool state, uint8_t debounce_count)
	{
		// Don't return state unless it remained the same during DEBOUNCE_COUNT calls
		bool changed = false;
		if (_count)
		{
			// We are in a debouncing phase, check if we have reached end of debounce time
			if (++_count == debounce_count)
			{
				if (state == _pending_state)
				{
					changed = true;
					_latest_state = state;
				}
				_count = 0;
			}
		}
		else if (state != _latest_state)
		{
			// State has changed for the first time, start debouncing period now
			_pending_state = state;
			_count = 1;
		}
		_changed = changed;
		// Note that we want state to return true when button is pushed (LOW), hence we negate state here
		return !_latest_state;
	}
	
	bool _unique_press(bool state, uint8_t debounce_count)
	{
		return _state(state, debounce_count) && changed();
	}
	
public:
	inline bool changed() INLINE
	{
		return _changed;
	}
	
private:
	bool _latest_state;
	bool _pending_state;
	bool _changed;
	uint8_t _count;
};

template<Board::DigitalPin DPIN, uint8_t DEBOUNCE_COUNT>
class Button: public AbstractButton
{
public:
	static constexpr const Board::Port PORT = FastPinType<DPIN>::PORT;
	static constexpr const uint8_t DDR_MASK = 0;
	static constexpr const uint8_t PORT_MASK = FastPinType<DPIN>::MASK;
	
	Button() {}

	inline void init() INLINE
	{
		_pin.set_mode(PinMode::INPUT_PULLUP);
	}

	inline bool state() INLINE
	{
		return _state(_pin.value(), DEBOUNCE_COUNT);
	}
	
	inline bool unique_press() INLINE
	{
		return _unique_press(_pin.value(), DEBOUNCE_COUNT);
	}

private:
	typename FastPinType<DPIN>::TYPE _pin;
};

#endif /* BUTTON_HH */