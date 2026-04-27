
#ifndef ECOS_FMI_FMI3_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_FMI3_MODEL_DESCRIPTION_HPP

#include "fmilibcpp/model_description.hpp"

#include <fmi4c.h>

namespace nova_fmi
{

model_description create_fmi3_model_description(fmuHandle* handle);

} // namespace nova_fmi

#endif // ECOS_FMI_FMI3_MODEL_DESCRIPTION_HPP
