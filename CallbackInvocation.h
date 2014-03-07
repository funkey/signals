#ifndef SIGNALS_CALLBACK_INVOCATION_H__
#define SIGNALS_CALLBACK_INVOCATION_H__

namespace signals {

/**
 * Type to indicate how to invoke a callback.
 */
enum CallbackInvocation {

	/**
	 * Of all compatible exclusive callbacks, only the most specific will be
	 * called.
	 */
	Exclusive,

	/**
	 * Transparent callbacks will always be called, regardless of the existance
	 * of other, possibly more specific, callbacks.
	 */
	Transparent
};

} // namespace signals

#endif // SIGNALS_CALLBACK_INVOCATION_H__

