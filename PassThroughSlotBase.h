#ifndef SIGNALS_PASS_THROUGH_SLOT_BASE_H__
#define SIGNALS_PASS_THROUGH_SLOT_BASE_H__

#include <set>
#include "Receiver.h"
#include "SlotBase.h"

namespace signals {

// forward declaration
class PassThroughCallbackBase;

class PassThroughSlotBase : public SlotBase {

public:

	PassThroughSlotBase() :
		_source(0) {}

	typedef std::set<Receiver*> receivers_type;

	/**
	 * Get all recievers that are registered to this receiver. This set is needed  
	 * by the PassThroughCallback at the other side to establish connections.
	 */
	std::set<Receiver*>& getReceivers() {

		return _receivers;
	}

	/**
	 * Set the other side of this tunnel.
	 */
	void setSource(PassThroughCallbackBase& source) {

		_source = &source;
	}

protected:

	/**
	 * Get the other side of this tunnel.
	 */
	PassThroughCallbackBase& getSource() { return *_source; }

	/**
	 * Add a receiver to this PassThroughSlot for future connections.
	 */
	void addReceiver(Receiver& receiver) {

		_receivers.insert(&receiver);
	}

	/**
	 * Remove a receiver from this PassThroughSlot.
	 */
	bool removeReceiver(Receiver& receiver) {

		return _receivers.erase(&receiver) == 1;
	}

private:

	receivers_type _receivers;

	// the other end of this pass-through tunnel
	PassThroughCallbackBase* _source;
};

} // namespace signals

#endif // SIGNALS_PASS_THROUGH_SLOT_BASE_H__

