#pragma once

#include <string>

namespace galaxy {

/// Type of port or station.
enum class SpacePortType {
    SurfacePort,
    OrbitalStation
};

/// Infrastructure for a populated body.
struct SpacePort {
    std::string id;
    std::string name;
    SpacePortType type;

    // Surface coordinates (for SurfacePort).
    float latitude{0.0f};  // -90 to 90 degrees
    float longitude{0.0f}; // -180 to 180 degrees

    // Orbital parameters (for OrbitalStation).
    float altitude_km{0.0f};     // Above surface
    float inclination_deg{0.0f};  // 0 to 180 degrees
    float phase_deg{0.0f};        // Initial position in orbit (0 to 360)
    float orbital_period_hours{0.0f};

    // Calculated relative position (meters from body center).
    // Updated via get_relative_location or SystemBody::update.
    double rel_x_m{0.0};
    double rel_y_m{0.0};
    double rel_z_m{0.0};

    /// Returns the position relative to the center of the SystemBody in metres.
    void get_relative_location(double elapsed_days, float body_radius_km,
                              double& out_x, double& out_y, double& out_z) const;

    /// Updates the internal rel_x_m, rel_y_m, rel_z_m based on time.
    void update(double elapsed_days, float body_radius_km);
};

}  // namespace galaxy
