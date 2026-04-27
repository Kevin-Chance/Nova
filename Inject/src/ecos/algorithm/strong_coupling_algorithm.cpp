#include "ecos/algorithm/strong_coupling_algorithm.hpp"

#include "ecos/logger/logger.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

using namespace ecos;

namespace
{

struct instance_wrapper
{
    model_instance* instance;
    std::map<std::string, double> prevOutputValues;
    std::map<std::string, double> prevPrevOutputValues;
    std::map<std::string, double> prevInputValues;  // 存储前一步输入值
    double lastTime = 0.0;
};

// 保持器类定义（保留原样，但本算法中未使用）
class ZeroOrderHold
{ /* 略 */
};
class FirstOrderHold
{ /* 略 */
};
class SecondOrderHold
{ /* 略 */
};

} // namespace

enum class HolderType
{
    None,
    ZeroOrder,
    FirstOrder,
    SecondOrder
};

class strong_coupling_algorithm::impl
{
public:
    impl(double stepSize,
        std::size_t maxIterations,
        bool parallel,
        HolderType outputHolderType = HolderType::None,
        HolderType inputHolderType = HolderType::None)
        : stepSize_(stepSize)
        , maxIterations_(maxIterations)
        , parallel_(parallel)
        , outputHolderType_(outputHolderType)
        , inputHolderType_(inputHolderType)
    {
        if (outputHolderType_ != HolderType::None || inputHolderType_ != HolderType::None) {
            log::warn("Energy‑correction algorithm may override holder settings. Use None for optimal behavior.");
        }
    }

    void model_instance_added(model_instance* instance)
    {
        log::debug("Strong coupling: model added [{}]", instance->instanceName());
        instances_.push_back(instance_wrapper{instance});
    }

    double step(double currentTime)
    {
        stepSize_ = std::max(stepSize_, minStep_);

        if (firstStep_) {
            initializeRoles();
            firstStep_ = false;
        }

        // 预积分阶段：设置输入
        preStepMode_ = true;
        for (auto& w : instances_) {
            apply_input_modifiers(w, currentTime);
        }
        preStepMode_ = false;

        for (auto& w : instances_) {
            w.instance->get_properties().apply_sets();
        }

        // 积分
        for (auto& w : instances_) {
            w.instance->step(currentTime, stepSize_);
        }

        // 读取输出
        for (auto& w : instances_) {
            w.instance->get_properties().apply_gets();
        }

        // 获取新的力输出（来自力源）和速度输出（来自速度源）
        double fNew = 0.0, vNew = 0.0;
        if (forceSource_.inst) {
            if (auto* prop = forceSource_.inst->get_properties().get_real_property(forceSource_.outputName)) {
                fNew = prop->get_value();
                //log::debug("Force source output = {}", fNew);
            }
        }
        if (velocitySource_.inst) {
            if (auto* prop = velocitySource_.inst->get_properties().get_real_property(velocitySource_.outputName)) {
                vNew = prop->get_value();
                //log::debug("Velocity source output = {}", vNew);
            }
        }

        // 接口迭代（保留原结构，不影响能量修正）
        for (std::size_t iter = 0; iter < maxIterations_; ++iter) {
            for (auto& w : instances_) {
                apply_output_holders(w, currentTime);
                apply_input_modifiers(w, currentTime);
                w.instance->get_properties().apply_sets();
                w.instance->get_properties().apply_gets();
            }
        }

        // 计算残余功率和能量误差
        double fIn = fPrev_ + fCorr_; // 当前步速度源的力输入
        double vIn = vPrev_; // 当前步力源的速度输入
        double residualPower = -fNew * vIn + vNew * fIn;
        double xi = mu_ * stepSize_ * residualPower;

        //log::debug("residualPower = {}, xi = {}", residualPower, xi);

        // 步长自适应
        if (std::abs(xi) > energyTol_) {
            stepSize_ *= 0.9;
            //log::debug("Energy error too large (|ξ|={:.2e}), reducing step to {}", std::abs(xi), stepSize_);
        } else if (std::abs(xi) < energyTol_ / 100.0) {
            stepSize_ = std::min(stepSize_ * 1.2 , maxStep_);
        }
        stepSize_ = std::max(stepSize_, minStep_);

        // 计算下一步修正力（作用于速度源的力输入）
        double fCorrNext = 0.0;
        if (std::abs(vNew) > 1e-12) {
            fCorrNext = -xi / (vNew * stepSize_);
        } else {
            fCorrNext = 0.0;
        }

        // 更新状态
        fPrev_ = fNew;
        vPrev_ = vNew;
        fCorr_ = fCorrNext;

        return currentTime + stepSize_;
    }

