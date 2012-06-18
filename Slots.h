#ifndef SIGNALS_SLOTS_H__
#define SIGNALS_SLOTS_H__

#include <vector>

#include "Slot.h"

namespace signals {

class SlotsBase {

public:

	/**
	 * Create a new slot.
	 *
	 * @return The current number of slots.
	 */
	virtual unsigned int addSlot() = 0;

	/**
	 * Remove a slot.
	 */
	virtual void removeSlot(unsigned int i) = 0;

	/**
	 * Remove all slots.
	 */
	virtual void clear() = 0;

	virtual SlotBase& operator[](unsigned int i) = 0;
};

template <typename SignalType>
class Slots : public SlotsBase {

	// TODO: ensure that SignalType is default constructible

public:

	~Slots() {

		for (unsigned int i = 0; i < _slots.size(); i++)
			delete _slots[i];
	}

	unsigned int addSlot() {

		_slots.push_back(new Slot<SignalType>());

		return _slots.size() - 1;
	}

	void removeSlot(unsigned int i) {

		delete _slots[i];

		_slots.erase(_slots.begin() + i);
	}

	void clear() {

		for (int i = 0; i < _slots.size(); i++)
			delete _slots[i];

		_slots.clear();
	}

	unsigned int size() {

		return _slots.size();
	}

	Slot<SignalType>& operator[](unsigned int i) {

		return *_slots[i];
	}

	const Slot<SignalType>& operator[](unsigned int i) const {

		return *_slots[i];
	}

private:

	std::vector<Slot<SignalType>*> _slots;
};

} // namespace signals

#endif // SIGNALS_SLOTS_H__


