#pragma once

#include <glm/vec3.hpp>

namespace citylab {
class Context;
struct PlotInfo;
}

namespace team25::steam {

// The compact equipment yard sits behind the central electrical court.
void draw(const citylab::PlotInfo& plot);
void addColliders(citylab::Context& ctx);
glm::vec3 switchgearPosition(const citylab::PlotInfo& plot);

}  // namespace team25::steam
