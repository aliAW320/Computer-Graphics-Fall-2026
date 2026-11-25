#include "SteamEquipment.h"

#include <array>
#include <cmath>

#include <citylab/CityLab.h>

namespace team25::steam {
namespace {

using namespace citylab;
using glm::vec2;
using glm::vec3;

constexpr vec3 kConcrete{0.58f, 0.57f, 0.53f};
constexpr vec3 kSteel{0.28f, 0.34f, 0.37f};
constexpr vec3 kDark{0.12f, 0.16f, 0.19f};
constexpr vec3 kSilver{0.74f, 0.78f, 0.77f};
constexpr vec3 kOrange{0.85f, 0.33f, 0.12f};
constexpr vec3 kYellow{0.96f, 0.69f, 0.20f};
constexpr vec3 kBlue{0.12f, 0.40f, 0.56f};

constexpr float kDeckHeight = 0.12f;
constexpr float kDeckWidth = 14.0f;
constexpr float kTowerSpan = 4.4f;
constexpr float kBayHeight = 2.9f;
constexpr int kBayCount = 5;
constexpr float kPlatformY = kBayHeight * kBayCount;
constexpr float kReceiverY = kPlatformY + 1.95f;
constexpr float kPipeOffset = 3.5f;
constexpr float kInletY = 2.15f;
constexpr float kInletZ = -4.55f;  // relative to the generator
constexpr vec3 kTowerFootprint{6.5f, 0.5f, 6.5f};
constexpr vec3 kGeneratorFootprint{6.0f, 0.5f, 12.8f};
constexpr vec3 kCabinetPosition{-5.2f, 0.0f, 1.5f};
constexpr vec3 kCabinetFootprint{2.2f, 2.15f, 1.0f};

struct YardLayout {
    explicit YardLayout(const PlotInfo& plot)
        : tower{-plot.width * 0.32f, kDeckHeight, -plot.depth * 0.13f},
          generator{tower.x, kDeckHeight, plot.depth * 0.05f} {}

