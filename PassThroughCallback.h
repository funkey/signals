#ifndef SIGNALS_PASS_TROUGH_CALLBACK_H__
#define SIGNALS_PASS_TROUGH_CALLBACK_H__

#include "CallbackBase.h"
#include "PassThroughSlot.h"

namespace signals {

template <typename SignalType>
class PassThroughCallback : public CallbackBase {

public:

	/**
	 * Connect this callback to the given slot. This function checks whether the 
	 * given slot type can be cast into the one we shall pass through. If so, it 
	 * connects the slot with all receivers that have been connected to our 
	 * other side (a PassThroughSlot) and keeps a pointer to slot for future 
	 * connections on the other side.
	 *
	 * @return
	 *         true, if the callback and slot are type compatible.
	 */
	bool connect(SlotBase&) {

		// use accept()

		return true;
	}

	/**
	 * Disconnects the slot from all receivers on the other side and removes the 
	 * stored pointer to it.
	 *
	 * @return
	 *         true, if the callback and slot are type compatible and have been 
	 *         disconnected.
	 */
	bool disconnect(SlotBase&) {

		return true;
	}

	/**
	 * Set the other end of this pass-through callback.
	 */
	void forwardTo(PassThroughSlot<SignalType>& target) {

		_target = &target;
	}

private:

	/**
	 * Return true if this callback can accept the provided signal, i.e., if the 
	 * signal's type is equal to or a superclass of SignalType.
	 */
	bool accepts(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	const Signal& createSignal() const {

		return Slot<SignalType>::referenceSignal;
	}

	// the other end of this pass-through tunnel
	PassThroughSlot<SignalType>* _target;
};

} // namespace signals

#endif // SIGNALS_PASS_TROUGH_CALLBACK_H__

