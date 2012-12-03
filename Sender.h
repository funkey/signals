#ifndef SIGNALS_SENDER_H__
#define SIGNALS_SENDER_H__

#include <vector>
#include "Slot.h"
#include "Receiver.h"

namespace signals {

class Sender {

public:

	typedef std::list<SlotBase*> slots_type;

	/**
	 * Register a signal slot with this sender.
	 */
	void registerSlot(SlotBase& slot) {

		_slots.push_back(&slot);
		_slots.sort(SlotComparator());
	}

	void connect(Receiver& receiver) {

		LOG_ALL(signalslog) << "sender trying to connect to receiver" << std::endl;

		// for every slot we provide
		for (slots_type::iterator slot = _slots.begin();
		     slot != _slots.end(); ++slot) {

			bool exclusiveFound = false;

			// find all transparent and the first (most specific) exclusive callback
			for (Receiver::callbacks_type::iterator callback = receiver.getCallbacks().begin();
			     callback != receiver.getCallbacks().end(); ++callback) {

				// if this is an exclusive callback and we found another
				// exclusive one already, continue
				if (!(*callback)->isTransparent() && exclusiveFound)
					continue;

				// if connection could be established
				if ((*callback)->tryToConnect(*(*slot))) {

					// we assigned the exclusive callback
					if (!(*callback)->isTransparent())
						exclusiveFound = true;
				}
			}
		}
	}

private:

	slots_type _slots;
};

} // namespace signals

#endif // SIGNALS_SENDER_H__

