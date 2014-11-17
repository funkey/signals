#ifndef SIGNALS_SLOT_COMPARATOR_H__
#define SIGNALS_SLOT_COMPARATOR_H__

namespace signals {

struct SlotComparator {

	bool operator()(const SlotBase* a, const SlotBase* b) const {

		return *a < *b;
	}
};

} // namespace signals

#endif // SIGNALS_SLOT_COMPARATOR_H__

