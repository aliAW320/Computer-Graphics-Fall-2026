#pragma once

#include <glm/vec3.hpp>

namespace citylab {
class Context;
struct PlotInfo;
}

namespace team25::steam {

// The equipment yard sits between the buildings on the west side.
void draw(const citylab::PlotInfo& plot);
void addColliders(citylab::Context& ctx);
float visitorEdgeX(const citylab::PlotInfo& plot);
glm::vec3 switchgearPosition(const citylab::PlotInfo& plot);

}  // namespace team25::steam
