#ifndef SIGNALS_SLOT_H__
#define SIGNALS_SLOT_H__

#include <typeinfo>

#include <boost/thread.hpp>

#include <util/exceptions.h>
#include <util/typename.h>
#include "CallbackInvoker.h"
#include "Signal.h"
#include "Logging.h"

namespace signals {

class SlotBase {

public:

	virtual ~SlotBase() {}

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

public:

	virtual ~Slot() {}

	/**
	 * Send a default-constructed signal of type SignalType.
	 */
	void operator()() {

		SignalType signal;

		LOG_ALL(signalslog) << typeName(this) << " sending signal " << typeName(signal) << std::endl;

		send(signal);
	}

	/**
	 * Send a given signal of type SignalType.
	 *
	 * @param signal The signal to send.
	 */
	void operator()(SignalType& signal) {

		LOG_ALL(signalslog) << typeName(this) << " sending signal " << typeName(signal) << std::endl;

		send(signal);
	}

	/**
	 * Create a reference signal of this slot.
	 */
	const Signal& createSignal() const {

		return referenceSignal;
	}

	/**
	 * Connect a callback to this slot.
	 */
	template <typename CallbackType>
	bool connect(CallbackType& callback) {

		if (isConnected(callback))
			return false;

		addInvoker(callback.getInvoker());

		LOG_ALL(signalslog) << typeName(callback) << " connected to " << typeName(this) << std::endl;

		return true;
	}

	/**
	 * Disconnect a callback from this slot.
	 */
	template <typename CallbackType>
	bool disconnect(CallbackType& callback) {

		if (!isConnected(callback))
			return false;

		removeInvoker(callback.getInvoker());

		LOG_ALL(signalslog) << typeName(callback) << " disconnected from " << typeName(this) << std::endl;

		return true;
	}

	// a reference signal for type comparison
	static SignalType referenceSignal;

private:

	typedef CallbackInvoker<SignalType>      CallbackInvokerType;
	typedef std::vector<CallbackInvokerType> CallbackInvokersType;

	bool canSend(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	template <typename CallbackType>
	bool isConnected(CallbackType& callback) {

		return std::find(_invokers.begin(), _invokers.end(), callback.getInvoker()) != _invokers.end();
	}

	void addInvoker(const CallbackInvokerType& invoker) {

		_invokers.push_back(invoker);
	}

	void removeInvoker(const CallbackInvokerType& invoker) {

		typename CallbackInvokersType::iterator i = std::find(_invokers.begin(), _invokers.end(), invoker);

		if (i != _invokers.end())
			_invokers.erase(i);
	}

	void send(SignalType& signal) {

		bool foundStaleInvokers = false;

		// for each callback invoker
		for (typename CallbackInvokersType::iterator invoker = _invokers.begin(); invoker != _invokers.end(); invoker++) {

			LOG_ALL(signalslog) << "processing callback invoker " << typeName(*invoker) << std::endl;

			// try to get the callback lock
			typename CallbackInvokerType::Lock lock = invoker->lock();

			// if failed, add invoker to list of stale invokers
			if (!lock) {

				LOG_ALL(signalslog) << "callback invoker " << typeName(*invoker) << " got stale" << std::endl;

				_staleInvokers.push_back(*invoker);
				foundStaleInvokers = true;

				continue;

			// otherwise, call
			} else {

				try {

					(*invoker)(signal);

				} catch (boost::exception& e) {

					UTIL_THROW_EXCEPTION(SignalsError, typeName(*this) << " function call did not succeed");
				}
			}
		}

		if (!foundStaleInvokers)
			return;

		// for each stale invoker
		for (typename CallbackInvokersType::iterator invoker = _staleInvokers.begin(); invoker != _staleInvokers.end(); invoker++) {

			// remove it
			removeInvoker(*invoker);

			LOG_ALL(signalslog) << "removed stale invoker " << typeName(*invoker) << std::endl;
		}

		// clear stale invokers
		_staleInvokers.clear();
	}

	// list of callback invokers
	CallbackInvokersType _invokers;

	// list of callback invokers that failed to lock and should be removed
	CallbackInvokersType _staleInvokers;

	// mutex to prevent concurrent access to the invoker list
	boost::mutex _mutex;
};

// If you got an error here, that means most likely that you have a signal that
// does not provide a default constructor.
template<typename SignalType> SignalType Slot<SignalType>::referenceSignal;

} // namespace signals

#endif // SIGNALS_SLOT_H__

