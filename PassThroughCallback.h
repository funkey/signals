#ifndef SIGNALS_PASS_TROUGH_CALLBACK_H__
#define SIGNALS_PASS_TROUGH_CALLBACK_H__

#include "CallbackBase.h"
#include "PassThroughCallbackBase.h"
#include "PassThroughSlotBase.h"

namespace signals {

template <typename SignalType = signals::Signal>
class PassThroughCallback : public PassThroughCallbackBase {

public:

	PassThroughCallback() {

		// pass through callbacks should always be connected to, even if more 
		// specific callbacks are registered in the same receiver
		setTransparent();
	}

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
	bool connect(SlotBase& slot) {

		// check if slot's signal should be passed through
		if (!accepts(slot.createSignal()))
			return false;

		// remember this slot for future connections on the other side
		addSlot(slot);

		// connect the new slot to each registered receiver on the other side
		typename PassThroughSlotBase::receivers_type::iterator receiver;
		for (receiver = getTarget().getReceivers().begin(); receiver != getTarget().getReceivers().end(); receiver++)
			slot.connect(*(*receiver));

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
	bool disconnect(SlotBase& slot) {

		removeSlot(slot);

		// disconnect this slot from each registered receiver on the other side
		typename PassThroughSlotBase::receivers_type::iterator receiver;
		for (receiver = getTarget().getReceivers().begin(); receiver != getTarget().getReceivers().end(); receiver++)
			slot.disconnect(*(*receiver));

		return true;
	}

	/**
	 * Set the other end of this pass-through callback.
	 */
	void forwardTo(PassThroughSlotBase& target) {

		setTarget(target);
		target.setSource(*this);
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
};

} // namespace signals

#endif // SIGNALS_PASS_TROUGH_CALLBACK_H__

