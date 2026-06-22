
#ifndef NOVA_ENGINE_OBSERVER_HPP
#define NOVA_ENGINE_OBSERVER_HPP

#include <functional>
#include <utility>

namespace nova_sim
{

class nova_engine;

struct engine_observer
{

    virtual void pre_init(nova_engine& sim);
    virtual void post_init(nova_engine& sim);

    virtual void pre_step(nova_engine& sim);
    virtual void post_step(nova_engine& sim);

    virtual void post_terminate(nova_engine& sim);

    virtual void on_reset();

    virtual ~engine_observer() = default;
};

struct post_terminate_hook : engine_observer
{

    explicit post_terminate_hook(std::function<void(nova_engine& sim)> hook)
        : hook_(std::move(hook))
    { }

    explicit post_terminate_hook(const std::function<void()>& hook)
        : hook_([hook](nova_engine&) { hook(); })
    { }

    void post_terminate(nova_engine& sim) override
    {
        hook_(sim);
    }

private:
    std::function<void(nova_engine& sim)> hook_;
};

} // namespace nova_sim

#endif // NOVA_ENGINE_OBSERVER_HPP
