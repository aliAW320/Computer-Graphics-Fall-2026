#include <citylab/CityLab.h>

#include <array>
#include <cmath>

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

constexpr Color kGround{0.50f, 0.49f, 0.37f};
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
constexpr float kFrontCourtDepth = 27.0f;
constexpr float kRearCourtDepth = 22.0f;
constexpr float kFirstRackDepth = 15.0f;
constexpr float kRackSpacing = 9.0f;
constexpr float kPanelHeight = 1.55f;
constexpr float kPanelThickness = 0.14f;
constexpr float kPanelHalfDepth = 1.5f;
constexpr float kTilt = 30.0f;
constexpr float kSquareEdge = 2.0f;
constexpr int kPanelColumns = 6;
constexpr int kPanelRows = 4;
constexpr int kRackRows = 5;
constexpr int kRackColumns = 5;
constexpr float kPostHeight = 5.1f;
constexpr float kFrontPathWidth = 5.0f;

struct Site {
    explicit Site(const PlotInfo& plot)
        : halfWidth(plot.width / 2.0f), halfDepth(plot.depth / 2.0f),
          control{-0.59f * halfWidth, 0.0f, halfDepth - kFrontCourtDepth},
          inverter{0.61f * halfWidth, 0.0f, halfDepth - kFrontCourtDepth - 4.0f},
          workshop{-0.59f * halfWidth, 0.0f, -halfDepth + kRearCourtDepth},
          canopy{0.0f, 0.0f, halfDepth - kEntryDepth} {}

    glm::vec3 panel(int column, int row) const {
        return {halfWidth * (0.20f + 0.16f * column), kPanelHeight,
                -halfDepth + kFirstRackDepth + kRackSpacing * row};
    }

    float rowCableZ(int row) const { return panel(0, row).z - 2.8f; }

    glm::vec3 fieldCabinet(int index) const {
        const float z = index == kRackRows ? panel(0, kRackRows - 1).z + 4.0f : rowCableZ(index);
        return {halfWidth * 0.60f, 0.0f, z};
    }

