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

			LOG_ALL(signalslog) << "offering slot " << typeName(*(*slot)) << std::endl;

			// find the first (most specific) callback
			for (Receiver::callbacks_type::iterator callback = receiver.getCallbacks().begin();
			     callback != receiver.getCallbacks().end(); ++callback) {

				LOG_ALL(signalslog) << "to callback " << typeName(*(*callback)) << std::endl;

				if ((*callback)->tryToConnect(*(*slot)))
					break;
			}
		}
	}

private:

	slots_type _slots;
};

} // namespace signals

#endif // SIGNALS_SENDER_H__

