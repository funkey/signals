#ifndef SIGNALS_SENDER_H__
#define SIGNALS_SENDER_H__

#include <vector>
#include "SlotComparator.h"

namespace signals {

// forward declaration
class SlotBase;
class Receiver;

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

			(*slot)->connect(receiver);
		}
	}

	void disconnect(Receiver& receiver) {

		LOG_ALL(signalslog) << "sender disconnecting from receiver" << std::endl;

		// for every slot we provide
		for (slots_type::iterator slot = _slots.begin();
		     slot != _slots.end(); ++slot) {

			(*slot)->disconnect(receiver);
		}
	}

private:

	slots_type _slots;
};

} // namespace signals

#endif // SIGNALS_SENDER_H__

