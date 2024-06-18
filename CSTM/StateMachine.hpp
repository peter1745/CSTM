#pragma once

#include "EnumUtils.hpp"

#include <functional>

namespace CSTM {

	template<scoped_enum StateT>
	class StateMachine
	{
		// TODO(Peter): Replace with FunctionRef
		using StateFunc = std::function<void(StateMachine&)>;
	public:

		explicit StateMachine(StateT initialState = {})
			: current_state(initialState), next_state(initialState) {}

		template<StateT State>
		StateMachine& state(StateFunc&& func)
		{
			states[std::to_underlying(State)] = CSTM_Move(func);
			return *this;
		}

		template<StateT State>
		void next()
		{
			next_state = State;
		}

		StateMachine& run()
		{
			while (next_state != StateT::Max)
			{
				current_state = next_state;
				next_state = StateT::Max;
				states[std::to_underlying(current_state)](*this);
			}

			return *this;
		}

		void stop()
		{
			next_state = StateT::Max;
		}

		StateT current_state, next_state;
		std::array<StateFunc, EnumTraits<StateT>::max()> states;
	};
}
