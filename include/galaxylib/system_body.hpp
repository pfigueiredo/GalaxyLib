#pragma once

#include "galaxylib/calendar_date.hpp"
#include "galaxylib/space_port.hpp"
#include "galaxylib/biome.hpp"
#include "galaxylib/resource.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace galaxy {

class StarSystem;

/// Mirrors `GalaxyE.Galaxy.SystemBodyType`.
enum class SystemBodyType : int {
    Asteroid = 0,
    RockyPlanetoid = 1,
    RockyWorld = 2,
    Venuzian = 3,
    Inferno = 4,
    IceWorld = 5,
    WaterWorld = 6,
    Terrestrial = 7,
    SubGasGiant = 8,
    RingedGasGiant = 9,
    GasGiant = 10
};

/// One planet/moon/asteroid orbit (`SystemBody.cs`).
class SystemBody {
public:
    SystemBody(const StarSystem& star_system, int system_body_id);

    int id() const { return system_body_id_; }
    const std::string& name() const { return name_; }

    SystemBodyType body_type() const { return body_type_; }

    /// Orbital distance in AU (`OrbitalRadius` in C#).
    float orbital_radius_au() const { return orbital_radius_au_; }
    /// Orbital period in Earth years (Kepler-style scaling from C#).
    float orbital_period_years() const { return orbital_period_years_; }
    int base_angle_deg() const { return base_angle_deg_; }

    int temperature_c() const { return temperature_c_; }
    /// Procedural size index (`_radius` in C#; `rand.Next(20,1000)/100`).
    int radius() const { return radius_; }

    /// In-system position in metres (see `update_system_location`).
    double system_x_m() const { return system_x_m_; }
    double system_y_m() const { return system_y_m_; }
    double system_z_m() const { return system_z_m_; }

    /// Recompute in-system metres from `base_angle_deg_` at `k_orbit_reference_date` (2012-01-01).
    /// Angle advance matches C#: (elapsed_days / 365.25) * orbital_period_years * 360 degrees.
    void update_system_location(double elapsed_days_since_reference);
    void update_system_location(CalendarDate date);

    std::string_view body_class_ext() const;

    float population_millions() const { return population_millions_; }
    float habitability_score() const { return habitability_score_; }
    const std::vector<SpacePort>& ports() const;

    const std::vector<PlanetZone>& zones() const { return zones_; }
    const std::vector<RingZone>& ring_zones() const { return ring_zones_; }

    /// Determine the biome at specific spherical coordinates.
    BiomeType get_biome_at(float lat, float lon) const;
    /// Determine the ring biome at a radial distance from the planet's center.
    BiomeType get_ring_biome_at(float distance_km) const;

    /// Get all resource deposits available at a specific location on the surface.
    std::vector<ResourceDeposit> get_resources_at(float lat, float lon) const;
    /// Get all resource deposits available in the rings at a specific radial distance.
    std::vector<ResourceDeposit> get_ring_resources_at(float distance_km) const;

    /// Return the master list of all available resources in the universe.
    static const std::vector<ResourceDefinition>& get_resource_definitions();

private:
    void generate_body(const StarSystem& star);
    void generate_sol_bodies(int system_body_id);
    void calculate_habitability(const StarSystem& star);
    void generate_ports() const;
    void generate_zones(const StarSystem& star);

    int system_body_id_{};
    const StarSystem& star_system_;
    std::string name_;

    SystemBodyType body_type_{SystemBodyType::Asteroid};

    float orbital_radius_au_{};
    float orbital_period_years_{};
    int base_angle_deg_{};

    int temperature_c_{};
    int radius_{};

    double system_x_m_{};
    double system_y_m_{};
    double system_z_m_{};

    float habitability_score_{0.0f};
    float population_millions_{0.0f};

    mutable std::vector<SpacePort> ports_;
    mutable bool ports_cached_{false};

    std::vector<PlanetZone> zones_;
    std::vector<RingZone> ring_zones_;
};

}  // namespace galaxy
