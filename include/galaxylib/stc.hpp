#pragma once

#include "galaxylib/coords.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <queue>

namespace galaxy {

class StarSystem;
class SystemBody;
struct SpacePort;

/// Flight phases as defined in ATC_ideas.txt
enum class FlightPhase {
    None = 0,
    Departure,
    EnRoute,
    Approach,
    ApproachHold,
    CloseFinal,
    MissedApproach
};

/// Segments as defined in ATC_ideas.txt
enum class FlightSegmentType {
    DepartureClimb,
    DepartureCoast,
    EnRouteAccel,
    EnRouteCoast,
    EnRouteDecel,
    Approach,
    ApproachHold,
    CloseFinal,
    MissedApproach
};

/// Waypoint in the flight plan
struct Waypoint {
    double x, y, z;
    double target_speed_m_s;
    bool is_relative; // RF relative to parent_object_id
    double rel_x, rel_y, rel_z;
};

struct FlightSegment {
    FlightSegmentType type;
    std::vector<Waypoint> waypoints;
    size_t current_waypoint_idx{0};
};

/// Orders / Advisories the STC can issue
enum class STCOrderType {
    TurnLeftRight,                  // Heading <1 360>
    PitchUpDown,                    // Vertical Heading <1 360>
    RotateLeftRight,                // Angle <-180 +180>
    AccelerateTo,                   // speed in km/h
    DecelerateTo,                   // speed in km/h
    InterceptStationApproachBeacon  // delivery to final approach vector
};

struct STCOrder {
    STCOrderType type;
    float value; // heading, angle, or speed
    std::string description;
};

/// Ship capabilities and state
struct ShipManifesto {
    std::string ship_id;
    std::string call_sign;
    float turn_capabilities;       // deg/s?
    float max_acceleration;        // m/s^2
    float max_deceleration;        // m/s^2
    
    // Current state
    double pos_x, pos_y, pos_z;    // System coordinates (meters)
    double vel_x, vel_y, vel_z;    // Velocity (m/s)
    float dir_x, dir_y, dir_z;     // Direction vector (normalized)
    
    bool is_ifr;                   // IFR or VFR operation
    
    std::string destination_id;    // SpacePort ID or Body ID
    // Optional: FlightPlan details
};

/// Update ping from ship (radar input)
struct ShipUpdatePing {
    std::string ship_id;
    double pos_x, pos_y, pos_z;
    double vel_x, vel_y, vel_z;
    float dir_x, dir_y, dir_z;
    double speed_m_s;

    // Reference Frame data
    std::string parent_object_id; // ID of body/station for relative RF
    double rel_pos_x, rel_pos_y, rel_pos_z;
    double rel_vel_x, rel_vel_y, rel_vel_z;
};

/// Internal representation of a ship in STC
struct STCShipState {
    ShipManifesto manifesto;
    FlightPhase current_phase{FlightPhase::None};
    std::vector<STCOrder> pending_orders;
    
    // Reference Frame
    std::string parent_object_id;
    double rel_pos_x, rel_pos_y, rel_pos_z;
    double rel_vel_x, rel_vel_y, rel_vel_z;

    // Flight plan
    std::vector<FlightSegment> segments;
    size_t current_segment_idx{0};
    double last_wp_x, last_wp_y, last_wp_z;

    // Expected state for deviation check
    double expected_pos_x, expected_pos_y, expected_pos_z;
    double expected_vel_x, expected_vel_y, expected_vel_z;
};

/// Station Type Manifesto data
struct StationTypeManifesto {
    float collision_radius_m;
    float approach_final_vector_x;
    float approach_final_vector_y;
    float approach_final_vector_z;
};

/**
 * @brief Sector Traffic Controller (STC)
 * 
 * Manages ship traffic, flight plans, and collision avoidance within a star system.
 */
class STC {
public:
    STC(const StarSystem& system);

    /// Initializes the STC by scanning system bodies and ports.
    void init();

    /// Registers a ship with the STC.
    void receive_manifesto(const ShipManifesto& manifesto);

    /// Main update loop, expected to be called at ~1Hz.
    void tick(double elapsed_seconds);

    /// Receives radar updates from ships.
    void process_radar_ping(const ShipUpdatePing& ping);

    /// Get pending orders for a specific ship.
    std::vector<STCOrder> get_pending_orders(const std::string& ship_id);

private:
    // Internal representation of system objects
    struct SystemObject {
        std::string id;
        double pos_x, pos_y, pos_z;
        float radius_m;
        bool is_station;
        StationTypeManifesto station_data;
    };

    void calculate_flight_plan(STCShipState& ship);
    void update_ship_guidance(STCShipState& ship, double dt);
    void check_collisions(STCShipState& ship);
    void issue_collision_advisory(STCShipState& ship, const SystemObject& obj);
    void check_deviations(STCShipState& ship);

    const StarSystem& system_;
    std::map<std::string, STCShipState> ships_;
    std::queue<ShipUpdatePing> radar_queue_;

    std::vector<SystemObject> objects_;

    // Helper to load station manifesto based on type
    StationTypeManifesto load_station_manifesto(const std::string& type);
};

} // namespace galaxy
