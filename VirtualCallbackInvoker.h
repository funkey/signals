#ifndef SIGNALS_VIRTUAL_CALLBACK_INVOKER_H__
#define SIGNALS_VIRTUAL_CALLBACK_INVOKER_H__

#include "Signal.h"
#include "VirtualCallbackBase.h"

namespace signals {

/**
 * Specialized functor to send signals of type SignalType to a callback that is 
 * described by a virtual function.
 *
 * Concept Handler:
 *
 *   class Handler {
 *   public:
 *     virtual ~Handler() {}
 *     virtual void onSignal(SignalType& signal) = 0;
 *   };
 */
template <typename SignalType, typename HandlerType>
class VirtualCallbackInvoker {

public:

	// accepts only virtual callbacks
	typedef typename HandlerType::HandlerBaseType HandlerBaseType;
	typedef VirtualCallbackBase<HandlerBaseType>  CallbackBaseType;

	/**
	 * Dummy lock. VirtualCallbackInvoker can not be locked.
	 */
	class Lock {

	public:

		inline operator bool() {

			return true;
		}
	};

	VirtualCallbackInvoker() :
		_handler(0),
		_ownHandler(false) {}

	VirtualCallbackInvoker(CallbackBaseType& callback) {

		// We already know that SignalType can be cast into the signal that 
		// callback is expecting. If his signal can also be cast into 
		// SignalType, we have a match. In this case, we can assume that the 
		// handler is of HandlerType<SignalType>.

		if (const SignalType* signal = dynamic_cast<const SignalType*>(&callback.createSignal())) {

			_handler = static_cast<HandlerType*>(callback.handler());
			_ownHandler = false;

		} else {

			_handler = new Relay(callback.relayFunction());
			_ownHandler = true;
		}
	}

	/**
	 * Move constructor.
	 */
	VirtualCallbackInvoker(VirtualCallbackInvoker<SignalType, HandlerType>&& other) :
		_handler(other._handler),
		_ownHandler(other._ownHandler) {

		other._handler = 0;
		other._ownHandler = false;
	}

	/**
	 * Move assignment.
	 */
	VirtualCallbackInvoker<SignalType, HandlerType>& operator=(VirtualCallbackInvoker<SignalType, HandlerType>&& other) {

		_handler = other._handler;
		_ownHandler = other._ownHandler;
		other._handler = 0;
		other._ownHandler = false;

		return *this;
	}

	/**
	 * Deallocate handler, if it was created by us.
	 */
	~VirtualCallbackInvoker() {

		if (_ownHandler)
			delete _handler;
	}

	/**
	 * Lock this callback invoker. VirtualCallbackInvoker will always return a 
	 * good lock.
	 */
	inline Lock lock() {

		return Lock();
	}

	/**
	 * Send a signal via this callback invoker.
	 */
	template <typename T>
	bool operator()(T& signal) {

		_handler->onSignal(signal);
		return true;
	}

	/**
	 * Comparison operator. Two invokers are considered equal, if they call the 
	 * same function.
	 */
	bool operator==(const VirtualCallbackInvoker<SignalType, HandlerType>& other) const {

		if (_ownHandler != other._ownHandler)
			return false;

		return _handler == other._handler;

		// TODO: for relay invokers, test target of std::function:
		//return (_callback.get_pointer() == other._callback.get_pointer());
	}

private:

	class Relay : public HandlerType {

	public:

		Relay(std::function<void(Signal&)> fun) : _fun(fun) {}

	private:

		void onSignal(SignalType& signal) override {

			_fun(signal);
		}

		std::function<void(Signal&)> _fun;
	};

	HandlerType* _handler;

	// did we create the handler?
	bool _ownHandler;
};

} // namespace signals

#endif // SIGNALS_VIRTUAL_CALLBACK_INVOKER_H__