    float halfWidth;
    float halfDepth;
    glm::vec3 control;
    glm::vec3 inverter;
    glm::vec3 workshop;
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
    color(kSteel);
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
        shapes::sphere(0.93f, 8, 5);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawGrounds(const Site& site, const PlotInfo& plot) {
    const float w = site.halfWidth;
    const float d = site.halfDepth;
    ground(kGround, 0.0f, 0.0f, 2.0f * w, 2.0f * d, 0.004f);
    // Four edge-to-edge landscape bands, plus the raised visual layer of paths.
    ground(kGravel, 0.0f, -0.64f * d, 2.0f * w, 0.72f * d, 0.012f);
    ground(kPlanting, 0.0f, -0.10f * d, 2.0f * w, 0.36f * d, 0.012f);
    ground(kGravel, 0.0f, 0.42f * d, 2.0f * w, 0.68f * d, 0.012f);
    ground(kPlanting, 0.0f, 0.88f * d, 2.0f * w, 0.24f * d, 0.012f);

    for (int side : {-1, 1}) {
        const float x = side * w;
        ground(kPlanting, 0.94f * x, 0.0f, 0.08f * w, 1.91f * d, 0.020f);
        ground(kDarkPaving, 0.62f * x, 0.42f * d, 0.61f * w, 0.42f * d, 0.022f);
        ground(kGrassLight, 0.34f * x, 0.79f * d, 0.30f * w, 0.14f * d, 0.022f);
        ground(kGravel, 0.65f * x, 0.82f * d, 0.29f * w, 0.17f * d, 0.024f);
        for (int i = 0; i < 5; ++i) {
            const float z = (-0.82f + 0.40f * i) * d;
            tree(0.91f * x, z, 0.88f + 0.09f * ((i + (side == 1)) % 3));
        }
        for (int i = 0; i < 3; ++i) {
            const float z = (0.33f + 0.19f * i) * d;
            bollard(side * 3.15f, z, 0.80f + 0.14f * (i % 2));
        }
        tree(0.35f * x, 0.82f * d, 0.85f);
    }

    ground(kDarkPaving, site.workshop.x + 1.0f, site.workshop.z + 2.0f,
           0.71f * w, 21.0f, 0.022f);
    const float arrayX = (site.panel(0, 0).x + site.panel(4, 0).x) / 2.0f;
    for (int row = 0; row < kRackRows; ++row) {
        const float z = site.panel(0, row).z;
        ground(kDarkPaving, arrayX, z, site.panel(4, 0).x - site.panel(0, 0).x + 6.0f,
               5.1f, 0.026f);
    }
    for (int i = 1; i < 5; ++i) {
        const float x = site.workshop.x + (i - 2.5f) * 4.0f;
        ground(kAmber, x, site.workshop.z + 8.3f, 0.115f, 3.4f, 0.029f);
    }

    const float rearLanding = site.workshop.z + 9.1f;
    const float visitorCourt = site.control.z + 8.65f;
    path(kPaving, 0.0f, rearLanding, 0.0f, d, kFrontPathWidth);
    path(kPaving, site.workshop.x, rearLanding, 0.0f, rearLanding, 3.3f);
    path(kPaving, site.control.x, visitorCourt, site.inverter.x, visitorCourt, 3.1f);
    path(kPaving, site.inverter.x, visitorCourt, site.inverter.x, site.inverter.z + 6.0f, 3.1f);
    path(kPaving, 0.0f, 2.0f, site.panel(4, 0).x + 2.3f, 2.0f, 2.4f);
    for (float z : {-0.33f * d, 0.18f * d}) {
        path(kPaving, 0.0f, z, steam::visitorEdgeX(plot), z, 2.8f);
    }
    for (int i = 0; i < 10; ++i) {
        const float z = d - 3.5f - i * 2.5f;
        ground(kIvory, -2.20f, z, 0.14f, 0.62f, 0.043f);
        ground(kIvory, 2.20f, z, 0.14f, 0.62f, 0.043f);
    }

    // Break up the rear perimeter with alternating plant beds and maintenance pads.
    for (int i = 0; i < 7; ++i) {
        const float x = (i - 3) * w * 0.24f;
        ground(i % 2 == 0 ? kPlanting : kGrassLight,
               x, -0.91f * d, 0.20f * w, 0.10f * d, 0.022f);
        if (i % 2 == 0) tree(x, -0.91f * d, 0.80f);
    }
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
    box(kTeal, {4.5f, 8.38f, -2.4f}, {5.8f, 0.39f, 5.5f});
    box(kSteel, {4.5f, 9.15f, -2.4f}, {0.17f, 1.55f, 0.17f});
    box(kAmber, {4.5f, 9.94f, -2.4f}, {0.27f, 0.22f, 0.27f});
    glPopMatrix();
}

void drawInverterHall(glm::vec3 position) {
    glPushMatrix();
    move(position);
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
    move({-3.82f, 1.95f, 6.73f});
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
    box(kInk, {-2.52f, 1.93f, 6.53f}, {5.33f, 3.46f, 0.12f});
    box(kSteel, {-1.19f, 1.93f, 6.65f}, {2.49f, 3.36f, 0.15f});
    box(kTeal, {0.0f, 4.43f, 6.57f}, {19.1f, 0.34f, 0.20f});
    openWorkshopDoor();
    for (int i = 0; i < 3; ++i) {
        box(kGlass, {2.0f + 2.65f * i, 2.66f, 6.59f}, {2.12f, 1.52f, 0.19f});
        box(kSteel, {2.0f + 2.65f * i, 1.84f, 6.64f}, {2.27f, 0.12f, 0.31f});
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
    for (int row = 0; row < kRackRows; ++row) {
        for (int column = 0; column < kRackColumns; ++column) {
            const float yaw = column == kRackColumns - 1 ? 8.0f : 0.0f;
            const float widthScale = row == kRackRows - 1 ? 0.90f : 1.0f;
            drawPanelRack(site.panel(column, row), yaw, widthScale);
        }
    }
    for (int i = 0; i <= kRackRows; ++i) {
        glPushMatrix();
        move(site.fieldCabinet(i));
        glRotatef(i % 2 == 0 ? 90.0f : -90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, i % 2 == 0 ? 1.0f : 1.1f, 1.0f);
        switchgear();
        glPopMatrix();
    }
    // Six demonstration panels in the garden west of the visitor route.
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 2; ++column) {
            drawPanelRack({-site.halfWidth * (0.24f + 0.16f * column),
                           kPanelHeight, -10.5f + 7.4f * row},
                          column == 0 ? -12.0f : 12.0f, 0.89f);
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
    const float firstX = site.panel(0, 0).x;
    const float lastX = site.panel(kRackColumns - 1, 0).x;
    const float trunkX = site.fieldCabinet(0).x;
    const float busZ = site.inverter.z - 7.15f;
    constexpr float groundY = 0.078f;
    for (int row = 0; row < kRackRows; ++row) {
        const float rowZ = site.rowCableZ(row);
        cableRun({firstX, groundY, rowZ}, {lastX, groundY, rowZ}, 0.19f);
        for (int column = 0; column < kRackColumns; ++column) {
            const auto rack = site.panel(column, row);
            cableRun({rack.x, groundY, rack.z - 0.80f}, {rack.x, groundY, rowZ}, 0.12f);
            box(kInk, {rack.x, 0.14f, rack.z - 0.80f}, {0.43f, 0.26f, 0.34f});
        }
    }
    cableRun({trunkX, groundY, site.rowCableZ(0)}, {trunkX, groundY, busZ}, 0.30f);

    // The inverter yard is the tie-in between PV and turbine-generator output.
    const auto turbineCabinet = steam::switchgearPosition(plot);
    const float deckEdge = steam::visitorEdgeX(plot);
    const float deckY = turbineCabinet.y + 0.018f;
    cableRun({turbineCabinet.x, deckY, turbineCabinet.z}, {turbineCabinet.x, deckY, busZ}, 0.27f);
    cableRun({turbineCabinet.x, deckY, busZ}, {deckEdge, deckY, busZ}, 0.27f);
    box(kConduit, {deckEdge, (deckY + groundY) * 0.5f, busZ}, {0.27f, deckY - groundY, 0.27f});
    cableRun({deckEdge, groundY, busZ}, {site.inverter.x, groundY, busZ}, 0.32f);
    const float inverterRear = site.inverter.z - 5.86f;
    cableRun({site.inverter.x, groundY, busZ}, {site.inverter.x, groundY, inverterRear}, 0.32f);
    box(kConduit, {site.inverter.x, 0.43f, inverterRear}, {0.32f, 0.86f, 0.31f});
    box(kSteel, {site.inverter.x, 0.91f, inverterRear}, {0.80f, 0.46f, 0.35f});
}

void powerBulb(glm::vec3 base, float size, bool powered) {
    glPushMatrix();
    move(base);
    glScalef(size, size, size);
    color(kInk);
    shapes::cylinder(0.27f, 0.17f, 12);
    move({0.0f, 0.37f, 0.0f});
    const GLfloat glow[] = {0.24f, 0.83f, 0.38f, 1.0f};
    const GLfloat dark[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, powered ? glow : dark);
    color(powered ? Color{0.55f, 0.91f, 0.64f} : Color{0.12f, 0.22f, 0.17f});
    shapes::sphere(0.26f, 12, 8);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, dark);
    glPopMatrix();
}

void drawPowerIndicators(const Site& site, const PlotInfo& plot, const citylab::Frame& frame) {
    // The clock has civil twilight. A positive solar elevation is needed for generation.
    const bool powered = frame.sunDirection.y > 0.0f && frame.daylight > 0.0f;
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
    for (int i = 0; i <= kRackRows; ++i) {
        const float cabinetHeight = i % 2 == 0 ? 1.815f : 1.815f * 1.1f;
        powerBulb(site.fieldCabinet(i) + glm::vec3(0.0f, cabinetHeight, 0.0f), 0.8f, powered);
    }
    powerBulb(steam::switchgearPosition(plot) + glm::vec3(0.0f, 2.14f, 0.0f), 1.05f, powered);
    powerBulb(site.inverter + glm::vec3(0.0f, 3.36f, 5.90f), 1.10f, powered);
    for (float x : {-5.8f, 5.8f}) {
        powerBulb(site.canopy + glm::vec3(x, 0.30f, 2.3f), 1.20f, powered);
    }
    glPopAttrib();
}

void drawSite(const Site& site, const PlotInfo& plot) {
    drawGrounds(site, plot);
    drawControlBuilding(site.control);
    drawInverterHall(site.inverter);
    drawWorkshop(site.workshop);
    drawSolarField(site);
    drawEntryCanopy(site.canopy);
    steam::draw(plot);
    drawPowerLinks(site, plot);
}

void solid(Context& ctx, glm::vec3 base, float width, float height, float depth) {
    ctx.addCollider(base + glm::vec3(-width / 2.0f, 0.0f, -depth / 2.0f),
                    base + glm::vec3(width / 2.0f, height, depth / 2.0f));
}

void registerColliders(Context& ctx, const Site& site) {
    solid(ctx, site.control, 17.8f, 8.3f, 13.2f);
    solid(ctx, site.inverter, 17.5f, 5.0f, 11.4f);
    solid(ctx, site.workshop, 19.0f, 4.7f, 13.0f);
    for (int side : {-1, 1}) {
        for (float z : {-2.3f, 2.3f}) {
            solid(ctx, site.canopy + glm::vec3(5.8f * side, 0.0f, z), 0.4f, kPostHeight, 0.4f);
        }
    }
    for (int row = 0; row < kRackRows; ++row) {
        for (int column = 0; column < kRackColumns; ++column) {
            glm::vec3 panel = site.panel(column, row);
            panel.y = 0.0f;
            solid(ctx, panel, 4.55f, 2.9f, 2.75f);
        }
    }
    for (int i = 0; i <= kRackRows; ++i) {
        solid(ctx, site.fieldCabinet(i), 0.99f, 2.1f, 1.65f);
    }
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 2; ++column) {
            solid(ctx, {-site.halfWidth * (0.24f + 0.16f * column), 0.0f, -10.5f + 7.4f * row},
                  4.3f, 2.9f, 2.9f);
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
