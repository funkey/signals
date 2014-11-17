#ifndef SIGNALS_PASS_THROUGH_SLOT_H__
#define SIGNALS_PASS_THROUGH_SLOT_H__

#include "SlotBase.h"

// forward declaration
template <typename SignalType>
class PassThroughCallback;

namespace signals {

template <typename SignalType>
class PassThroughSlot : public SlotBase {

	/**
	 * Create a reference signal of this slot.
	 */
	const Signal& createSignal() const {

		return Slot<SignalType>::referenceSignal;
	}

	/**
	 * Connect this slot to the given receiver. This will keep a pointer to the 
	 * receiver for future reference and connect every source at the other side 
	 * (a PassThroughCallback) with every currently connected receiver.
	 */
	bool connect(Receiver&) {

		return true;
	}

	/**
	 * Disconnect this slot from the given receiver and remove the pointer to 
	 * the receiver.
	 */
	bool disconnect(Receiver&) {

		return true;
	}

private:

	void setSource(PassThroughCallback<SignalType>& source) {

		_source = &source;
	}

	bool canSend(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	PassThroughCallback<SignalType>* _source;
};

} // namespace signals

#endif // SIGNALS_PASS_THROUGH_SLOT_H__

