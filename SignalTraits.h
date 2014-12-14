#ifndef SIGNALS_SIGNAL_TRAITS_H__
#define SIGNALS_SIGNAL_TRAITS_H__

namespace signals {

template <typename SignalType>
class SignalTraits {

public:

	static SignalType Reference;
};

// If you got an error here, that means most likely that you have a signal that
// does not provide a default constructor.
template <typename SignalType> SignalType SignalTraits<SignalType>::Reference = SignalType();

} // namespace signals

#endif // SIGNALS_SIGNAL_TRAITS_H__

