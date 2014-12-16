#ifndef SIGNALS_SLOT_H__
#define SIGNALS_SLOT_H__

#include <utility>
#include <boost/thread.hpp>

#include "Signal.h"
#include "SignalTraits.h"
#include "SlotBase.h"
#include "Receiver.h"
#include "CallbackInvoker.h"
#include "Logging.h"

namespace signals {

template <typename SignalType, typename CallbackInvokerType = CallbackInvoker<SignalType> >
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

		return SignalTraits<SignalType>::Reference;
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
	 *
	 * Precondition: The callback does accept our signal.
	 */
	bool addCallback(CallbackBase& callback) {

		if (isConnected(callback))
			return false;

		typename CallbackInvokerType::CallbackBaseType* p = dynamic_cast<typename CallbackInvokerType::CallbackBaseType*>(&callback);

		// not the type of callback we are interested in?
		if (!p)
			return false;

		addInvoker(CallbackInvokerType(*p));

		LOG_ALL(signalslog) << typeName(callback) << " connected to " << typeName(this) << std::endl;

		return true;
	}

	/**
	 * Disconnect a callback from this slot.
	 */
	bool removeCallback(CallbackBase& callback) {

		if (!isConnected(callback))
			return false;

		typename CallbackInvokerType::CallbackBaseType* p = dynamic_cast<typename CallbackInvokerType::CallbackBaseType*>(&callback);

		// not the type of callback we are interested in?
		if (!p)
			return false;

		removeInvoker(CallbackInvokerType(*p));

		LOG_ALL(signalslog) << typeName(callback) << " disconnected from " << typeName(this) << std::endl;

		return true;
	}

	/**
	 * Get the number of callbacks that are registered for this slot.
	 */
	size_t numTargets() const {

		return _invokers.size();
	}

private:

	typedef std::vector<CallbackInvokerType> CallbackInvokersType;

	bool canSend(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	template <typename CallbackType>
	bool isConnected(CallbackType& callback) {

		typename CallbackInvokerType::CallbackBaseType* p = dynamic_cast<typename CallbackInvokerType::CallbackBaseType*>(&callback);

		// not the type of callback we are interested in?
		if (!p)
			return false;

		return std::find(_invokers.begin(), _invokers.end(), CallbackInvokerType(*p)) != _invokers.end();
	}

	void addInvoker(CallbackInvokerType&& invoker) {

		_invokers.push_back(std::forward<CallbackInvokerType>(invoker));
	}

	void removeInvoker(const CallbackInvokerType& invoker) {

		auto i = std::find(_invokers.begin(), _invokers.end(), invoker);

		if (i != _invokers.end())
			_invokers.erase(i);
	}

	void send(SignalType& signal) {

		// for each callback invoker
		for (auto& invoker : _invokers)
			if (!invoker(signal))
				_staleInvokers.push_back(&invoker);

		if (!_staleInvokers.size() == 0)
			return;

		// for each stale invoker
		for (auto* invoker : _staleInvokers) {

			// remove it
			removeInvoker(*invoker);

			LOG_ALL(signalslog) << "removed stale invoker " << typeName(invoker) << std::endl;
		}

		// clear stale invokers
		_staleInvokers.clear();
	}

	// list of callback invokers
	std::vector<CallbackInvokerType> _invokers;

	// list of callback invokers that failed to lock and should be removed
	std::vector<CallbackInvokerType*> _staleInvokers;

	// mutex to prevent concurrent access to the invoker list
	boost::mutex _mutex;
};

} // namespace signals

#endif // SIGNALS_SLOT_H__

