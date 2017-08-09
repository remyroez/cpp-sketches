
#ifndef ENTITY_COMPONENT_SYSTEM_ENTITY_HPP_
#define ENTITY_COMPONENT_SYSTEM_ENTITY_HPP_

#include <limits>

namespace entity_component_system {

using entity_id = unsigned int;

enum const_entity_id : entity_id {
	invalid_entity_id = std::numeric_limits<entity_id>::max()
};

} // namespace entity_component_system

#endif // ENTITY_COMPONENT_SYSTEM_ENTITY_HPP_
