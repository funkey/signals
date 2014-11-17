#ifndef SIGNALS_RECEIVER_H__
#define SIGNALS_RECEIVER_H__

#include <vector>
#include "CallbackBase.h"
#include "CallbackComparator.h"

namespace signals {

class Receiver {

public:

	typedef std::list<CallbackBase*> callbacks_type;

	void registerCallback(CallbackBase& callback) {

		// make sure that transparent callbacks or callbacks of the same 
		// specificity are called in the reverse order in which they have been 
		// added
		callback.setPrecendence(_callbacks.size());

		_callbacks.push_back(&callback);
		_callbacks.sort(CallbackComparator());
	}

	callbacks_type& getCallbacks() {

		return _callbacks;
	}

private:

	callbacks_type _callbacks;
};

} // namespace signals

#endif // SIGNALS_RECEIVER_H__

