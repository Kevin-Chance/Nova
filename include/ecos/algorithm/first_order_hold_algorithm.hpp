#pragma once

#include <memory>
#include <cstddef>

#include "ecos/algorithm/algorithm.hpp"   // ← 联合仿真算法基类
#include "ecos/model_instance.hpp"        // ← model_instance 前置声明依赖

namespace nova_sim
{

	class first_order_hold_algorithm
	{
	public:
		first_order_hold_algorithm(double stepSize, bool parallel);

		void model_instance_added(model_instance* instance);

		double step(double currentTime);

		~first_order_hold_algorithm();

	private:
		class impl;
		std::unique_ptr<impl> pimpl_;
	};

} // namespace nova_sim

