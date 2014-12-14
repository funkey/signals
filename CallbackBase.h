#ifndef SIGNALS_CALLBACK_BASE_H__
#define SIGNALS_CALLBACK_BASE_H__

namespace signals {

// forward declarations
class Signal;
class SlotBase;

class CallbackBase {

public:

	CallbackBase(std::function<void(Signal&)> relayFunction) :
		_relayFunction(relayFunction),
		_isTransparent(false),
		_precedence(0) {}

	virtual ~CallbackBase() {}

	/**
	 * Make this callback transparent. Transparent callbacks will always be
	 * called, regardless of the existance of other, possibly more specific,
	 * callbacks in the same receiver. Thus, they are transparent to these other 
	 * callbacks.
	 */
	void setTransparent(bool transparent = true) {

		_isTransparent = transparent;
	}

	/**
	 * Returns true, if this is a transparent callback.
	 */
	bool isTransparent() const {

		return _isTransparent;
	}

	/**
	 * Set the precedence for this callback. If two callbacks are both 
	 * transparent or have the same specificity, the one with the higher 
	 * precedence will be called first.
	 */
	void setPrecendence(unsigned int precedence) {

		_precedence = precedence;
	}

	/**
	 * Get the precedence of this callback.
	 */
	unsigned int getPrecedence() const {

		return _precedence;
	}

	/**
	 * Try to connect to the given slot. For a successful connection, the slot's
	 * signal type has to be castable to the callbacks signal type.
	 *
	 * @return True, if the connection could be established.
	 */
	virtual bool connect(SlotBase& slot) = 0;

	/**
	 * Disconnect from the given slot.
	 *
	 * @return True, if the callback was previously connected to the slot.
	 */
	virtual bool disconnect(SlotBase& slot) = 0;

	/**
	 * Comparison operator that sorts callbacks according to their specificity:
	 *
	 * Callback<Derived> < Callback<Base>
	 */
	bool operator<(const CallbackBase& other) const {

		// Return true, if the other callback accepts our signals as well. This
		// means that our signal type ≤ other signal type, i.e., we are more
		// specific.
		return other.accepts(createSignal());
	}

	/**
	 * Get the relay function, i.e., the most general form of the function 
	 * provided by this callback. Calling this function with the signal (and 
	 * derived signals) provided by this callback should be valid.
	 */
	const std::function<void(Signal&)>& relayFunction() const {

		return _relayFunction;
	}

protected:

	/**
	 * Return true, if the passed signal can be cast to the signal type this
	 * callback accepts.
	 */
	virtual bool accepts(const Signal& signal) const = 0;

	/**
	 * Create a reference signal for run-time type inference. This reference is
	 * used to find compatible pairs of slots and callbacks.
	 */
	virtual const Signal& createSignal() const = 0;

private:

	// the most general way to provide the callback
	std::function<void(Signal&)> _relayFunction;

	// indicates that this is a transparent callback
	bool _isTransparent;

	// final sorting criteria for otherwise equal callbacks
	unsigned int _precedence;
};

} // namespace signals

#endif // SIGNALS_CALLBACK_BASE_H__

