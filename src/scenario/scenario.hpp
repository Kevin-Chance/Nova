#ifndef LIBNOVA_SCENARIO_HPP
#define LIBNOVA_SCENARIO_HPP

#include "nova/model_instance.hpp"
#include <algorithm>
#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <filesystem>

namespace nova_sim
{

class predicate_action
{
public:
    predicate_action(std::function<bool()> pred, std::function<void()> f)
        : f_(std::move(f)), pred_(std::move(pred)) { }
    bool invoke() {
        if (pred_()) { f_(); return true; }
        return false;
    }
private:
    std::function<bool()> pred_;
    std::function<void()> f_;
};

class timed_action
{
public:
    timed_action(double timePoint, std::function<void()> f, const std::optional<double>& eps = std::nullopt)
        : eps_(eps.value_or(0)), timePoint_(timePoint), f_(std::move(f)) { }
    [[nodiscard]] double eps() const { return eps_; }
    [[nodiscard]] double time_point() const { return timePoint_; }
    void invoke() { f_(); }
    bool operator<(const timed_action& t) const { return timePoint_ > t.timePoint_; }
private:
    double eps_;
    double timePoint_;
    std::function<void()> f_;
};

class scenario
{
public:
    void runInitActions();
    void apply(double t);
    void on_init(std::function<void()> f);
    void invoke_when(predicate_action pa);
    void invoke_at(timed_action ta);
    void reset();

    // Week 6: Scenario intervention using linear search
    void load(const std::filesystem::path& config, std::vector<std::unique_ptr<model_instance>>& instances);

private:
    bool active_{false};
    std::vector<std::function<void()>> initActions;
    std::vector<timed_action> timedActions;
    std::vector<predicate_action> predicateActions;
    std::vector<timed_action> discardedTimedActions;
    std::vector<predicate_action> discardedPredicateActions;
    std::vector<timed_action> timedActionsQueue_;
};

} // namespace nova_sim

#endif // LIBNOVA_SCENARIO_HPP
