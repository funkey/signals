#ifndef SIGNALS_CALLBACK_TRACKING_H__
#define SIGNALS_CALLBACK_TRACKING_H__

#include "CallbackInvoker.h"

namespace signals {

/**
 * No-tracking strategy for callbacks. The slots will only keep the plain 
 * functor implemented by the callback.
 */
class NoTracking {

public:

	template <typename SignalType>
	CallbackInvoker<SignalType> createInvoker(boost::reference_wrapper<boost::function<void(SignalType&)> > callback) {

		return CallbackInvoker<SignalType>(callback);
	}
};

/**
 * Weak pointer tracking strategy for callbacks. For callbacks that use this 
 * strategy, a connected slot will keep a weak pointer to the callback's holder 
 * (set via track() on the callback). The weak pointer is locked whenever a 
 * signal needs to be sent. If locking fails, i.e., the holder does not live 
 * anymore, the callback gets automatically removed from the slot.
 */
template <typename HolderType>
class WeakTracking {

public:

	void track(boost::shared_ptr<HolderType> holder) const {

		_holder = holder;
	}

	template <typename SignalType>
	CallbackInvoker<SignalType> createInvoker(boost::reference_wrapper<boost::function<void(SignalType&)> > callback) {

		CallbackInvoker<SignalType> invoker(callback);
		invoker.setWeakTracking(_holder);

		return invoker;
	}

private:

	mutable boost::weak_ptr<HolderType> _holder;
};

/**
 * Shared pointer tracking for callbacks. For callbacks that use this strategy, 
 * a connected slot will keep a shared pointer to the callback's holder and thus 
 * makes sure that the holder will live at least as long as the connection to 
 * the slot is established.
 */
template <typename HolderType>
class SharedTracking {

public:

	void track(boost::shared_ptr<HolderType> holder) const {

		_holder = holder;
	}

	template <typename SignalType>
	CallbackInvoker<SignalType> createInvoker(boost::reference_wrapper<boost::function<void(SignalType&)> > callback) {

		CallbackInvoker<SignalType> invoker(callback);
		invoker.setSharedTracking(_holder.lock());

		return invoker;
	}

private:

	mutable boost::weak_ptr<HolderType> _holder;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_TRACKING_H__

