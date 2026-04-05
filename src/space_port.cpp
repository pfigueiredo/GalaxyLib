#include "galaxylib/space_port.hpp"
#include <cmath>

namespace galaxy {

void SpacePort::get_relative_location(double elapsed_days, float body_radius_km,
                                     double& out_x, double& out_y, double& out_z) const {
    if (type != SpacePortType::OrbitalStation || orbital_period_hours <= 0.0f) {
        out_x = out_y = out_z = 0.0;
        return;
    }

    const double elapsed_hours = elapsed_days * 24.0;
    const double current_phase_deg = std::fmod(static_cast<double>(phase_deg) + (elapsed_hours / orbital_period_hours) * 360.0, 360.0);
    
    constexpr double deg_to_rad = 0.0174532925;
    const double orbit_radius_m = (static_cast<double>(body_radius_km) + static_cast<double>(altitude_km)) * 1000.0;
    const double phase_rad = current_phase_deg * deg_to_rad;
    const double inc_rad = static_cast<double>(inclination_deg) * deg_to_rad;

    // Standard orbital position (in orbit plane).
    const double x_orbit = std::cos(phase_rad) * orbit_radius_m;
    const double y_orbit = std::sin(phase_rad) * orbit_radius_m;

    // Rotate into 3D using inclination (assuming inclination rotates around the X axis for simplicity).
    out_x = x_orbit;
    out_y = y_orbit * std::cos(inc_rad);
    out_z = y_orbit * std::sin(inc_rad);
}

void SpacePort::update(double elapsed_days, float body_radius_km) {
    get_relative_location(elapsed_days, body_radius_km, rel_x_m, rel_y_m, rel_z_m);
}

} // namespace galaxy