    void set_output_holder_type(HolderType type) { outputHolderType_ = type; }
    void set_input_holder_type(HolderType type) { inputHolderType_ = type; }

private:
    double stepSize_;
    std::size_t maxIterations_;
    bool parallel_;
    HolderType outputHolderType_;
    HolderType inputHolderType_;

    std::vector<instance_wrapper> instances_;
    double lastTime_ = -1.0; // 上次宏步时间
    double forcePrev_ = 0.0; // 力源上一步输出
    double forcePrevPrev_ = 0.0; // 力源上两步输出
    double velPrev_ = 0.0; // 速度源上一步输出
    double velPrevPrev_ = 0.0; 
    // 算法参数
    double mu_ = 0.5;
    double energyTol_ = 1e-5;
    double maxStep_ = 0.001;
    double minStep_ = 1e-12;

    double fPrev_ = 0.0; // 上一步的力源输出
    double vPrev_ = 0.0; // 上一步的速度源输出
    double fCorr_ = 0.0; // 当前步施加在速度源上的修正力
    bool firstStep_ = true;
    bool preStepMode_ = false;

    // 角色信息：基于固定变量名，不再依赖方向判断
    struct SourceInfo
    {
        model_instance* inst = nullptr;
        std::string outputName; // 输出变量名
        std::string inputName; // 输入变量名
    };
    SourceInfo forceSource_; // 力源：输出 "F"，输入 "V_input"
    SourceInfo velocitySource_; // 速度源：输出 "V"，输入 "F_input"

    void initializeRoles()
    {
        for (auto& w : instances_) {
            auto& props = w.instance->get_properties();

            // 检查是否为力源：拥有输出 "F" 和输入 "V_input"
            bool hasF = props.get_real_property("F") != nullptr;
            bool hasVInput = props.get_real_property("V_input") != nullptr;
            if (hasF && hasVInput) {
                if (!forceSource_.inst) {
                    forceSource_.inst = w.instance;
                    forceSource_.outputName = "F";
                    forceSource_.inputName = "V_input";
                    if (auto* prop = props.get_real_property("F")) {
                        fPrev_ = prop->get_value(); // 初始力输出
                    }
                    //log::debug("Force source: {}", w.instance->instanceName());
                } else {
                    //log::warn("Multiple force sources detected, using first one.");
                }
            }

            // 检查是否为速度源：拥有输出 "V" 和输入 "F_input"
            bool hasV = props.get_real_property("V") != nullptr;
            bool hasFInput = props.get_real_property("F_input") != nullptr;
            if (hasV && hasFInput) {
                if (!velocitySource_.inst) {
                    velocitySource_.inst = w.instance;
                    velocitySource_.outputName = "V";
                    velocitySource_.inputName = "F_input";
                    if (auto* prop = props.get_real_property("V")) {
                        vPrev_ = prop->get_value(); // 初始速度输出
                    }
                    //log::debug("Velocity source: {}", w.instance->instanceName());
                } else {
                    //log::warn("Multiple velocity sources detected, using first one.");
                }
            }
        }

        if (!forceSource_.inst || !velocitySource_.inst) {
            log::err("Energy‑correction algorithm requires one force source (with F output and V_input input) and one velocity source (with V output and F_input input).");
        }
    }

