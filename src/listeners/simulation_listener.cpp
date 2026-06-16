
#include "nova/listeners/simulation_listener.hpp"

#include "nova/simulation.hpp"

namespace nova_sim
{

void simulation_listener::pre_init(simulation& sim) { }
void simulation_listener::post_init(simulation& sim) { }

void simulation_listener::pre_step(simulation& sim) { }
void simulation_listener::post_step(simulation& sim) { }

void simulation_listener::post_terminate(simulation& sim) { }

void simulation_listener::on_reset() { }

} // namespace nova_sim
