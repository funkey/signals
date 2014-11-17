#ifndef SIGNALS_CASTING_POLICY_H__
#define SIGNALS_CASTING_POLICY_H__

namespace signals {

template <typename ToType>
class StaticCast {

public:

	template <typename FromType>
	ToType cast(FromType& from) {

		return static_cast<ToType>(from);
	}
};

} // namespace signals

#endif // SIGNALS_CASTING_POLICY_H__

