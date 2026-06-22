
#include "nova/components/recorder/engine_observer.hpp"

#include "nova/engine/nova_engine.hpp"

namespace nova_sim
{

void engine_observer::pre_init(nova_engine& sim) { }
void engine_observer::post_init(nova_engine& sim) { }

void engine_observer::pre_step(nova_engine& sim) { }
void engine_observer::post_step(nova_engine& sim) { }

void engine_observer::post_terminate(nova_engine& sim) { }

void engine_observer::on_reset() { }

} // namespace nova_sim
