#ifndef SIGNALS_RECEIVER_H__
#define SIGNALS_RECEIVER_H__

#include <vector>
#include "Callback.h"

namespace signals {

class Receiver {

public:

	typedef std::list<CallbackBase*> callbacks_type;

	void registerCallback(CallbackBase& callback) {

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

