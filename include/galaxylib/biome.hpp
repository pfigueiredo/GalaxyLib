#pragma once

#include <string>

namespace galaxy {

/// Biome types for planet surfaces and rings.
enum class BiomeType {
    Ocean,
    Plains,
    Mountains,
    Desert,
    Rocky,
    Forest,
    Tundra,
    Volcanic,
    Ice,
    Gas,
    RingMetalRich,
    RingIce,
    RingRock
};

/// A spatial region on a planet surface.
struct PlanetZone {
    std::string name;
    BiomeType biome;
    float center_lat;   // -90 to 90
    float center_lon;   // -180 to 180
    float influence;    // 0.0 to 1.0 (relative weight)
};

/// A radial band in a planet's ring system.
struct RingZone {
    BiomeType biome;
    float inner_radius_km;
    float outer_radius_km;
};

} // namespace galaxy
