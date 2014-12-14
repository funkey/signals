#ifndef SIGNALS_PASS_THROUGH_SLOT_H__
#define SIGNALS_PASS_THROUGH_SLOT_H__

#include "SignalTraits.h"
#include "PassThroughSlotBase.h"
#include "PassThroughCallbackBase.h"

namespace signals {

template <typename SignalType = signals::Signal>
class PassThroughSlot : public PassThroughSlotBase {

public:

	/**
	 * Create a reference signal of this slot.
	 */
	const Signal& createSignal() const {

		return SignalTraits<SignalType>::Reference;
	}

	/**
	 * Connect this slot to the given receiver. This will keep a pointer to the 
	 * receiver for future reference and connect every source at the other side 
	 * (a PassThroughCallback) with every currently connected receiver.
	 */
	bool connect(Receiver& receiver) {

		// remember this receiver
		addReceiver(receiver);

		// connect all registered slots at the other side to the new receiver
		typename PassThroughCallbackBase::slots_type::iterator slot;
		for (slot = getSource().getSlots().begin(); slot != getSource().getSlots().end(); slot++)
			(*slot)->connect(receiver);

		return true;
	}

	/**
	 * Disconnect this slot from the given receiver and remove the pointer to 
	 * the receiver.
	 */
	bool disconnect(Receiver& receiver) {

		removeReceiver(receiver);

		// disconnect all registered slots at the other side from this receiver
		typename PassThroughCallbackBase::slots_type::iterator slot;
		for (slot = getSource().getSlots().begin(); slot != getSource().getSlots().end(); slot++)
			(*slot)->disconnect(receiver);

		return true;
	}

private:

	void setSource(PassThroughCallbackBase& source) {

		_source = &source;
	}

	bool canSend(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	// all receivers that are connected to this slot
	std::set<Receiver*> _receivers;
};

} // namespace signals

#endif // SIGNALS_PASS_THROUGH_SLOT_H__