    void apply_output_holders(instance_wrapper&, double) { }

    void apply_input_modifiers(instance_wrapper& w, double /*currentTime*/)
    {
        if (!preStepMode_) return;

        // 设置力源的速度输入
        if (forceSource_.inst == w.instance) {
            if (auto* prop = w.instance->get_properties().get_real_property(forceSource_.inputName)) {
                double baseValue = vPrev_;
                double correctedValue = baseValue;

                // 应用一阶保持器修正
                if (inputHolderType_ == HolderType::FirstOrder) {
                    auto it = w.prevInputValues.find(forceSource_.inputName);
                    if (it != w.prevInputValues.end() && stepSize_ > 1e-12) {
                        double prevInputVal = it->second;
                        double holderCorrection = 0.45*(baseValue - prevInputVal); // / stepSize_;
                        correctedValue = baseValue + holderCorrection;
                        //log::debug("[{}] FirstOrder holder: base={}, prev={}, correction={}, final={}",
                            //w.instance->instanceName(), baseValue, prevInputVal, holderCorrection, correctedValue);
                    }
                    // 更新前一步输入值
                    w.prevInputValues[forceSource_.inputName] = baseValue;
                }

                prop->set_value(correctedValue);
                //log::debug("[{}] Setting {} to {}", w.instance->instanceName(), forceSource_.inputName, correctedValue);
            }
        }

        // 设置速度源的力输入（带能量修正和保持器修正）
        if (velocitySource_.inst == w.instance) {
            if (auto* prop = w.instance->get_properties().get_real_property(velocitySource_.inputName)) {
                double baseValue = fPrev_ + fCorr_;
                double correctedValue = baseValue;

                // 应用一阶保持器修正
                if (inputHolderType_ == HolderType::FirstOrder) {
                    auto it = w.prevInputValues.find(velocitySource_.inputName);
                    if (it != w.prevInputValues.end() && stepSize_ > 1e-12) {
                        double prevInputVal = it->second;
                        double holderCorrection = 0.45*(baseValue - prevInputVal); // / stepSize_;
                        correctedValue = baseValue + holderCorrection;
                        //log::debug("[{}] FirstOrder holder: base={}, prev={}, correction={}, final={}",
                            //w.instance->instanceName(), baseValue, prevInputVal, holderCorrection, correctedValue);
                    }
                    // 更新前一步输入值
                    w.prevInputValues[velocitySource_.inputName] = baseValue;
                }

                prop->set_value(correctedValue);
                //log::debug("[{}] Setting {} to {}", w.instance->instanceName(), velocitySource_.inputName, correctedValue);
            }
        }
    }
    void print_property_values(const instance_wrapper& w)
    {
        auto& props = w.instance->get_properties();
        const std::vector<std::string> propNames = {"F", "V", "F_input", "V_input"};
        for (const auto& name : propNames) {
            if (auto* prop = props.get_real_property(name)) {
                log::debug("[{}] {} = {}", w.instance->instanceName(), name, prop->get_value());
            }
        }
    }
};

/**************************************************************
 对外接口实现（完全不变）
 **************************************************************/

strong_coupling_algorithm::strong_coupling_algorithm(
    double stepSize,
    std::size_t maxIterations,
    bool parallel,
    int outputHolderType,
    int inputHolderType)
    : pimpl_(std::make_unique<impl>(
          stepSize,
          maxIterations,
          parallel,
          static_cast<HolderType>(outputHolderType),
          static_cast<HolderType>(inputHolderType)))
{ }

void strong_coupling_algorithm::model_instance_added(model_instance* instance)
{
    pimpl_->model_instance_added(instance);
}

double strong_coupling_algorithm::step(double currentTime)
{
    return pimpl_->step(currentTime);
}

strong_coupling_algorithm::~strong_coupling_algorithm() = default;