#ifndef SIGNALS_SLOT_BASE_H__
#define SIGNALS_SLOT_BASE_H__

namespace signals {

// forward declarations
class Receiver;
class Signal;

class SlotBase {

public:

	virtual ~SlotBase() {}

	/**
	 * Connect this slot to a receiver.
	 */
	virtual bool connect(Receiver& receiver) = 0;

	/**
	 * Disconnect this slot from a receiver.
	 */
	virtual bool disconnect(Receiver& receiver) = 0;

	/**
	 * Comparison operator that sorts slots according to their specificity:
	 *
	 * Slot<Derived> < Slot<Base>
	 */
	bool operator<(const SlotBase& other) const {

		// Return true, if the other slot can send our signals as well. This
		// means that our signal type ≤ other signal type, i.e., we are more
		// specific.
		return other.canSend(createSignal());
	}

	/**
	 * Create a reference signal for run-time type inference. This reference is
	 * used to find compatible pairs of slots and callbacks.
	 */
	virtual const Signal& createSignal() const = 0;

protected:

	/**
	 * Return true, if the passed signal can be cast to the signal type this
	 * slot provides. This is to determine whether a slot can send a signal of
	 * the given type.
	 */
	virtual bool canSend(const Signal& signal) const = 0;
};

} // namespace signals

#endif // SIGNALS_SLOT_H__


