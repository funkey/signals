#ifndef SIGNALS_PASS_THROUGH_CALLBACK_BASE_H__
#define SIGNALS_PASS_THROUGH_CALLBACK_BASE_H__

#include <set>
#include "CallbackBase.h"
#include "SlotBase.h"

namespace signals {

// forward declaration
class PassThroughSlotBase;

class PassThroughCallbackBase : public CallbackBase {

public:

	PassThroughCallbackBase() :
		CallbackBase([](Signal&){}),
		_target(0) {}

	typedef std::set<SlotBase*> slots_type;

	/**
	 * Get all slots that are registered to this callback. This set is needed  
	 * by the PassThroughSlot at the other side to establish connections.
	 */
	std::set<SlotBase*>& getSlots() {

		return _slots;
	}

protected:

	/**
	 * Add a slot to this PassThroughCallback for future connections.
	 */
	void addSlot(SlotBase& slot) {

		_slots.insert(&slot);
	}

	/**
	 * Remove a slot from this PassThroughCallback.
	 */
	bool removeSlot(SlotBase& slot) {

		return _slots.erase(&slot) == 1;
	}

	void setTarget(PassThroughSlotBase& target) {

		_target = &target;
	}

	PassThroughSlotBase& getTarget() { return *_target; }

private:

	slots_type _slots;

	// the other end of this pass-through tunnel
	PassThroughSlotBase* _target;
};

} // namespace signals


#endif // SIGNALS_PASS_THROUGH_CALLBACK_BASE_H__

