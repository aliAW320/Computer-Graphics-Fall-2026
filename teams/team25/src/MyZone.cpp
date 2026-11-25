#include <citylab/CityLab.h>

#include <array>
#include <cmath>
#include <string_view>

#include "SteamEquipment.h"

namespace team25 {
namespace {

using citylab::Context;
using citylab::PlotInfo;
using citylab::Zone;
namespace shapes = citylab::shapes;

struct Color {
    float r;
    float g;
    float b;
};

constexpr Color kGround{0.40f, 0.51f, 0.28f};
constexpr Color kReserve{0.49f, 0.57f, 0.34f};
constexpr Color kRoof{0.22f, 0.41f, 0.50f};
constexpr Color kGravel{0.56f, 0.51f, 0.39f};
constexpr Color kPlanting{0.37f, 0.46f, 0.30f};
constexpr Color kGrassLight{0.47f, 0.54f, 0.31f};
constexpr Color kPaving{0.56f, 0.57f, 0.53f};
constexpr Color kDarkPaving{0.32f, 0.37f, 0.36f};
constexpr Color kIvory{0.91f, 0.89f, 0.78f};
constexpr Color kSteel{0.29f, 0.39f, 0.41f};
constexpr Color kInk{0.08f, 0.16f, 0.21f};
constexpr Color kGlass{0.22f, 0.48f, 0.55f};
constexpr Color kTeal{0.10f, 0.46f, 0.45f};
constexpr Color kAmber{1.00f, 0.66f, 0.19f};
constexpr Color kPanelBlue{0.11f, 0.27f, 0.45f};
constexpr Color kPanelLight{0.18f, 0.37f, 0.57f};
constexpr Color kConduit{0.77f, 0.37f, 0.13f};

constexpr float kEntryDepth = 13.0f;
constexpr float kPanelHeight = 1.55f;
constexpr float kPanelThickness = 0.14f;
constexpr float kPanelHalfDepth = 1.5f;
constexpr float kTilt = 30.0f;
constexpr float kSquareEdge = 2.0f;
constexpr int kPanelColumns = 6;
constexpr int kPanelRows = 4;
constexpr int kRackRows = 4;
constexpr int kRackColumns = 5;
constexpr float kPostHeight = 5.1f;
constexpr float kFrontPathWidth = 5.0f;

struct Site {
    explicit Site(const PlotInfo& plot)
        : halfWidth(plot.width / 2.0f), halfDepth(plot.depth / 2.0f),
          control{-0.39f * halfWidth, 0.0f, 0.51f * halfDepth},
          inverter{-0.19f * halfWidth, 0.0f, -0.09f * halfDepth},
          workshop{0.39f * halfWidth, 0.0f, 0.51f * halfDepth},
          transformer{0.19f * halfWidth, 0.0f, -0.09f * halfDepth},
          canopy{0.0f, 0.0f, halfDepth - kEntryDepth} {}

    glm::vec3 panel(int bank, int column, int row) const {
        return {bank * halfWidth * (0.4175f + 0.1175f * column), kPanelHeight,
                halfDepth * (0.11f - 0.13f * row)};
    }

    float rowCableZ(int row) const { return panel(1, 0, row).z - 2.15f; }

    glm::vec3 fieldCabinet(int bank, int row) const {
        return {bank * halfWidth * 0.345f, 0.0f, rowCableZ(row)};
    }

    glm::vec3 booth() const { return {7.25f, 0.0f, halfDepth - 3.9f}; }

