#ifndef SIGNALS_VIRTUAL_CALLBACK_H__
#define SIGNALS_VIRTUAL_CALLBACK_H__

#include "CallbackBase.h"
#include "CallbackInvocation.h"
#include "VirtualCallbackBase.h"
#include "VirtualCallbackInvoker.h"
#include "CastingPolicy.h"
#include "SignalTraits.h"
#include "Slot.h"

namespace signals {

/**
 * Specialized callback for a virtual function handler. Represents a callback 
 * that defers to a virtual method provided by a class modelling the concept 
 * Handler:
 *
 *   class Handler {
 *   public:
 *     virtual ~Handler() {}
 *     virtual void on(SignalType& signal) = 0;
 *   };
 */
template <typename SignalType, typename HandlerType>
class VirtualCallback : public VirtualCallbackBase<typename HandlerType::HandlerBaseType> {

public:

	/**
	 * Create a new callback that defers to a virtual method
	 *
	 *   void on(SignalType&)
	 *
	 * of the given handler.
	 *
	 * @param handler
	 *              Pointer to a handler that models the Handler concept for 
	 *              SignalType.
	 */
	VirtualCallback(HandlerType* handler, CallbackInvocation invocation = Exclusive) :
		VirtualCallbackBase<typename HandlerType::HandlerBaseType>([this](Signal& signal){ static_cast<HandlerType*>(this->handler())->on(static_cast<SignalType&>(signal)); }, handler),
		_handler(handler) {

		if (invocation == Transparent)
			this->setTransparent();
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

		return slot.addCallback(*this);
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

		return slot.removeCallback(*this);
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

		return SignalTraits<SignalType>::Reference;
	}

	HandlerType* _handler;
};

} // namespace signals

#endif // SIGNALS_VIRTUAL_CALLBACK_H__