    vec3 tower;
    vec3 generator;
};

void box(vec3 center, vec3 size, vec3 color) {
    glColor3fv(glm::value_ptr(color));
    glPushMatrix();
    glTranslatef(center.x, center.y, center.z);
    glScalef(size.x, size.y, size.z);
    shapes::box(1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void pipe(vec3 from, vec3 to, float radius, vec3 color, int slices = 16) {
    const vec3 direction = to - from;
    const float length = glm::length(direction);
    const vec3 axis = glm::cross(vec3(0.0f, 1.0f, 0.0f), direction);
    glColor3fv(glm::value_ptr(color));
    glPushMatrix();
    glTranslatef(from.x, from.y, from.z);
    if (glm::dot(axis, axis) > 0.0f) {
        const float angle = glm::degrees(std::atan2(glm::length(axis), direction.y));
        glRotatef(angle, axis.x, axis.y, axis.z);
    } else if (direction.y < 0.0f) {
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    }
    shapes::cylinder(radius, length, slices);
    glPopMatrix();
}

// Extrude a convex, counterclockwise XY outline into a closed solid along Z.
template <std::size_t N>
void drawHousing(const std::array<vec2, N>& outline, float depth) {
    const float half = depth * 0.5f;
    glBegin(GL_QUADS);
    for (std::size_t i = 0; i < N; ++i) {
        const vec2 a = outline[i];
        const vec2 b = outline[(i + 1) % N];
        const vec2 edge = glm::normalize(b - a);
        glNormal3f(edge.y, -edge.x, 0.0f);
        glVertex3f(a.x, a.y, -half);
        glVertex3f(b.x, b.y, -half);
        glVertex3f(b.x, b.y, half);
        glVertex3f(a.x, a.y, half);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (const vec2 point : outline) glVertex3f(point.x, point.y, half);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (auto it = outline.rbegin(); it != outline.rend(); ++it)
        glVertex3f(it->x, it->y, -half);
    glEnd();
}

void drawTowerBay(bool reverseBrace) {
    const float half = kTowerSpan * 0.5f;
    const float angle = glm::degrees(std::atan2(kBayHeight, kTowerSpan));
    const float direction = reverseBrace ? -1.0f : 1.0f;
    const vec3 brace{glm::length(vec2(kTowerSpan, kBayHeight)), 0.19f, 0.16f};
    for (float x : {-half, half}) {
        box({x, kBayHeight, 0.0f}, {0.23f, 0.23f, kTowerSpan}, kOrange);
    }
    for (float z : {-half, half}) {
        box({0.0f, kBayHeight, z}, {kTowerSpan, 0.23f, 0.23f}, kOrange);
        glPushMatrix();
        glTranslatef(0.0f, kBayHeight * 0.5f, z);
        glRotatef(direction * angle, 0.0f, 0.0f, 1.0f);
        box({}, brace, kSteel);
        glPopMatrix();
    }
    for (float x : {-half, half}) {
        glPushMatrix();
        glTranslatef(x, kBayHeight * 0.5f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-direction * angle, 0.0f, 0.0f, 1.0f);
        box({}, brace, kSteel);
        glPopMatrix();
    }
}

void drawPlatform() {
    constexpr float half = 3.0f;
    box({}, {6.2f, 0.32f, 6.2f}, kDark);
    for (int side = 0; side < 4; ++side) {
        // Platform frame -> guardrail frame -> rail and upright parts.
        glPushMatrix();
        glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, half);
        for (float y : {0.60f, 1.12f})
            box({0.0f, y, 0.0f}, {5.94f, 0.07f, 0.07f}, kYellow);
        for (float x : {-2.12f, 0.0f, 2.12f})
            box({x, 0.65f, 0.0f}, {0.08f, 1.0f, 0.08f}, kYellow);
        glPopMatrix();
    }
}

void drawReceiver() {
    // Twelve outline edges form a faceted casing around the exposed receiver face.
    constexpr std::array<vec2, 12> outline{{
        {-1.35f, -1.65f}, {1.35f, -1.65f}, {1.75f, -1.42f}, {1.95f, -0.95f},
        {1.95f, 0.75f}, {1.75f, 1.28f}, {1.35f, 1.65f}, {-1.35f, 1.65f},
        {-1.75f, 1.28f}, {-1.95f, 0.75f}, {-1.95f, -0.95f}, {-1.75f, -1.42f},
    }};
    glColor3fv(glm::value_ptr(kSilver));
    drawHousing(outline, 2.6f);
    box({0.0f, 0.0f, 1.34f}, {2.95f, 2.45f, 0.13f}, kDark);
    for (int tube = -7; tube <= 7; ++tube) {
        const float x = tube * 0.185f;
        pipe({x, -1.10f, 1.48f}, {x, 1.10f, 1.48f}, 0.062f, kSteel, 8);
    }
    for (float y : {-1.10f, 1.10f})
        pipe({-1.39f, y, 1.48f}, {1.39f, y, 1.48f}, 0.095f, kOrange);
    box({0.0f, 1.65f, 1.20f}, {3.25f, 0.22f, 0.55f}, kOrange);
}

void drawSteamTower() {
    const float half = kTowerSpan * 0.5f;
    box({0.0f, kTowerFootprint.y * 0.5f, 0.0f}, kTowerFootprint, kConcrete);
    for (float x : {-half, half}) {
        for (float z : {-half, half}) {
            box({x, 0.59f, z}, {0.82f, 0.18f, 0.82f}, kDark);
            box({x, kPlatformY * 0.5f, z}, {0.31f, kPlatformY, 0.31f}, kOrange);
        }
    }
    for (int bay = 0; bay < kBayCount; ++bay) {
        glPushMatrix();
        glTranslatef(0.0f, bay * kBayHeight, 0.0f);
        drawTowerBay(bay % 2 != 0);
        glPopMatrix();
    }
    glPushMatrix();
    glTranslatef(0.0f, kPlatformY, 0.0f);
    drawPlatform();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, kReceiverY, 0.0f);
    drawReceiver();
    glPopMatrix();
}

void drawTurbine() {
    glColor3fv(glm::value_ptr(kBlue));
    constexpr std::array<vec2, 8> outline{{
        {-1.15f, -1.05f}, {1.15f, -1.05f}, {1.55f, -0.60f}, {1.55f, 0.60f},
        {1.02f, 1.18f}, {-1.02f, 1.18f}, {-1.55f, 0.60f}, {-1.55f, -0.60f},
    }};
    drawHousing(outline, 3.4f);
    for (float z : {-1.72f, 1.72f})
        pipe({0.0f, 0.0f, z - 0.14f}, {0.0f, 0.0f, z + 0.14f}, 1.24f, kSilver, 24);
    pipe({1.24f, 0.0f, -0.72f}, {2.45f, 0.0f, -0.72f}, 0.32f, kSteel);
    pipe({2.27f, 0.0f, -0.72f}, {2.46f, 0.0f, -0.72f}, 0.46f, kSilver);
}

void drawGeneratorBody() {
    pipe({0.0f, 0.0f, -1.65f}, {0.0f, 0.0f, 1.65f}, 1.07f, kBlue, 24);
    for (int fin = -4; fin <= 4; ++fin) {
        const float z = fin * 0.34f;
        pipe({0.0f, 0.0f, z - 0.06f}, {0.0f, 0.0f, z + 0.06f}, 1.22f, kBlue, 24);
    }
    pipe({0.0f, 0.0f, 1.50f}, {0.0f, 0.0f, 1.80f}, 1.17f, kSilver, 24);
    pipe({0.0f, 0.0f, 1.78f}, {0.0f, 0.0f, 1.88f}, 0.83f, kDark, 24);
    for (int bar = -3; bar <= 3; ++bar) {
        const float x = bar * 0.20f;
        const float height = 2.0f * std::sqrt(0.80f * 0.80f - x * x);
        box({x, 0.0f, 1.90f}, {0.062f, height, 0.08f}, kSteel);
    }
    box({0.0f, 1.10f, 0.55f}, {0.96f, 0.62f, 1.12f}, kSteel);
    box({0.0f, 1.44f, 0.55f}, {1.05f, 0.12f, 1.20f}, kSilver);
}

void drawSwitchgear() {
    const float halfWidth = kCabinetFootprint.x * 0.5f;
    const float front = kCabinetFootprint.z * 0.5f;
    box({0.0f, 0.17f, 0.0f}, {2.4f, 0.34f, 1.2f}, kConcrete);
    box({0.0f, 1.22f, -0.42f}, {2.2f, 1.80f, 0.15f}, kSteel);
    for (float side : {-1.0f, 1.0f})
        box({side * (halfWidth - 0.05f), 1.22f, 0.0f}, {0.10f, 1.80f, 1.0f}, kSteel);
    for (float y : {0.37f, 2.08f})
        box({0.0f, y, 0.0f}, {2.2f, 0.12f, 1.0f}, kSilver);
    box({0.0f, 1.45f, -0.18f}, {1.86f, 0.73f, 0.28f}, kDark);
    for (float x : {-0.55f, 0.0f, 0.55f})
        box({x, 1.58f, -0.02f}, {0.25f, 0.10f, 0.08f}, kYellow);

    for (float side : {-1.0f, 1.0f}) {
        const float pivot = side * 0.52f;
        glPushMatrix();
        glTranslatef(side * 0.55f, 1.22f, front);
        glTranslatef(pivot, 0.0f, 0.0f);          // T(p): outer hinge in door coordinates
        glRotatef(side * 28.0f, 0.0f, 1.0f, 0.0f);  // R
        glTranslatef(-pivot, 0.0f, 0.0f);         // T(-p)
        box({}, {1.04f, 1.55f, 0.10f}, kBlue);
        box({-side * 0.38f, 0.0f, 0.09f}, {0.065f, 0.23f, 0.09f}, kSilver);
        glPopMatrix();
    }
}

void drawTurbineGenerator() {
    box({0.0f, kGeneratorFootprint.y * 0.5f, 0.0f}, kGeneratorFootprint, kConcrete);
    box({0.0f, 0.66f, 0.0f}, {4.55f, 0.32f, 11.7f}, kDark);
    for (float z : {-4.9f, -2.9f, 1.7f, 4.3f})
        box({0.0f, 0.96f, z}, {3.8f, 0.48f, 0.52f}, kSteel);

    glPushMatrix();
    glTranslatef(0.0f, kInletY, kInletZ + 0.72f);
    drawTurbine();
    glPopMatrix();
    pipe({0.0f, kInletY, -2.3f}, {0.0f, kInletY, 1.42f}, 0.40f, kSilver);
    pipe({0.0f, kInletY, -1.80f}, {0.0f, kInletY, 0.95f}, 0.64f, kYellow);

    glPushMatrix();
    glTranslatef(0.0f, kInletY, 3.0f);
    drawGeneratorBody();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(kCabinetPosition.x, kCabinetPosition.y, kCabinetPosition.z);
    drawSwitchgear();
    glPopMatrix();

    // The generator terminal feeds the yard switchgear beside the skid.
    const float terminalY = kInletY + 1.44f;
    const float terminalZ = 3.55f;
    constexpr float riserX = -2.45f;
    constexpr float conduitY = 0.57f;
    pipe({-0.48f, terminalY, terminalZ}, {riserX, terminalY, terminalZ}, 0.085f, kOrange);
    pipe({riserX, terminalY, terminalZ}, {riserX, conduitY, terminalZ}, 0.085f, kOrange);
    pipe({riserX, conduitY, terminalZ}, {riserX, conduitY, kCabinetPosition.z}, 0.085f, kOrange);
    pipe({riserX, conduitY, kCabinetPosition.z},
         {kCabinetPosition.x + 1.07f, conduitY, kCabinetPosition.z}, 0.085f, kOrange);
}

void drawSteamLine(const YardLayout& yard) {
    const float x = yard.tower.x + kPipeOffset;
    const float top = yard.tower.y + kReceiverY - 0.45f;
    const float bottom = yard.tower.y + kInletY;
    const float end = yard.generator.z + kInletZ;
    const std::array<vec3, 5> route{{
        {yard.tower.x + 1.6f, top, yard.tower.z},
        {x, top, yard.tower.z},
        {x, bottom, yard.tower.z},
        {x, bottom, end},
        {yard.generator.x + 2.38f, bottom, end},
    }};
    constexpr float radius = 0.25f;
    for (std::size_t i = 0; i + 1 < route.size(); ++i)
        pipe(route[i], route[i + 1], radius, kSilver);
    for (std::size_t i = 1; i + 1 < route.size(); ++i) {
        const vec3 elbow = route[i];
        glPushMatrix();
        glTranslatef(elbow.x, elbow.y, elbow.z);
        glColor3fv(glm::value_ptr(kSilver));
        shapes::sphere(radius, 16, 8);
        glPopMatrix();
    }
    for (int support = 1; support <= 3; ++support) {
        const float z = glm::mix(yard.tower.z, end, support * 0.25f);
        box({x, yard.tower.y + 0.10f, z}, {0.82f, 0.20f, 0.82f}, kConcrete);
        box({x, yard.tower.y + 0.95f, z}, {0.23f, 1.85f, 0.24f}, kSteel);
        pipe({x, bottom, z - 0.11f}, {x, bottom, z + 0.11f}, 0.32f, kOrange);
    }
}

void addFootprint(Context& ctx, vec3 origin, vec3 size, float height, float frontExtra = 0.0f) {
    ctx.addCollider(origin + vec3(-size.x * 0.5f, 0.0f, -size.z * 0.5f),
                    origin + vec3(size.x * 0.5f, height, size.z * 0.5f + frontExtra));
}

}  // namespace

void draw(const PlotInfo& plot) {
    const YardLayout yard(plot);
    const float length = yard.generator.z - yard.tower.z + 14.0f;
    const float center = (yard.generator.z + yard.tower.z) * 0.5f;
    box({yard.tower.x, kDeckHeight * 0.5f, center}, {kDeckWidth, kDeckHeight, length}, kConcrete);

    for (int segment = -4; segment <= 4; ++segment) {
        const float z = center + segment * (length / 10.0f);
        box({visitorEdgeX(plot) - 0.45f, kDeckHeight + 0.006f, z},
            {0.12f, 0.012f, 2.1f}, kYellow);
    }

    glPushMatrix();
    glTranslatef(yard.tower.x, yard.tower.y, yard.tower.z);
    drawSteamTower();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(yard.generator.x, yard.generator.y, yard.generator.z);
    drawTurbineGenerator();
    glPopMatrix();
    drawSteamLine(yard);
}

float visitorEdgeX(const PlotInfo& plot) {
    return YardLayout(plot).tower.x + kDeckWidth * 0.5f;
}

glm::vec3 switchgearPosition(const PlotInfo& plot) {
    return YardLayout(plot).generator + kCabinetPosition;
}

void addColliders(Context& ctx) {
    const YardLayout yard(ctx.plot());
    addFootprint(ctx, yard.tower, kTowerFootprint, 3.0f);
    addFootprint(ctx, yard.generator, kGeneratorFootprint, 3.7f);
    addFootprint(ctx, yard.generator + kCabinetPosition, kCabinetFootprint, 2.15f, 0.55f);
    const float pipeX = yard.tower.x + kPipeOffset;
    const float end = yard.generator.z + kInletZ;
    ctx.addCollider({pipeX - 0.29f, yard.tower.y, yard.tower.z - 0.29f},
                    {pipeX + 0.29f, yard.tower.y + kReceiverY, yard.tower.z + 0.29f});
    for (int support = 1; support <= 3; ++support) {
        const float z = glm::mix(yard.tower.z, end, support * 0.25f);
        addFootprint(ctx, {pipeX, yard.tower.y, z}, {0.82f, 0.20f, 0.82f}, kInletY);
    }
}

}  // namespace team25::steam
