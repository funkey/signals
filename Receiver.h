#ifndef SIGNALS_RECEIVER_H__
#define SIGNALS_RECEIVER_H__

#include <vector>
#include "CallbackBase.h"
#include "CallbackComparator.h"

namespace signals {

class Receiver {

public:

	typedef std::list<CallbackBase*> callbacks_type;

	~Receiver() {

		for (auto* callback : _own)
			delete callback;
	}

	/**
	 * Add a callback to this receiver, keep ownership.
	 */
	void registerCallback(CallbackBase& callback) {

		// make sure that transparent callbacks or callbacks of the same 
		// specificity are called in the reverse order in which they have been 
		// added
		callback.setPrecendence(_callbacks.size());

		_callbacks.push_back(&callback);
		_callbacks.sort(CallbackComparator());
	}

	/**
	 * Add a callback to this receiver, transmit ownership.
	 */
	void registerCallback(CallbackBase* callback) {

		// make sure that transparent callbacks or callbacks of the same 
		// specificity are called in the reverse order in which they have been 
		// added
		callback->setPrecendence(_callbacks.size());

		_callbacks.push_back(callback);
		_own.push_back(callback);
		_callbacks.sort(CallbackComparator());
	}

	callbacks_type& getCallbacks() {

		return _callbacks;
	}

private:

	callbacks_type _callbacks;
	callbacks_type _own;
};

} // namespace signals

#endif // SIGNALS_RECEIVER_H__