    float halfWidth;
    float halfDepth;
    glm::vec3 control;
    glm::vec3 inverter;
    glm::vec3 workshop;
    glm::vec3 transformer;
    glm::vec3 canopy;
};

void color(Color c) {
    glColor3f(c.r, c.g, c.b);
}

void move(glm::vec3 p) {
    glTranslatef(p.x, p.y, p.z);
}

void box(Color c, glm::vec3 center, glm::vec3 size) {
    glPushMatrix();
    move(center);
    glScalef(size.x, size.y, size.z);
    color(c);
    shapes::box(1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void ground(Color c, float x, float z, float width, float depth, float height) {
    glPushMatrix();
    move({x, height, z});
    color(c);
    shapes::plane(width, depth);
    glPopMatrix();
}

void path(Color c, float x1, float z1, float x2, float z2, float width) {
    glPushMatrix();
    move({(x1 + x2) / 2.0f, 0.035f, (z1 + z2) / 2.0f});
    constexpr float degrees = 57.2957795f;
    glRotatef(std::atan2(x2 - x1, z2 - z1) * degrees, 0.0f, 1.0f, 0.0f);
    color(c);
    shapes::plane(width, std::hypot(x2 - x1, z2 - z1));
    glPopMatrix();
}

// Closed eight-sided panel shell: eight rim faces, top and bottom.
void panelShell() {
    constexpr std::array<glm::vec2, 8> outline{{
        {-kSquareEdge, -kPanelHalfDepth}, {kSquareEdge, -kPanelHalfDepth},
        {2.2f, -1.3f}, {2.2f, 1.3f},
        {kSquareEdge, kPanelHalfDepth}, {-kSquareEdge, kPanelHalfDepth},
        {-2.2f, 1.3f}, {-2.2f, -1.3f},
    }};
    color(kInk);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    for (auto corner = outline.rbegin(); corner != outline.rend(); ++corner) {
        glVertex3f(corner->x, kPanelThickness, corner->y);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    for (const auto& corner : outline) {
        glVertex3f(corner.x, 0.0f, corner.y);
    }
    glEnd();

    color(kSteel);
    glBegin(GL_QUADS);
    for (std::size_t i = 0; i < outline.size(); ++i) {
        const auto a = outline[i];
        const auto b = outline[(i + 1) % outline.size()];
        glVertex3f(a.x, 0.0f, a.y);
        glVertex3f(a.x, kPanelThickness, a.y);
        glVertex3f(b.x, kPanelThickness, b.y);
        glVertex3f(b.x, 0.0f, b.y);
    }
    glEnd();
}

// Closed tapered roof slab, rising toward the street-facing edge.
void pitchedRoof(float width, float depth, float rise) {
    const float x = width / 2.0f;
    const float z = depth / 2.0f;
    constexpr float backHeight = 0.20f;
    const float frontHeight = backHeight + rise;
    color(kRoof);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    // Top and bottom.
    glVertex3f(-x, backHeight, -z); glVertex3f(-x, frontHeight, z);
    glVertex3f(x, frontHeight, z); glVertex3f(x, backHeight, -z);
    glVertex3f(-x, 0.0f, -z); glVertex3f(x, 0.0f, -z);
    glVertex3f(x, 0.0f, z); glVertex3f(-x, 0.0f, z);
    // Back and front.
    glVertex3f(-x, 0.0f, -z); glVertex3f(-x, backHeight, -z);
    glVertex3f(x, backHeight, -z); glVertex3f(x, 0.0f, -z);
    glVertex3f(-x, 0.0f, z); glVertex3f(x, 0.0f, z);
    glVertex3f(x, frontHeight, z); glVertex3f(-x, frontHeight, z);
    // Left and right.
    glVertex3f(-x, 0.0f, -z); glVertex3f(-x, 0.0f, z);
    glVertex3f(-x, frontHeight, z); glVertex3f(-x, backHeight, -z);
    glVertex3f(x, 0.0f, -z); glVertex3f(x, backHeight, -z);
    glVertex3f(x, frontHeight, z); glVertex3f(x, 0.0f, z);
    glEnd();
}

void solarModule() {
    panelShell();
    constexpr float gap = 0.045f;
    constexpr float cellWidth = 0.64f;
    constexpr float cellDepth = 0.64f;
    constexpr float height = kPanelThickness + 0.005f;
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    for (int row = 0; row < kPanelRows; ++row) {
        for (int column = 0; column < kPanelColumns; ++column) {
            const float x = (column - (kPanelColumns - 1) / 2.0f) * (cellWidth + gap);
            const float z = (row - (kPanelRows - 1) / 2.0f) * (cellDepth + gap);
            const float x0 = x - cellWidth / 2.0f;
            const float x1 = x + cellWidth / 2.0f;
            const float z0 = z - cellDepth / 2.0f;
            const float z1 = z + cellDepth / 2.0f;
            color((row + column) % 3 == 0 ? kPanelLight : kPanelBlue);
            glVertex3f(x0, height, z0);
            glVertex3f(x0, height, z1);
            glVertex3f(x1, height, z1);
            glVertex3f(x1, height, z0);
        }
    }
    glEnd();
    ground(kGlass, 0.0f, 0.0f, 3.97f, 0.014f, kPanelThickness + 0.009f);
}

void glazedWindow() {
    box(kInk, {}, {2.45f, 1.60f, 0.16f});
    box(kGlass, {0.0f, 0.0f, 0.11f}, {2.23f, 1.29f, 0.08f});
    box(kSteel, {0.36f, 0.0f, 0.17f}, {0.08f, 1.37f, 0.05f});
    box(kIvory, {0.0f, -0.83f, 0.10f}, {2.58f, 0.13f, 0.38f});
}

void switchgear() {
    box(kInk, {0.0f, 0.15f, 0.0f}, {1.65f, 0.30f, 0.99f});
    box(kSteel, {0.0f, 0.99f, 0.0f}, {1.45f, 1.65f, 0.82f});
    box(kTeal, {0.0f, 1.02f, 0.44f}, {1.25f, 1.35f, 0.11f});
    box(kAmber, {0.0f, 1.46f, 0.52f}, {0.24f, 0.17f, 0.05f});
    for (int slat = 0; slat < 3; ++slat) {
        box(kInk, {0.08f, 0.7f + 0.11f * slat, 0.52f}, {0.71f, 0.035f, 0.038f});
    }
}

void bollard(float x, float z, float height) {
    glPushMatrix();
    move({x, 0.0f, z});
    color(kInk);
    glScalef(1.0f, height, 1.0f);
    shapes::cylinder(0.12f, 0.91f, 8);
    move({0.0f, 0.72f, 0.0f});
    color(kAmber);
    shapes::cylinder(0.124f, 0.14f, 8);
    glPopMatrix();
}

void tree(float x, float z, float scale) {
    glPushMatrix();
    move({x, 0.0f, z});
    glScalef(scale, scale, scale);
    color(kSteel);
    shapes::cylinder(0.15f, 2.15f, 7);
    constexpr std::array<glm::vec3, 3> crowns{{
        {-0.45f, 2.43f, 0.0f}, {0.38f, 2.65f, 0.13f}, {0.02f, 2.62f, -0.55f},
    }};
    for (const auto& crown : crowns) {
        glPushMatrix();
        move(crown);
        glScalef(1.16f, 0.55f, 0.88f);
        color(crown.x < 0.0f ? kGrassLight : kPlanting);
        shapes::sphere(0.93f, 7, 4);
        glPopMatrix();
    }
    glPopMatrix();
}

void dashLine(float x1, float z1, float x2, float z2) {
    const float length = std::hypot(x2 - x1, z2 - z1);
    const int count = static_cast<int>(std::ceil(length / 1.7f));
    for (int i = 0; i < count; ++i) {
        const float a = static_cast<float>(i) / count;
        const float b = (i + 0.67f) / count;
        const float xa = glm::mix(x1, x2, a);
        const float za = glm::mix(z1, z2, a);
        const float xb = glm::mix(x1, x2, b);
        const float zb = glm::mix(z1, z2, b);
        ground(kIvory, (xa + xb) / 2.0f, (za + zb) / 2.0f,
               std::abs(xb - xa) + 0.10f, std::abs(zb - za) + 0.10f, 0.048f);
    }
}

void reserve(float x, float z, float width, float depth) {
    ground(kReserve, x, z, width, depth, 0.020f);
    const float left = x - width / 2.0f;
    const float right = x + width / 2.0f;
    const float back = z - depth / 2.0f;
    const float front = z + depth / 2.0f;
    dashLine(left, back, right, back);
    dashLine(left, front, right, front);
    dashLine(left, back, left, front);
    dashLine(right, back, right, front);
}

void drawGrounds(const Site& site) {
    const float w = site.halfWidth;
    const float d = site.halfDepth;
    ground(kGround, 0.0f, 0.0f, 2.0f * w, 2.0f * d, 0.004f);
    for (int side : {-1, 1}) {
        const float fieldX = side * 0.64f * w;
        ground(kGravel, fieldX, -0.105f * d, 0.63f * w, 0.54f * d, 0.012f);
        for (int row = 0; row < kRackRows; ++row) {
            ground(kDarkPaving, fieldX, site.panel(side, 0, row).z, 0.59f * w, 4.0f, 0.026f);
        }
        ground(kPlanting, side * 0.969f * w, 0.0f, 0.043f * w, 1.94f * d, 0.018f);
        reserve(side * 0.572f * w, -0.715f * d, 0.72f * w, 0.29f * d);
        reserve(side * 0.855f * w, 0.537f * d, 0.225f * w, 0.30f * d);

        const float loopX = side * 0.72f * w;
        path(kPaving, loopX, 0.23f * d, loopX, 0.80f * d, 2.8f);
        path(kPaving, 0.0f, 0.80f * d, loopX, 0.80f * d, 2.9f);
        path(kPaving, 0.0f, 0.23f * d, side * 0.95f * w, 0.23f * d, 3.2f);
        tree(side * 0.32f * w, 0.755f * d, 1.0f);
        tree(side * 0.61f * w, 0.73f * d, 0.89f);
        tree(side * 0.94f * w, 0.88f * d, 1.0f);
        tree(side * 0.94f * w, -0.45f * d, 1.0f);
        tree(side * 0.29f * w, -0.45f * d, 0.92f);
        tree(side * 0.76f * w, -0.92f * d, 0.91f);
        tree(side * 0.35f * w, -0.92f * d, 0.87f);
    }

    reserve(0.0f, -0.755f * d, 0.32f * w, 0.21f * d);
    ground(kGravel, 0.0f, site.canopy.z, 15.0f, 7.0f, 0.019f);
    ground(kDarkPaving, site.control.x, site.control.z, 21.0f, 16.8f, 0.022f);
    ground(kDarkPaving, site.workshop.x, site.workshop.z, 21.0f, 16.8f, 0.022f);
    ground(kPaving, 0.0f, site.inverter.z, 0.645f * w, 0.265f * d, 0.022f);
    path(kPaving, 0.0f, -0.26f * d, 0.0f, d, kFrontPathWidth);
    path(kPaving, site.control.x - 1.3f, site.control.z + 7.8f, site.control.x - 1.3f, 0.80f * d, 3.3f);
    path(kPaving, site.workshop.x, site.workshop.z + 6.7f, site.workshop.x, 0.80f * d, 4.3f);
    for (float x : {-3.0f, 3.0f}) bollard(x, d - 2.65f, 0.90f);
}

void controlFloor(int level) {
    constexpr float floorHeight = 4.1f;
    box(kIvory, {0.0f, 2.12f, 0.0f}, {17.8f, 4.0f, 13.2f});
    box(kTeal, {0.0f, 3.84f, 0.2f}, {18.3f, 0.30f, 13.5f});
    for (int column = 0; column < 6; ++column) {
        if (level == 0 && column == 2) continue;
        glPushMatrix();
        move({(column - 2.5f) * 2.63f, 2.42f, 6.65f});
        glazedWindow();
        glPopMatrix();
    }
    for (int side : {-1, 1}) {
        for (int column = 0; column < 3; ++column) {
            glPushMatrix();
            move({side * 8.94f, 2.42f, (column - 1) * 3.65f});
            glRotatef(side * 90.0f, 0.0f, 1.0f, 0.0f);
            glazedWindow();
            glPopMatrix();
        }
    }
    box(kSteel, {0.0f, floorHeight, 0.0f}, {18.5f, 0.17f, 13.65f});
}

void drawControlBuilding(glm::vec3 position) {
    glPushMatrix();
    move(position);
    box(kSteel, {0.0f, 0.16f, 0.4f}, {19.2f, 0.32f, 15.1f});
    for (int level = 0; level < 2; ++level) {
        glPushMatrix();
        move({0.0f, 4.1f * level, 0.0f});
        controlFloor(level);
        glPopMatrix();
    }
    box(kInk, {-1.3f, 1.36f, 6.66f}, {2.48f, 2.47f, 0.22f});
    box(kGlass, {-1.3f, 1.36f, 6.80f}, {2.11f, 2.15f, 0.13f});
    box(kAmber, {-1.3f, 3.17f, 7.37f}, {4.3f, 0.25f, 1.7f});
    box(kSteel, {-3.4f, 1.75f, 7.90f}, {0.19f, 2.85f, 0.19f});
    box(kSteel, {0.8f, 1.75f, 7.90f}, {0.19f, 2.85f, 0.19f});
    box(kRoof, {0.0f, 8.24f, 0.0f}, {19.4f, 0.19f, 14.0f});
    box(kTeal, {4.5f, 8.38f, -2.4f}, {5.8f, 0.39f, 5.5f});
    box(kSteel, {4.5f, 9.15f, -2.4f}, {0.17f, 1.55f, 0.17f});
    box(kAmber, {4.5f, 9.94f, -2.4f}, {0.27f, 0.22f, 0.27f});
    glPopMatrix();
}

void drawInverterHall(glm::vec3 position) {
    glPushMatrix();
    move(position);
    glScalef(0.56f, 0.86f, 0.73f);
    box(kSteel, {0.0f, 0.16f, 0.0f}, {19.6f, 0.32f, 13.1f});
    box(kIvory, {0.0f, 2.54f, 0.0f}, {17.5f, 4.8f, 11.4f});
    box(kTeal, {0.0f, 4.47f, 5.78f}, {17.7f, 0.45f, 0.27f});
    box(kInk, {0.0f, 1.76f, 5.80f}, {3.8f, 3.2f, 0.22f});
    box(kSteel, {0.0f, 1.76f, 5.95f}, {3.47f, 2.92f, 0.14f});
    for (int side : {-1, 1}) {
        box(kAmber, {side * 2.18f, 1.95f, 5.9f}, {0.24f, 3.59f, 0.30f});
        for (int louver = 0; louver < 7; ++louver) {
            box(kInk, {side * 5.63f, 1.92f + 0.23f * louver, 5.85f},
                {3.59f, 0.085f, 0.23f});
        }
    }
    glPushMatrix();
    move({0.0f, 4.93f, 0.0f});
    pitchedRoof(18.9f, 12.55f, 1.26f);
    glPopMatrix();
    glPopMatrix();
}

void openWorkshopDoor() {
    constexpr float halfWidth = 1.32f;
    constexpr float doorAngle = -34.0f;
    glPushMatrix();
    move({-9.68f, 1.95f, 1.5f});
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-halfWidth, 0.0f, 0.0f);  // T(p)
    glRotatef(doorAngle, 0.0f, 1.0f, 0.0f);  // R
    glTranslatef(halfWidth, 0.0f, 0.0f);  // T(-p)
    box(kTeal, {}, {2.0f * halfWidth, 3.42f, 0.19f});
    box(kAmber, {0.92f, 0.0f, 0.16f}, {0.13f, 0.37f, 0.17f});
    glPopMatrix();
}

void drawWorkshop(glm::vec3 position) {
    glPushMatrix();
    move(position);
    box(kSteel, {0.0f, 0.15f, 0.0f}, {20.1f, 0.30f, 14.5f});
    box(kIvory, {0.0f, 2.39f, 0.0f}, {19.0f, 4.55f, 13.0f});
    box(kInk, {0.0f, 1.94f, 6.53f}, {6.0f, 3.55f, 0.12f});
    box(kSteel, {0.0f, 1.94f, 6.64f}, {5.63f, 3.31f, 0.14f});
    box(kTeal, {0.0f, 4.43f, 6.57f}, {19.1f, 0.34f, 0.20f});
    for (int slat = 0; slat < 8; ++slat) {
        box(kInk, {0.0f, 0.58f + 0.36f * slat, 6.73f}, {5.58f, 0.039f, 0.05f});
    }
    for (float x : {-3.15f, 3.15f}) {
        box(kAmber, {x, 1.96f, 6.69f}, {0.24f, 3.57f, 0.29f});
    }
    box(kInk, {-9.51f, 1.95f, 1.5f}, {0.19f, 3.46f, 2.75f});
    openWorkshopDoor();
    for (float x : {-6.05f, 6.05f}) {
        box(kGlass, {x, 2.57f, 6.59f}, {2.55f, 1.32f, 0.19f});
        box(kSteel, {x, 1.86f, 6.64f}, {2.68f, 0.12f, 0.31f});
    }
    glPushMatrix();
    move({0.0f, 4.66f, 0.0f});
    pitchedRoof(20.15f, 14.0f, 1.52f);
    glPopMatrix();
    glPopMatrix();
}

void tiltPanel() {
    glTranslatef(0.0f, 0.0f, kPanelHalfDepth);  // T(p): lower edge
    glRotatef(kTilt, 1.0f, 0.0f, 0.0f);  // R
    glTranslatef(0.0f, 0.0f, -kPanelHalfDepth);  // T(-p)
}

void drawPanelRack(glm::vec3 origin, float yaw, float widthScale) {
    glPushMatrix();
    move(origin);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    for (int side : {-1, 1}) {
        const float x = side * 1.56f;
        box(kSteel, {x, -1.43f, 0.0f}, {0.40f, 0.24f, 2.32f});
        box(kInk, {x, -0.55f, 1.02f}, {0.16f, 1.83f, 0.17f});
        box(kInk, {x, -0.02f, -0.82f}, {0.16f, 2.68f, 0.17f});
    }
    tiltPanel();
    for (float z : {-1.18f, 0.95f}) {
        box(kSteel, {0.0f, -0.082f, z}, {3.62f, 0.17f, 0.24f});
    }
    glScalef(widthScale, 1.0f, 1.0f);
    solarModule();
    glPopMatrix();
}

void drawSolarField(const Site& site) {
    for (int bank : {-1, 1}) {
        for (int row = 0; row < kRackRows; ++row) {
            for (int column = 0; column < kRackColumns; ++column) {
                const float yaw = column == kRackColumns - 1 ? bank * 5.0f : 0.0f;
                const float widthScale = row == kRackRows - 1 ? 0.93f : 1.0f;
                drawPanelRack(site.panel(bank, column, row), yaw, widthScale);
            }
            glPushMatrix();
            move(site.fieldCabinet(bank, row));
            glRotatef(bank * 90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(0.74f, row % 2 == 0 ? 0.75f : 0.82f, 0.75f);
            switchgear();
            glPopMatrix();
        }
    }
}

void drawEntryCanopy(glm::vec3 position) {
    glPushMatrix();
    move(position);
    for (int side : {-1, 1}) {
        const float x = side * 5.8f;
        for (float z : {-2.3f, 2.3f}) {
            box(kSteel, {x, 0.15f, z}, {0.85f, 0.30f, 0.85f});
            box(kInk, {x, kPostHeight / 2.0f, z}, {0.31f, kPostHeight, 0.31f});
            box(kAmber, {x, 3.85f, z}, {0.36f, 0.45f, 0.36f});
        }
        box(kSteel, {x, kPostHeight, 0.0f}, {0.38f, 0.46f, 5.55f});
    }
    box(kTeal, {0.0f, kPostHeight, 2.50f}, {13.45f, 0.42f, 0.35f});
    box(kAmber, {0.0f, kPostHeight + 0.13f, 2.71f}, {12.91f, 0.075f, 0.12f});
    box(kSteel, {0.0f, kPostHeight, -2.5f}, {13.45f, 0.35f, 0.31f});
    for (int i = 0; i < 3; ++i) {
        glPushMatrix();
        move({(i - 1) * 4.40f, kPostHeight + 0.18f, 0.0f});
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
        glScalef(0.99f, 1.0f, 1.59f);
        solarModule();
        glPopMatrix();
    }
    glPopMatrix();
}

void drawTransformerPad(glm::vec3 position) {
    glPushMatrix();
    move(position);
    box(kSteel, {0.0f, 0.16f, 0.0f}, {11.6f, 0.32f, 9.4f});
    box(kPaving, {0.0f, 0.34f, 0.0f}, {10.7f, 0.16f, 8.5f});
    box(kInk, {-2.70f, 0.62f, 0.0f}, {4.7f, 0.38f, 4.35f});
    box(kTeal, {-2.70f, 2.0f, 0.0f}, {3.9f, 2.6f, 3.7f});
    for (int fin = 0; fin < 9; ++fin) {
        box(kRoof, {-4.40f + fin * 0.42f, 2.06f, 1.94f}, {0.17f, 2.38f, 0.43f});
    }
    box(kGlass, {-2.70f, 3.34f, 0.0f}, {4.11f, 0.21f, 3.96f});
    for (float x : {-3.8f, -2.7f, -1.6f}) {
        box(kIvory, {x, 3.73f, -0.5f}, {0.28f, 0.72f, 0.28f});
        box(kAmber, {x, 4.10f, -0.5f}, {0.36f, 0.15f, 0.36f});
    }
    for (int i = 0; i < 3; ++i) {
        glPushMatrix();
        move({0.60f + i * 1.71f, 0.42f, 0.0f});
        glScalef(0.92f, 1.43f, 1.86f);
        switchgear();
        glPopMatrix();
    }
    glPopMatrix();
}

void drawVisitorFurniture(const Site& site) {
    for (float x : {-4.1f, 4.1f}) {
        glPushMatrix();
        move(site.canopy + glm::vec3(x, 0.023f, -1.95f));
        citylab::props::bench();
        glPopMatrix();
    }
    const auto kiosk = site.canopy + glm::vec3(-3.9f, 0.0f, 1.25f);
    box(kSteel, kiosk + glm::vec3(0.0f, 0.65f, 0.0f), {0.25f, 1.30f, 0.28f});
    box(kTeal, kiosk + glm::vec3(0.0f, 1.38f, 0.0f), {0.86f, 0.62f, 0.17f});
    box(kIvory, kiosk + glm::vec3(0.0f, 1.42f, 0.10f), {0.105f, 0.32f, 0.03f});
}

void drawSecurity(const Site& site) {
    const glm::vec3 booth = site.booth();
    box(kSteel, booth + glm::vec3(0.0f, 0.13f, 0.0f), {3.5f, 0.26f, 3.55f});
    box(kIvory, booth + glm::vec3(0.0f, 1.45f, 0.0f), {2.9f, 2.4f, 2.9f});
    box(kGlass, booth + glm::vec3(0.0f, 1.65f, 1.47f), {1.98f, 1.0f, 0.11f});
    box(kGlass, booth + glm::vec3(-1.47f, 1.65f, 0.0f), {0.11f, 0.92f, 1.96f});
    box(kRoof, booth + glm::vec3(0.0f, 2.71f, 0.0f), {3.4f, 0.25f, 3.45f});
    const float z = site.halfDepth - 3.2f;
    box(kSteel, {-4.05f, 0.52f, z}, {0.57f, 1.04f, 0.66f});
    // Park the striped arm upright so the visitor route stays open.
    box(kIvory, {-4.05f, 2.33f, z}, {0.135f, 3.12f, 0.15f});
    for (int band = 0; band < 5; ++band) {
        box({0.78f, 0.16f, 0.10f}, {-4.05f, 1.02f + 0.61f * band, z}, {0.149f, 0.27f, 0.164f});
    }
}

// Rows of a 3-by-5 block alphabet, encoded as three-bit masks.
constexpr std::array<std::string_view, 26> kLetters{{
    "25755", "65656", "34443", "65556", "74647", "74644", "34553",
    "55755", "72227", "11152", "55655", "44447", "57755", "57775",
    "25552", "65644", "25573", "65655", "34216", "72222", "55557",
    "55552", "55775", "55255", "55222", "71247",
}};

std::string_view glyph(char letter) {
    if (letter >= 'A' && letter <= 'Z') return kLetters[letter - 'A'];
    if (letter == '1') return "26227";
    if (letter == '-') return "00700";
    return "00000";
}

void lettering(std::string_view label, float cell) {
    const float width = (label.size() * 4 - 1) * cell;
    color(kInk);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    for (std::size_t index = 0; index < label.size(); ++index) {
        const auto rows = glyph(label[index]);
        for (int row = 0; row < 5; ++row) {
            for (int column = 0; column < 3; ++column) {
                if (((rows[row] - '0') & (4 >> column)) == 0) continue;
                const float x = (4 * index + column) * cell - width / 2.0f;
                const float y = (2 - row) * cell;
                glVertex3f(x, y, 0.0f);
                glVertex3f(x + cell, y, 0.0f);
                glVertex3f(x + cell, y + cell, 0.0f);
                glVertex3f(x, y + cell, 0.0f);
            }
        }
    }
    glEnd();
}

void placard(glm::vec3 position, std::string_view label, float cell) {
    glPushMatrix();
    move(position);
    box(kIvory, {}, {(label.size() * 4 + 4) * cell, 8.0f * cell, 0.13f});
    move({0.0f, -0.5f * cell, 0.071f});
    lettering(label, cell);
    glPopMatrix();
}

void groundLabel(glm::vec3 position, std::string_view label, float cell) {
    glPushMatrix();
    move(position);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    lettering(label, cell);
    glPopMatrix();
}

void drawWayfinding(const Site& site) {
    placard(site.control + glm::vec3(0.0f, 7.8f, 7.04f), "CONTROL OFFICE", 0.21f);
    placard(site.workshop + glm::vec3(0.0f, 5.03f, 7.02f), "MAINTENANCE", 0.22f);
    placard(site.inverter + glm::vec3(0.0f, 4.05f, 4.59f), "INVERTER ROOM", 0.15f);
    placard(site.transformer + glm::vec3(0.0f, 1.25f, 4.42f), "TRANSFORMER PAD", 0.15f);
    placard(site.canopy + glm::vec3(0.0f, 5.11f, 2.75f), "SOLAR VISITOR CANOPY", 0.10f);

    const float w = site.halfWidth;
    const float d = site.halfDepth;
    for (int bank : {-1, 1}) {
        const float x = bank * 0.64f * w;
        for (float dx : {-2.7f, 2.7f}) {
            box(kSteel, {x + dx, 0.46f, 0.167f * d}, {0.09f, 0.92f, 0.10f});
        }
        placard({x, 0.83f, 0.167f * d}, bank < 0 ? "PV FIELD A" : "PV FIELD B", 0.19f);
        groundLabel({bank * 0.572f * w, 0.052f, -0.715f * d}, "PV EXPANSION AREA", 0.27f);
        groundLabel({bank * 0.855f * w, 0.052f, 0.56f * d}, "PV RESERVE", 0.18f);
    }
    groundLabel({0.0f, 0.052f, -0.735f * d}, "TECHNICAL", 0.26f);
    groundLabel({0.0f, 0.052f, -0.785f * d}, "EXPANSION", 0.26f);
}

// Cable-tray covers sit almost flush where the route crosses visitor paths.
void cableRun(glm::vec3 start, glm::vec3 end, float width) {
    const auto delta = end - start;
    glPushMatrix();
    move((start + end) * 0.5f);
    glRotatef(glm::degrees(std::atan2(delta.x, delta.z)), 0.0f, 1.0f, 0.0f);
    color(kConduit);
    shapes::box(width, 0.03f, std::hypot(delta.x, delta.z) + width);
    glPopMatrix();
}

void drawPowerLinks(const Site& site, const PlotInfo& plot) {
    constexpr float y = 0.078f;
    const float busZ = site.inverter.z - 0.9f;
    const float transformerX = site.transformer.x;
    for (int bank : {-1, 1}) {
        const float trunkX = site.fieldCabinet(bank, 0).x;
        for (int row = 0; row < kRackRows; ++row) {
            const float rowZ = site.rowCableZ(row);
            const float outerX = site.panel(bank, kRackColumns - 1, row).x;
            cableRun({trunkX, y, rowZ}, {outerX, y, rowZ}, 0.19f);
            for (int column = 0; column < kRackColumns; ++column) {
                const auto rack = site.panel(bank, column, row);
                cableRun({rack.x, y, rack.z - 0.80f}, {rack.x, y, rowZ}, 0.12f);
            }
        }
        cableRun({trunkX, y, site.rowCableZ(kRackRows - 1)}, {trunkX, y, site.rowCableZ(0)}, 0.27f);
        const float destinationX = bank < 0 ? site.inverter.x : transformerX;
        cableRun({trunkX, y, busZ}, {destinationX, y, busZ}, 0.27f);
    }
    cableRun({site.inverter.x, y, busZ}, {transformerX, y, busZ}, 0.30f);
    const auto turbineCabinet = steam::switchgearPosition(plot);
    const float rearTie = site.transformer.z - 5.1f;
    cableRun({turbineCabinet.x, y, turbineCabinet.z}, {turbineCabinet.x, y, rearTie}, 0.25f);
    cableRun({turbineCabinet.x, y, rearTie}, {transformerX, y, rearTie}, 0.25f);
    cableRun({transformerX, y, rearTie}, {transformerX, y, busZ}, 0.28f);
}

void powerBulb(glm::vec3 base, float size, bool powered) {
    glPushMatrix();
    move(base);
    glScalef(size, size, size);
    color(kInk);
    shapes::cylinder(0.27f, 0.17f, 8);
    move({0.0f, 0.37f, 0.0f});
    const GLfloat glow[] = {0.24f, 0.83f, 0.38f, 1.0f};
    const GLfloat dark[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, powered ? glow : dark);
    color(powered ? Color{0.55f, 0.91f, 0.64f} : Color{0.12f, 0.22f, 0.17f});
    shapes::sphere(0.26f, 8, 5);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, dark);
    glPopMatrix();
}

void drawPowerIndicators(const Site& site, const PlotInfo& plot, const citylab::Frame& frame) {
    // The clock has civil twilight. A positive solar elevation is needed for generation.
    const bool powered = frame.sunDirection.y > 0.0f && frame.daylight > 0.0f;
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
    for (int bank : {-1, 1}) {
        for (int row = 0; row < kRackRows; ++row) {
            const float height = row % 2 == 0 ? 0.75f : 0.82f;
            powerBulb(site.fieldCabinet(bank, row) + glm::vec3(0.0f, 1.815f * height, 0.0f), 0.66f, powered);
        }
    }
    powerBulb(steam::switchgearPosition(plot) + glm::vec3(0.0f, 1.05f, 0.0f), 0.62f, powered);
    powerBulb(site.transformer + glm::vec3(3.9f, 3.0f, 0.0f), 0.80f, powered);
    glPopAttrib();
}

void drawSite(const Site& site, const PlotInfo& plot) {
    drawGrounds(site);
    drawControlBuilding(site.control);
    drawInverterHall(site.inverter);
    drawWorkshop(site.workshop);
    drawSolarField(site);
    drawTransformerPad(site.transformer);
    drawEntryCanopy(site.canopy);
    drawVisitorFurniture(site);
    drawSecurity(site);
    drawWayfinding(site);
    steam::draw(plot);
    drawPowerLinks(site, plot);
}

void solid(Context& ctx, glm::vec3 base, float width, float height, float depth) {
    ctx.addCollider(base + glm::vec3(-width / 2.0f, 0.0f, -depth / 2.0f),
                    base + glm::vec3(width / 2.0f, height, depth / 2.0f));
}

void registerColliders(Context& ctx, const Site& site) {
    solid(ctx, site.control, 17.8f, 8.3f, 13.2f);
    solid(ctx, site.inverter, 9.8f, 4.3f, 8.32f);
    solid(ctx, site.transformer, 10.2f, 4.1f, 7.8f);
    solid(ctx, site.booth(), 2.9f, 2.7f, 2.9f);
    solid(ctx, {-4.05f, 0.0f, site.halfDepth - 3.2f}, 0.57f, 1.04f, 0.66f);
    solid(ctx, site.workshop, 19.0f, 4.7f, 13.0f);
    for (int side : {-1, 1}) {
        for (float z : {-2.3f, 2.3f}) {
            solid(ctx, site.canopy + glm::vec3(5.8f * side, 0.0f, z), 0.4f, kPostHeight, 0.4f);
        }
    }
    for (int bank : {-1, 1}) {
        for (int row = 0; row < kRackRows; ++row) {
            for (int column = 0; column < kRackColumns; ++column) {
                glm::vec3 panel = site.panel(bank, column, row);
                panel.y = 0.0f;
                solid(ctx, panel, 4.42f, 2.9f, 2.93f);
            }
            solid(ctx, site.fieldCabinet(bank, row), 0.66f, 1.82f, 1.22f);
        }
    }

}

}  // namespace

class SolarPlantZone : public Zone {
public:
    void init(Context& ctx) override {
        const Site site(ctx.plot());
        registerColliders(ctx, site);
        steam::addColliders(ctx);
        scenery_ = glGenLists(1);
        if (scenery_ != 0) {
            glNewList(scenery_, GL_COMPILE);
            drawSite(site, ctx.plot());
            glEndList();
        }
    }

    void draw(const Context& ctx) const override {
        const Site site(ctx.plot());
        if (scenery_ != 0) glCallList(scenery_);
        else drawSite(site, ctx.plot());
        drawPowerIndicators(site, ctx.plot(), ctx.frame());
    }

private:
    GLuint scenery_ = 0;
};

}  // namespace team25

CITYLAB_REGISTER_ZONE(team25::SolarPlantZone);
