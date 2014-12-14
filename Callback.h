#ifndef SIGNALS_CALLBACK_H__
#define SIGNALS_CALLBACK_H__

#include <boost/function.hpp>

#include "CallbackBase.h"
#include "CallbackInvocation.h"
#include "CallbackInvoker.h"
#include "CallbackTracking.h"
#include "CastingPolicy.h"
#include "SignalTraits.h"
#include "Slot.h"

namespace signals {

/**
 * Callback for a specific signal type. A tracking policy can be specified as a 
 * template parameter. This can be used to track the lifetime of an object and 
 * invoke the callback only if the tracked object is still alive. Currently 
 * available are NoTracking (default), WeakTracking (track an arbitrary object 
 * via a weak pointer, invoke callback only if weak pointer can be locked), and 
 * SharedTracking (keep an arbitrary object alive via a shared pointer as long 
 * as any invoker of the callback is used).
 */
template <
	typename SignalType,
	typename TrackingPolicy = NoTracking>
class Callback :
		public CallbackBase,
		public TrackingPolicy,
		public boost::noncopyable /* prevents references to _callback to get invalidated */ {

public:

	/**
	 * Create a new callback.
	 *
	 * @param callback
	 *              Any expression that can be cast into a 
	 *              std::function<void(SignalType&)>.
	 *
	 * @param invocation
	 *              Optional invocation type. Influences which callbacks are  
	 *              connected to signals, if several callbacks are compatible.  
	 *              See CallbackInvocation.
	 */
	Callback(std::function<void(SignalType&)> callback, CallbackInvocation invocation = Exclusive) :
		CallbackBase([callback](Signal& signal){ callback(static_cast<SignalType&>(signal)); }) {

		if (invocation == Transparent)
			setTransparent();
	}

	/**
	 * Try to connect this callback to the given slot.
	 *
	 * @return
	 *         true, if the callback and slot are type compatible and have been 
	 *         connected.
	 */
	bool connect(SlotBase& slot) {

		const Signal& reference = slot.createSignal();

		if (!accepts(reference))
			return false;

		slot.addCallback(*this);

		return true;
	}

	/**
	 * Disconnect this callback from the given slot.
	 *
	 * @return
	 *         true, if the callback and slot are type compatible and have been 
	 *         disconnected.
	 */
	bool disconnect(SlotBase& slot) {

		const Signal& reference = slot.createSignal();

		if (!accepts(reference))
			return false;

		slot.removeCallback(*this);

		return true;
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

		return SignalTraits<SignalType>::Reference;
	}
};

} // namespace signals

#endif // SIGNALS_CALLBACK_H__

