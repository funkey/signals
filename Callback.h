#ifndef SIGNALS_CALLBACK_H__
#define SIGNALS_CALLBACK_H__

#include <cassert>

#include <boost/function.hpp>

#include <util/typename.h>
#include "Slot.h"
#include "Logging.h"

namespace signals {

/**
 * Type to indicate how to invoke a callback.
 */
enum CallbackInvocation {

	/**
	 * Of all compatible exclusive callbacks, only the most specific will be
	 * called.
	 */
	Exclusive,

	/**
	 * Transparent callbacks will always be called, regardless of the existance
	 * of other, possibly more specific, callbacks.
	 */
	Transparent
};

class CallbackBase {

public:

	CallbackBase() :
		_isTransparent(false) {}

	virtual ~CallbackBase() {}

	/**
	 * Comparison operator that sorts callbacks according to their specificity:
	 *
	 * Callback<Derived> < Callback<Base>
	 */
	bool operator<(const CallbackBase& other) const {

		// Return true, if the other callback accepts our signals as well. This
		// means that our signal type ≤ other signal type, i.e., we are more
		// specific.
		return other.accepts(createSignal());
	}

	/**
	 * Try to connect to the given slot. For a successful connection, the slot's
	 * signal type has to be castable to the callbacks signal type.
	 *
	 * @return True, if the connection could be established.
	 */
	virtual bool tryToConnect(SlotBase& slot) = 0;

	/**
	 * Disconnect from the given slot.
	 *
	 * @return True, if the callback was previously connected to the slot.
	 */
	virtual bool disconnect(SlotBase& slot) = 0;

	/**
	 * Make this callback transparent. Transparent callbacks will always be
	 * called, regardless of the existance of other, possibly more specific,
	 * callbacks. Thus, they are transparent to these other callbacks.
	 */
	void setTransparent(bool transparent = true) {

		_isTransparent = transparent;
	}

	/**
	 * Returns true, if this is a transparent callback.
	 */
	bool isTransparent() const {

		return _isTransparent;
	}

protected:

	/**
	 * Return true, if the passed signal can be cast to the signal type this
	 * callback accepts.
	 */
	virtual bool accepts(const Signal& signal) const = 0;

	/**
	 * Create a reference signal for run-time type inference. This reference is
	 * used to find compatible pairs of slots and callbacks.
	 */
	virtual const Signal& createSignal() const = 0;

private:

	// indicates that this is a transparent callback
	bool _isTransparent;
};

/**
 * Sorts callbacks based on their invokation type (exclusive precedes 
 * transparent) and then on the specificity of their signals, i.e., the most 
 * specific callbacks come first.
 */
struct CallbackComparator {

	bool operator()(const CallbackBase* a, const CallbackBase* b) const {

		// sort all transparent callbacks to the end...
		if (a->isTransparent() != b->isTransparent()) {

			if (a->isTransparent())
				return false;

			return true;
		}

		// ...then consider the specificity of the signals
		return *a < *b;
	}
};

/**
 * No-tracking strategy for callbacks. The slots will only keep the plain 
 * functor implemented by the callback.
 */
class NoTracking {

public:

	template <typename CallbackType>
	boost::reference_wrapper<CallbackType> wrap(CallbackType& callback) const {

		return boost::ref(callback);
	}
};

/**
 * Weak pointer tracking strategy for callbacks. For callbacks that use this 
 * strategy, a connected slot will keep a weak pointer to the callback's holder 
 * (set via track() on the callback). The weak pointer is locked whenever a 
 * signal needs to be sent. If locking fails, i.e., the holder does not live 
 * anymore, the callback gets automatically removed from the slot.
 */
template <typename SignalType, typename HolderType>
class WeakTracking {

	typedef boost::signals2::signal<void(SignalType&)> boost_signal_type;
	typedef typename boost_signal_type::slot_type      boost_slot_type;

public:

	void track(boost::shared_ptr<HolderType> holder) const {

		_holder = holder;
	}

	template <typename CallbackType>
	typename boost::signals2::signal<void(typename CallbackType::signal_type&)>::slot_type wrap(CallbackType& callback) {

		boost::shared_ptr<HolderType> sharedHolder = _holder.lock();

		return boost_slot_type(boost::ref(callback)).track(sharedHolder);
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
template <typename SignalType, typename HolderType>
class SharedTracking {

	typedef boost::signals2::signal<void(SignalType&)> boost_signal_type;
	typedef typename boost_signal_type::slot_type      boost_slot_type;

public:

	template <typename CallbackType>
	struct SharedHolderCallback {

		SharedHolderCallback(CallbackType& callback_, boost::shared_ptr<HolderType> holder_) :
			callback(&callback_),
			holder(holder_) {}

		void operator()(SignalType& signal) {

			(*callback)(signal);
		}

		CallbackType* callback;
		boost::shared_ptr<HolderType> holder;
	};

	void track(boost::shared_ptr<HolderType> holder) const {

		_holder = holder;
	}

	template <typename CallbackType>
	SharedHolderCallback<CallbackType> wrap(CallbackType& callback) {

		boost::shared_ptr<HolderType> sharedHolder = _holder.lock();

		return SharedHolderCallback<CallbackType>(callback, sharedHolder);
	}

private:

	mutable boost::weak_ptr<HolderType> _holder;
};

template <typename ToType>
class StaticCast {

public:

	template <typename FromType>
	ToType cast(FromType& from) {

		return static_cast<ToType>(from);
	}
};

template <
	typename SignalType,
	class TrackingPolicy = NoTracking,
	template <typename ToType> class CastingPolicy = StaticCast>
class Callback : public CallbackBase, public TrackingPolicy, public CastingPolicy<SignalType&>, public boost::noncopyable {

public:

	typedef SignalType                         signal_type;
	typedef boost::function<void(SignalType&)> callback_type;

	Callback(callback_type callback, CallbackInvocation invocation = Exclusive) :
		_callback(callback) {

		if (invocation == Transparent)
			setTransparent();
	}

	bool accepts(const Signal& signal) const {

		return dynamic_cast<const SignalType*>(&signal);
	}

	bool tryToConnect(SlotBase& slot) {

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

		s.connect(*this);

		return true;
	}

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

		return s.disconnect(*this);
	}

	/* The point of this method is to cast the signal -- that was transmitted as
	 * a Signal -- to SignalType (where SignalType can be ≥ than the actual type
	 * of the signal).
	 */
	void operator()(Signal& signal) {

		SignalType& casted = CastingPolicy<SignalType&>::cast(signal);

		_callback(casted);
	}

	/* The point of this method is to cast the signal -- that was transmitted as
	 * a Signal -- to SignalType (where SignalType can be ≥ than the actual type
	 * of the signal).
	 */
	void operator()(const Signal& signal) {

		const SignalType& casted = CastingPolicy<SignalType&>::cast(signal);

		_callback(casted);
	}

protected:

	const Signal& createSignal() const {

		return Slot<SignalType>::referenceSignal;
	}

private:

	callback_type _callback;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_H__

