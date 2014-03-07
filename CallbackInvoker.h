#ifndef SIGNALS_CALLBACK_INVOKER_H__
#define SIGNALS_CALLBACK_INVOKER_H__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace signals {

/**
 * Functor to send signals of type SignalType to a callback.
 */
template <typename SignalType>
class CallbackInvoker {

public:

	/**
	 * Lock to guard this callback invoker. Successful locking guarantees that 
	 * the weakly tracked object (if it was set) is still alive and will stay 
	 * alive for the duration of the lock.
	 */
	class Lock {

	public:

		Lock(bool isGood) :
			_isGood(isGood),
			_isWeakTracking(false) {}

		Lock(boost::weak_ptr<void> weakTrackedObject) :
			_isGood(true),
			_isWeakTracking(true),
			_weakObjectLock(weakTrackedObject.lock()) {}

		operator bool() {

			if (!_isGood)
				return false;

			if (_isWeakTracking)
				return _weaklyTrackedObject;

			return true;
		}

	private:

		bool _isWeakTracking;

		boost::shared_ptr<void> _weakObjectLock;
	};

	/**
	 * Create a callback invoker from a boost::function.
	 */
	CallbackInvoker(boost::function<void(SignalType&)> callback) :
		_callback(callback) {}

	/**
	 * Register an object for weak tracking. The invoker will only be successful 
	 * locked, if the tracked object does still exist.
	 */
	void setWeakTracking(boost::weak_ptr<void> object) {

		_weaklyTrackedObject = object;
		_isWeakTracking = true;
	}

	/**
	 * Register an object for shared tracking. As long as this invoker exists, 
	 * the registered object will be kept alive.
	 */
	void setSharedTracking(boost::shared_ptr<void> object) {

		_sharedTrackedObject = object;
		_isSharedTracking = true;
	}

	/**
	 * Lock this callback invoker. If a weak tracking object was set, successful 
	 * locking ensures that the object still exists and will be alive for the 
	 * duration of the lock.
	 *
	 * Usage:
	 *
	 *   CallbackInvoker<MySignal>::Lock lock = invoker.lock();
	 *
	 *   if (lock)
	 *     invoker(signal); // save to assume weak tracked object exists
	 */
	Lock& lock() {

		if (_isWeakTracking)
			return Lock(_weaklyTrackedObject);
		else if (_isSharedTracking && !_sharedTrackedObject)
			return Lock(false);

		return Lock(true);
	}

	/* The point of this method is to cast the signal -- that was transmitted as
	 * a Signal -- to SignalType (where SignalType can be ≥ than the actual type
	 * of the signal).
	 */
	void operator()(SignalType& signal) {

		_callback(signal);
	}

	/* The point of this method is to cast the signal -- that was transmitted as
	 * a Signal -- to SignalType (where SignalType can be ≥ than the actual type
	 * of the signal).
	 */
	void operator()(const SignalType& signal) {

		_callback(signal);
	}

	/**
	 * Comparison operator. Two invokers are considered equal, if they call the 
	 * same function.
	 */
	bool operator==(const CallbackInvoker<SignalType>& other) {

		return _callback == other._callback;
	}

private:

	// the function that will be called by this invoker
	boost::function<void(SignalType&)> _callback;

	// weak pointer to an object that is tracked by this invoker
	boost::weak_ptr<void> _weaklyTrackedObject;

	// shared pointer to an object that is tracked by this invoker
	boost::shared_ptr<void> _sharedTrackedObject;

	bool _isWeakTracking;
	bool _isSharedTracking;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_INVOKER_H__

