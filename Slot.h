#ifndef SIGNALS_SLOT_H__
#define SIGNALS_SLOT_H__

#include <boost/thread.hpp>

#include "CallbackInvoker.h"
#include "Signal.h"
#include "SlotBase.h"
#include "Receiver.h"
#include "Logging.h"

namespace signals {

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
	 * Connect to all compatible callbacks of the given reveiver.
	 */
	bool connect(Receiver& receiver) {

		bool exclusiveFound = false;

		// find all transparent and the first (most specific) exclusive callback
		for (Receiver::callbacks_type::iterator callback = receiver.getCallbacks().begin();
			 callback != receiver.getCallbacks().end(); ++callback) {

			// if this is an exclusive callback and we found another
			// exclusive one already, continue
			if (!(*callback)->isTransparent() && exclusiveFound)
				continue;

			// if connection could be established
			if ((*callback)->connect(*this)) {

				// we assigned the exclusive callback
				if (!(*callback)->isTransparent())
					exclusiveFound = true;
			}
		}

		return true;
	}

	bool disconnect(Receiver& receiver) {

		// for all callbacks of receiver
		for (Receiver::callbacks_type::iterator callback = receiver.getCallbacks().begin();
			 callback != receiver.getCallbacks().end(); ++callback) {

			// disconnect
			(*callback)->disconnect(*this);
		}

		return true;
	}

	/**
	 * Add a callback to this slot.
	 */
	template <typename CallbackType>
	bool addCallback(CallbackType& callback) {

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

	/**
	 * Get the number of callbacks that are registered for this slot.
	 */
	size_t numTargets() const {

		return _invokers.size();
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

				(*invoker)(signal);
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

