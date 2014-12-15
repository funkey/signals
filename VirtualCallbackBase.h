#ifndef SIGNALS_VIRTUAL_CALLBACK_BASE_H__
#define SIGNALS_VIRTUAL_CALLBACK_BASE_H__

#include "CallbackBase.h"

namespace signals {

template <typename HandlerBaseType>
class VirtualCallbackBase : public CallbackBase {

public:

	VirtualCallbackBase(std::function<void(Signal&)> relayFunction, HandlerBaseType* handler) :
		CallbackBase(relayFunction),
		_handler(handler) {}

	HandlerBaseType* handler() const { return _handler; }

private:

	HandlerBaseType* _handler;
};

} // namespace signals

#endif // SIGNALS_VIRTUAL_CALLBACK_BASE_H__

