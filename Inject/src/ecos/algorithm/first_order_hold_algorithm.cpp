#include "ecos/algorithm/first_order_hold_algorithm.hpp"

#include "ecos/logger/logger.hpp"

#include <algorithm>
#include <cmath>
#include <execution>
#include <vector>

using namespace ecos;

namespace
{

	struct instance_wrapper
	{
		int decimationFactor;
		model_instance* instance;

		// FOH 所需历史量
		bool hasPrev = false;
		double prevTime = 0.0;
	};

	int calculateDecimationFactor(const model_instance& m, double baseStepSize)
	{
		constexpr double EPS = 1e-3;

		const auto& stepSizeHint = m.stepSizeHint();
		if (!stepSizeHint) return 1;

		const int factor =
			std::max(1, static_cast<int>(std::ceil(*stepSizeHint / baseStepSize)));

		const double actualStepSize = baseStepSize * factor;
		if (std::fabs(actualStepSize - *stepSizeHint) >= EPS) {
			log::warn(
				"Actual stepSize for {} will be {} rather than requested value {}",
				m.instanceName(), actualStepSize, *stepSizeHint);
		}

		return factor;
	}

} // namespace

  // ====================== impl ======================

class first_order_hold_algorithm::impl
{
public:
	impl(double stepSize, bool parallel)
		: parallel_(parallel)
		, stepSize_(stepSize)
		, stepNumber_(0)
	{}

	void model_instance_added(model_instance* instance)
	{
		const int factor = calculateDecimationFactor(*instance, stepSize_);
		instances_.push_back(instance_wrapper{ factor, instance });
	}

	double step(double currentTime)
	{
		auto f = [this, currentTime](auto& wrapper)
		{
			if (!should_step(stepNumber_, wrapper.decimationFactor))
				return;

			auto& props = wrapper.instance->get_properties();

			// ---------- FOH 输入预测 ----------
			if (wrapper.hasPrev)
			{
				const double dt = currentTime - wrapper.prevTime;
				if (dt > 0.0)
				{
					// 对所有 set 变量做线性外推
					props.extrapolate_linear(stepSize_);
				}
			}

			// ---------- 写入预测输入 ----------
			props.apply_sets();

			// ---------- FMU 步进 ----------
			wrapper.instance->step(currentTime, stepSize_);

			// ---------- 读取新输出 ----------
			props.apply_gets();

			// ---------- 更新历史 ----------
			wrapper.prevTime = currentTime;
			wrapper.hasPrev = true;
		};

		if (!parallel_) {
			std::ranges::for_each(instances_, f);
		}
		else {
			std::for_each(std::execution::par,
				instances_.begin(),
				instances_.end(),
				f);
		}

		++stepNumber_;
		return currentTime + stepSize_;
	}

private:
	bool parallel_;
	double stepSize_;
	size_t stepNumber_;
	std::vector<instance_wrapper> instances_;

	static bool should_step(size_t step, int factor)
	{
		return step % factor == 0;
	}
};

// ====================== API ======================

first_order_hold_algorithm::first_order_hold_algorithm(
	double stepSize, bool parallel)
	: pimpl_(std::make_unique<impl>(stepSize, parallel))
{}

void first_order_hold_algorithm::model_instance_added(
	model_instance* instance)
{
	pimpl_->model_instance_added(instance);
}

double first_order_hold_algorithm::step(double currentTime)
{
	return pimpl_->step(currentTime);
}

first_order_hold_algorithm::~first_order_hold_algorithm() = default;
