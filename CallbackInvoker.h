#ifndef SIGNALS_CALLBACK_INVOKER_H__
#define SIGNALS_CALLBACK_INVOKER_H__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "Logging.h"
#include "CallbackBase.h"

namespace signals {

/**
 * Generic functor to send signals of type SignalType to a callback. The 
 * callback is stored as a std::function.
 */
template <typename SignalType>
class CallbackInvoker {

public:

	// accepts all callbacks
	typedef CallbackBase CallbackBaseType;

	/**
	 * Lock to guard this callback invoker. Successful locking guarantees that 
	 * the weakly tracked object (if it was set) is still alive and will stay 
	 * alive for the duration of the lock.
	 */
	class Lock {

	public:

		Lock(bool isGood) :
			_isGood(isGood),
			_isWeakTracking(false) {

			LOG_ALL(signalslog) << "created an always " << (isGood ? "good" : "bad") << " lock" << std::endl;
		}

		Lock(boost::weak_ptr<void> weakTrackedObject) :
			_isGood(true),
			_isWeakTracking(true),
			_weakObjectLock(weakTrackedObject.lock()) {}

		operator bool() {

			if (!_isGood)
				return false;

			if (_isWeakTracking)
				return static_cast<bool>(_weakObjectLock);

			return true;
		}

	private:

		bool _isGood;
		bool _isWeakTracking;

		boost::shared_ptr<void> _weakObjectLock;
	};

	CallbackInvoker(const CallbackBase& callback) :
		_callback(boost::ref(callback.relayFunction())),
		_isWeakTracking(false),
		_isSharedTracking(false) {}

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
	Lock lock() {

		LOG_ALL(signalslog) << "create lock, weak tracking = " << _isWeakTracking << ", shared tracking = " << _isSharedTracking << ", shared object = " << _sharedTrackedObject << std::endl;

		if (_isWeakTracking)
			return Lock(_weaklyTrackedObject);
		else if (_isSharedTracking && !_sharedTrackedObject)
			return Lock(false);

		return Lock(true);
	}

	/**
	 * Send a signal via this callback invoker.
	 */
	template <typename T>
	void operator()(T& signal) {

		boost::unwrap_ref(_callback)(signal);
	}

	/**
	 * Comparison operator. Two invokers are considered equal, if they call the 
	 * same function.
	 */
	bool operator==(const CallbackInvoker<SignalType>& other) {

		return (_callback.get_pointer() == other._callback.get_pointer());
	}

private:

	// the function that will be called by this invoker
	boost::reference_wrapper<const std::function<void(Signal&)> > _callback;

	// weak pointer to an object that is tracked by this invoker
	boost::weak_ptr<void> _weaklyTrackedObject;

	// shared pointer to an object that is tracked by this invoker
	boost::shared_ptr<void> _sharedTrackedObject;

	bool _isWeakTracking;
	bool _isSharedTracking;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_INVOKER_H__

