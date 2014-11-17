#ifndef SIGNALS_CALLBACK_H__
#define SIGNALS_CALLBACK_H__

#include <boost/function.hpp>

#include "CallbackBase.h"
#include "CallbackInvocation.h"
#include "CallbackInvoker.h"
#include "CallbackTracking.h"
#include "CastingPolicy.h"
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
	typename TrackingPolicy = NoTracking,
	template <typename ToType> class CastingPolicy = StaticCast>
class Callback :
		public CallbackBase,
		public TrackingPolicy,
		public CastingPolicy<SignalType&>,
		public boost::noncopyable /* prevents references to _callback to get invalidated */ {

public:

	typedef SignalType                         signal_type;
	typedef boost::function<void(SignalType&)> callback_type;

	/**
	 * Create a new callback.
	 *
	 * @param callback
	 *              Any expression that can be cast into a 
	 *              boost::function<void(SignalType&)>.
	 *
	 * @param invocation
	 *              Optional invocation type. Influences which callbacks are  
	 *              connected to signals, if several callbacks are compatible.  
	 *              See CallbackInvocation.
	 */
	Callback(callback_type callback, CallbackInvocation invocation = Exclusive) :
		_callback(callback) {

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

		/* Here, we cast whatever comes to Slot<SignalType>. This means,
		 * that we every once in a while cast Slot<Dervied> to Slot<Base>
		 * (note that the Slots themselves are not dervied from each other).
		 *
		 * What can go wrong?
		 *
		 * The only thing we do is to call connect(Callback<Base>), where
		 * instead the object implements connect(Callback<Derived>). In
		 * connect, we take the actual boost::signal and connect
		 * Callback<Base>::operator()(Signal&) to it.
		 *
		 * Since all the Slot<SignalType>s are identical in memory (we know
		 * this since we made them), the static casts are safe. We will talk
		 * to the right object with the wrong name, but this doesn't matter,
		 * since they all have a boost::signal of the same name -- and there
		 * everything is save again.
		 */
		Slot<SignalType>& s = static_cast<Slot<SignalType>&>(slot);

		s.addCallback(*this);

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

		/* Here, we cast whatever comes to Slot<SignalType>. This means,
		 * that we every once in a while cast Slot<Dervied> to Slot<Base>
		 * (note that the Slots themselves are not dervied from each other).
		 *
		 * What can go wrong?
		 *
		 * The only thing we do is to call connect(Callback<Base>), where
		 * instead the object implements connect(Callback<Derived>). In
		 * connect, we take the actual boost::signal and connect
		 * Callback<Base>::operator()(Signal&) to it.
		 *
		 * Since all the Slot<SignalType>s are identical in memory (we know
		 * this since we made them), the static casts are safe. We will talk
		 * to the right object with the wrong name, but this doesn't matter,
		 * since they all have a boost::signal of the same name -- and there
		 * everything is save again.
		 */
		Slot<SignalType>& s = static_cast<Slot<SignalType>&>(slot);

		s.disconnect(*this);

		return true;
	}

	/**
	 * Get an invoker for this callback. The invoker can be used as a function 
	 * to send a signal to this callback.
	 */
	CallbackInvoker<SignalType> getInvoker() {

		// delegate creation of the invoker to the tracking policy
		return this->createInvoker(boost::ref(_callback));
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

	callback_type _callback;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_H__

