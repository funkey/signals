#ifndef SIGNALS_CALLBACK_COMPARATOR_H__
#define SIGNALS_CALLBACK_COMPARATOR_H__

#include "CallbackBase.h"

namespace signals {

/**
 * Sorts callbacks based on their invokation type (exclusive precedes 
 * transparent) and then on the specificity of their signals, i.e., the most 
 * specific callbacks come first.
 */
struct CallbackComparator {

	bool operator()(const CallbackBase* a, const CallbackBase* b) const {

		// both are transparent, the precedence decides
		if (a->isTransparent() && b->isTransparent())
			return a->getPrecedence() > b->getPrecedence();

		// both are exclusive, casting decides, then precedence
		if (!a->isTransparent() && !b->isTransparent()) {

			if (*a < *b)
				return true;

			if (*b < *a)
				return false;

			return a->getPrecedence() > b->getPrecedence();
		}

		// exactly one of them is transparent, sort it to the back
		return b->isTransparent();
	}
};

} // namespace signals

#endif // SIGNALS_CALLBACK_COMPARATOR_H__

