#ifndef SIGNALS_SLOT_H__
#define SIGNALS_SLOT_H__

#include <typeinfo>

#include <boost/signals2/signal.hpp>

#include <util/typename.h>
#include "Signal.h"
#include "Logging.h"

namespace signals {

class SlotBase {

public:

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

struct SlotComparator {

	bool operator()(const SlotBase* a, const SlotBase* b) const {

		return *a < *b;
	}
};

template <typename SignalType>
class Slot : public SlotBase {

	// TODO: ensure that SignalType is default constructible

public:

	/**
	 * Send a default-constructed signal of type SignalType.
	 */
	void operator()() {

		SignalType signal;

		LOG_ALL(signalslog) << typeName(this) << " sending signal " << typeName(signal) << std::endl;

		_slot(signal);
	}

	/**
	 * Send a given signal of type SignalType.
	 *
	 * @param signal The signal to send.
	 */
	void operator()(SignalType& signal) {

		LOG_ALL(signalslog) << typeName(this) << " sending signal " << typeName(signal) << std::endl;

		_slot(signal);
	}

	const Signal& createSignal() const {

		return referenceSignal;
	}

	void connect(boost::function<void(SignalType&)> callback) {

		LOG_ALL(signalslog) << typeName(this) << " connecting to " << typeName(callback) << std::endl;

		// If we want tracking of the callback holder, this is the point to add
		// it. That means we need a shared_ptr to the holder. The holder should
		// be a member of the Callback.
		_slot.connect(callback);

		LOG_ALL(signalslog) << typeName(this) << " connected to " << typeName(callback) << std::endl;
	}

	static SignalType referenceSignal;

protected:

	bool canSend(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

private:

	boost::signals2::signal<void(SignalType&)> _slot;
};

// TODO: I'm not sure if the linker will always find that -- check
// parashift.com, there was an FAQ for that (static members in template class)

// If you got an error here, that means most likely that you have a signal that
// does not provide a default constructor.
template<typename SignalType> SignalType Slot<SignalType>::referenceSignal;

} // namespace signals

#endif // SIGNALS_SLOT_H__

