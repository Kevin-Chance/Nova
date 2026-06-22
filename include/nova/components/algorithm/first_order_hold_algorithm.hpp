#pragma once

#include <memory>
#include <cstddef>

#include "nova/components/algorithm/algorithm.hpp"   // ← 联合仿真算法基类
#include "nova/engine/model_instance.hpp"        // ← model_instance 前置声明依赖

namespace nova_sim
{

	class first_order_hold_algorithm : public algorithm
	{
	public:
		first_order_hold_algorithm(double stepSize, bool parallel);

		void model_instance_added(model_instance* instance);

		void initialize(double startTime) override {}
		double step(double currentTime, nova_engine& sim) override;

		~first_order_hold_algorithm() override;

	private:
		class impl;
		std::unique_ptr<impl> pimpl_;
	};

} // namespace nova_sim

