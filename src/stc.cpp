#include "galaxylib/stc.hpp"
#include "galaxylib/star_system.hpp"
#include "galaxylib/system_body.hpp"
#include "galaxylib/space_port.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

#include <sstream>
#include <iomanip>

namespace galaxy {

namespace {
    std::string format_float(float val, int precision = 1) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << val;
        return ss.str();
    }

    const char* segment_type_to_string(FlightSegmentType type) {
        switch (type) {
            case FlightSegmentType::DepartureClimb: return "DepartureClimb";
            case FlightSegmentType::DepartureCoast: return "DepartureCoast";
            case FlightSegmentType::EnRouteAccel: return "EnRouteAccel";
            case FlightSegmentType::EnRouteCoast: return "EnRouteCoast";
            case FlightSegmentType::EnRouteDecel: return "EnRouteDecel";
            case FlightSegmentType::Approach: return "Approach";
            case FlightSegmentType::ApproachHold: return "ApproachHold";
            case FlightSegmentType::CloseFinal: return "CloseFinal";
            case FlightSegmentType::MissedApproach: return "MissedApproach";
            default: return "Unknown";
        }
    }
}

STC::STC(const StarSystem& system) : system_(system) {}

void STC::init() {
    objects_.clear();
    for (const auto& body : system_.bodies()) {
        SystemObject obj;
        obj.id = "body_" + std::to_string(body.id());
        obj.pos_x = body.system_x_m();
        obj.pos_y = body.system_y_m();
        obj.pos_z = body.system_z_m();
        obj.radius_m = static_cast<float>(body.radius()) * 6371.0f * 1000.0f; 
        obj.is_station = false;
        objects_.push_back(obj);

        for (const auto& port : body.ports()) {
            SystemObject port_obj;
            port_obj.id = port.id;
            port_obj.pos_x = obj.pos_x + port.rel_x_m;
            port_obj.pos_y = obj.pos_y + port.rel_y_m;
            port_obj.pos_z = obj.pos_z + port.rel_z_m;
            
            port_obj.is_station = (port.type == SpacePortType::OrbitalStation);
            
            if (port_obj.is_station) {
                port_obj.station_data = load_station_manifesto("default_station"); 
                port_obj.radius_m = port_obj.station_data.collision_radius_m;
            } else {
                port_obj.radius_m = 500.0f;
                port_obj.station_data.approach_final_vector_x = 0.0f;
                port_obj.station_data.approach_final_vector_y = 1.0f; 
                port_obj.station_data.approach_final_vector_z = 0.0f;
            }
            objects_.push_back(port_obj);
        }
    }
}

StationTypeManifesto STC::load_station_manifesto(const std::string& type) {
    StationTypeManifesto manifesto;
    manifesto.collision_radius_m = 2000.0f; 
    manifesto.approach_final_vector_x = 0.0f;
    manifesto.approach_final_vector_y = 0.0f;
    manifesto.approach_final_vector_z = 1.0f; 
    return manifesto;
}

void STC::receive_manifesto(const ShipManifesto& manifesto) {
    STCShipState state;
    state.manifesto = manifesto;
    state.current_phase = FlightPhase::None;
    
    state.last_wp_x = manifesto.pos_x;
    state.last_wp_y = manifesto.pos_y;
    state.last_wp_z = manifesto.pos_z;

    calculate_flight_plan(state);
    
    ships_[manifesto.ship_id] = state;
}

void STC::tick(double elapsed_seconds) {
    while (!radar_queue_.empty()) {
        auto ping = radar_queue_.front();
        radar_queue_.pop();
        
        if (ships_.count(ping.ship_id)) {
            auto& state = ships_[ping.ship_id];
            state.manifesto.pos_x = ping.pos_x;
            state.manifesto.pos_y = ping.pos_y;
            state.manifesto.pos_z = ping.pos_z;
            state.manifesto.vel_x = ping.vel_x;
            state.manifesto.vel_y = ping.vel_y;
            state.manifesto.vel_z = ping.vel_z;
            state.manifesto.dir_x = ping.dir_x;
            state.manifesto.dir_y = ping.dir_y;
            state.manifesto.dir_z = ping.dir_z;
            
            state.parent_object_id = ping.parent_object_id;
            state.rel_pos_x = ping.rel_pos_x;
            state.rel_pos_y = ping.rel_pos_y;
            state.rel_pos_z = ping.rel_pos_z;
            state.rel_vel_x = ping.rel_vel_x;
            state.rel_vel_y = ping.rel_vel_y;
            state.rel_vel_z = ping.rel_vel_z;

            check_deviations(state);
            check_collisions(state);
        }
    }

    for (auto& pair : ships_) {
        update_ship_guidance(pair.second, elapsed_seconds);
    }
}

void STC::process_radar_ping(const ShipUpdatePing& ping) {
    radar_queue_.push(ping);
}

std::vector<STCOrder> STC::get_pending_orders(const std::string& ship_id) {
    if (ships_.count(ship_id)) {
        auto orders = ships_[ship_id].pending_orders;
        ships_[ship_id].pending_orders.clear();
        return orders;
    }
    return {};
}

void STC::calculate_flight_plan(STCShipState& ship) {
    ship.segments.clear();
    ship.current_segment_idx = 0;

    const SystemObject* dest = nullptr;
    for (const auto& obj : objects_) {
        if (obj.id == ship.manifesto.destination_id) {
            dest = &obj;
            break;
        }
    }
    if (!dest) return;

    double target_x = dest->pos_x;
    double target_y = dest->pos_y;
    double target_z = dest->pos_z;

    // Phase: Departure
    ship.current_phase = FlightPhase::Departure;
    
    bool at_surface_port = false;
    bool at_station = false;
    const SystemObject* start_obj = nullptr;
    for (const auto& obj : objects_) {
        double dx = ship.manifesto.pos_x - obj.pos_x;
        double dy = ship.manifesto.pos_y - obj.pos_y;
        double dz = ship.manifesto.pos_z - obj.pos_z;
        if (dx*dx + dy*dy + dz*dz < (obj.radius_m + 50.0)*(obj.radius_m + 50.0)) {
            start_obj = &obj;
            if (obj.is_station) at_station = true;
            else at_surface_port = true;
            break;
        }
    }

    if (at_surface_port) {
        // Calculate a 15-20 degree climb towards destination
        // tan(18.4 deg) approx 0.33, so for 500m up, we move 1500m forward.
        double dir_to_dest_x = target_x - ship.manifesto.pos_x;
        double dir_to_dest_z = target_z - ship.manifesto.pos_z;
        double mag = std::sqrt(dir_to_dest_x*dir_to_dest_x + dir_to_dest_z*dir_to_dest_z);
        if (mag > 0.1) {
            dir_to_dest_x /= mag;
            dir_to_dest_z /= mag;
        } else {
            dir_to_dest_x = 1.0; // Default direction if destination is directly above
            dir_to_dest_z = 0.0;
        }

        FlightSegment climb;
        climb.type = FlightSegmentType::DepartureClimb;
        climb.waypoints.push_back({
            ship.manifesto.pos_x + dir_to_dest_x * 1500.0, 
            ship.manifesto.pos_y + 500.0, 
            ship.manifesto.pos_z + dir_to_dest_z * 1500.0, 
            100.0 / 3.6, false, 0, 0, 0});
        ship.segments.push_back(climb);

        FlightSegment coast;
        coast.type = FlightSegmentType::DepartureCoast;
        coast.waypoints.push_back({
            ship.manifesto.pos_x + dir_to_dest_x * 4500.0, 
            ship.manifesto.pos_y + 1500.0, 
            ship.manifesto.pos_z + dir_to_dest_z * 4500.0, 
            300.0 / 3.6, false, 0, 0, 0});
        ship.segments.push_back(coast);
    } else if (at_station) {
        FlightSegment coast;
        coast.type = FlightSegmentType::DepartureCoast;
        coast.waypoints.push_back({ship.manifesto.pos_x + 1500.0, ship.manifesto.pos_y, ship.manifesto.pos_z, 300.0 / 3.6, false, 0, 0, 0});
        ship.segments.push_back(coast);
    }

    // Phase: EnRoute
    target_x = dest->pos_x;
    target_y = dest->pos_y;
    target_z = dest->pos_z;
    double dx = target_x - ship.manifesto.pos_x;
    double dy = target_y - ship.manifesto.pos_y;
    double dz = target_z - ship.manifesto.pos_z;
    double dist = std::sqrt(dx*dx + dy*dy + dz*dz);

    if (dist > 500000.0) {
        double enroute_dist = dist - 500000.0;
        double dir_x = dx / dist;
        double dir_y = dy / dist;
        double dir_z = dz / dist;

        FlightSegment accel;
        accel.type = FlightSegmentType::EnRouteAccel;
        accel.waypoints.push_back({ship.manifesto.pos_x + dir_x * enroute_dist * 0.2, 
                                   ship.manifesto.pos_y + dir_y * enroute_dist * 0.2, 
                                   ship.manifesto.pos_z + dir_z * enroute_dist * 0.2, 
                                   5000000.0, false, 0, 0, 0});
        ship.segments.push_back(accel);

        FlightSegment e_coast;
        e_coast.type = FlightSegmentType::EnRouteCoast;
        e_coast.waypoints.push_back({ship.manifesto.pos_x + dir_x * enroute_dist * 0.8, 
                                     ship.manifesto.pos_y + dir_y * enroute_dist * 0.8, 
                                     ship.manifesto.pos_z + dir_z * enroute_dist * 0.8, 
                                     10000000.0, false, 0, 0, 0});
        ship.segments.push_back(e_coast);

        FlightSegment decel;
        decel.type = FlightSegmentType::EnRouteDecel;
        decel.waypoints.push_back({dest->pos_x - dir_x * 500000.0, 
                                   dest->pos_y - dir_y * 500000.0, 
                                   dest->pos_z - dir_z * 500000.0, 
                                   10000000.0, false, 0, 0, 0});
        ship.segments.push_back(decel);
    }

    // Phase: Approach (500km to 10km)
    FlightSegment approach;
    approach.type = FlightSegmentType::Approach;
    double dir_x = dx / dist;
    double dir_y = dy / dist;
    double dir_z = dz / dist;
    approach.waypoints.push_back({dest->pos_x - dir_x * 10000.0, 
                                  dest->pos_y - dir_y * 10000.0, 
                                  dest->pos_z - dir_z * 10000.0, 
                                  500.0 / 3.6, false, 0, 0, 0});
    ship.segments.push_back(approach);

    // Phase: CloseFinal
    FlightSegment close_final;
    close_final.type = FlightSegmentType::CloseFinal;
    close_final.waypoints.push_back({dest->pos_x, dest->pos_y, dest->pos_z, 50.0 / 3.6, true, 0, 0, 0});
    ship.segments.push_back(close_final);
}

void STC::update_ship_guidance(STCShipState& ship, double dt) {
    if (ship.segments.empty() || ship.current_segment_idx >= ship.segments.size()) return;

    auto& segment = ship.segments[ship.current_segment_idx];
    if (segment.waypoints.empty() || segment.current_waypoint_idx >= segment.waypoints.size()) {
        ship.current_segment_idx++;
        if (ship.current_segment_idx >= ship.segments.size()) {
            ship.current_phase = FlightPhase::None;
            return;
        }
        segment = ship.segments[ship.current_segment_idx];
    }

    // Update global phase based on segment type
    switch (segment.type) {
        case FlightSegmentType::DepartureClimb:
        case FlightSegmentType::DepartureCoast: ship.current_phase = FlightPhase::Departure; break;
        case FlightSegmentType::EnRouteAccel:
        case FlightSegmentType::EnRouteCoast:
        case FlightSegmentType::EnRouteDecel: ship.current_phase = FlightPhase::EnRoute; break;
        case FlightSegmentType::Approach: ship.current_phase = FlightPhase::Approach; break;
        case FlightSegmentType::CloseFinal: ship.current_phase = FlightPhase::CloseFinal; break;
        default: break;
    }

    const auto& wp = segment.waypoints[segment.current_waypoint_idx];
    double dx, dy, dz;
    if (wp.is_relative && !ship.parent_object_id.empty()) {
        dx = wp.rel_x - ship.rel_pos_x;
        dy = wp.rel_y - ship.rel_pos_y;
        dz = wp.rel_z - ship.rel_pos_z;
    } else {
        dx = wp.x - ship.manifesto.pos_x;
        dy = wp.y - ship.manifesto.pos_y;
        dz = wp.z - ship.manifesto.pos_z;
    }
    double dist_sq = dx*dx + dy*dy + dz*dz;

    double reach_threshold = 100.0; 
    if (ship.current_phase == FlightPhase::EnRoute) reach_threshold = 10000.0;

    if (dist_sq < reach_threshold * reach_threshold) {
        ship.last_wp_x = wp.x;
        ship.last_wp_y = wp.y;
        ship.last_wp_z = wp.z;
        segment.current_waypoint_idx++;
        // If segment finished, it will be handled on next tick or loop
        return;
    }

    double speed_m_s = std::sqrt(ship.manifesto.vel_x*ship.manifesto.vel_x + ship.manifesto.vel_y*ship.manifesto.vel_y + ship.manifesto.vel_z*ship.manifesto.vel_z);

    // Calculate current orientation from manifesto direction vector
    float current_heading = std::atan2(ship.manifesto.dir_x, ship.manifesto.dir_z) * 180.0f / 3.14159f;
    if (current_heading < 0) current_heading += 360.0f;
    float current_pitch = std::atan2(ship.manifesto.dir_y, std::sqrt(ship.manifesto.dir_x*ship.manifesto.dir_x + ship.manifesto.dir_z*ship.manifesto.dir_z)) * 180.0f / 3.14159f;

    // Speed Advisories
    STCOrder speed_order;
    speed_order.type = (speed_m_s < wp.target_speed_m_s) ? STCOrderType::AccelerateTo : STCOrderType::DecelerateTo;
    speed_order.value = static_cast<float>(wp.target_speed_m_s * 3.6);
    speed_order.description = std::string("Maintain target speed for ") + segment_type_to_string(segment.type);
    ship.pending_orders.push_back(speed_order);

    // Heading Advisories (only if not already there AND error > 5 degrees)
    double dist_xz = std::sqrt(dx*dx + dz*dz);
    if (dist_xz > 10.0) {
        float target_heading = std::atan2(static_cast<float>(dx), static_cast<float>(dz)) * 180.0f / 3.14159f;
        if (target_heading < 0) target_heading += 360.0f;
        
        float h_err = std::abs(target_heading - current_heading);
        if (h_err > 180.0f) h_err = 360.0f - h_err;

        if (h_err >= 5.0f) {
            STCOrder heading_order;
            heading_order.type = STCOrderType::TurnLeftRight;
            heading_order.value = target_heading;
            heading_order.description = "Turn to heading " + format_float(target_heading);
            ship.pending_orders.push_back(heading_order);
        }
    }

    if (std::abs(dy) > 10.0) {
        float target_pitch = std::atan2(static_cast<float>(dy), static_cast<float>(dist_xz)) * 180.0f / 3.14159f;
        float p_err = std::abs(target_pitch - current_pitch);
        
        if (p_err >= 5.0f) {
            STCOrder pitch_order;
            pitch_order.type = STCOrderType::PitchUpDown;
            pitch_order.value = target_pitch;
            pitch_order.description = "Pitch to " + format_float(target_pitch);
            ship.pending_orders.push_back(pitch_order);
        }
    }
}

void STC::check_collisions(STCShipState& ship) {
    for (const auto& obj : objects_) {
        double dx = obj.pos_x - ship.manifesto.pos_x;
        double dy = obj.pos_y - ship.manifesto.pos_y;
        double dz = obj.pos_z - ship.manifesto.pos_z;
        
        double safety_margin = obj.radius_m + 1000.0; 
        double dist_sq = dx*dx + dy*dy + dz*dz;

        double vx = ship.manifesto.vel_x;
        double vy = ship.manifesto.vel_y;
        double vz = ship.manifesto.vel_z;
        double v_sq = vx*vx + vy*vy + vz*vz;

        // Vector from ship to object center
        double dot_dv = dx*vx + dy*vy + dz*vz;

        // If we are moving away (dot product <= 0), no advisory is needed
        // (Note: dx, dy, dz is Vector(Object - Ship), so dot > 0 means moving TOWARDS object)
        if (dot_dv <= 0 && v_sq > 0.1) continue; 

        // If already inside safety margin and NOT moving away, immediate advisory
        if (dist_sq < safety_margin * safety_margin) {
            issue_collision_advisory(ship, obj);
            continue;
        }

        // Predictive check: will we collide in the next 20 seconds?
        if (v_sq < 0.1) continue; // Stationary and outside margin

        // Closest point calculation t_min = (D.V) / |V|^2
        double t_min = dot_dv / v_sq;
        
        if (t_min > 20.0) continue; // Closest point is too far in future

        // Check distance at closest point
        double closest_dx = dx - vx * t_min;
        double closest_dy = dy - vy * t_min;
        double closest_dz = dz - vz * t_min;
        double closest_dist_sq = closest_dx*closest_dx + closest_dy*closest_dy + closest_dz*closest_dz;

        if (closest_dist_sq < safety_margin * safety_margin) {
            issue_collision_advisory(ship, obj);
        }
    }
}

void STC::issue_collision_advisory(STCShipState& ship, const SystemObject& obj) {
    STCOrder order;
    order.type = STCOrderType::TurnLeftRight;
    
    // Calculate current velocity heading
    float current_heading = std::atan2(static_cast<float>(ship.manifesto.vel_x), static_cast<float>(ship.manifesto.vel_z)) * 180.0f / 3.14159f;
    order.value = current_heading + 45.0f;
    if (order.value > 360.0f) order.value -= 360.0f;

    order.description = "COLLISION ADVISORY: Predicted intercept with " + obj.id + ". Break turn to " + format_float(order.value);
    ship.pending_orders.push_back(order);
}

void STC::check_deviations(STCShipState& ship) {
    if (ship.segments.empty() || ship.current_segment_idx >= ship.segments.size()) return;
    const auto& segment = ship.segments[ship.current_segment_idx];
    if (segment.waypoints.empty() || segment.current_waypoint_idx >= segment.waypoints.size()) return;

    const auto& wp = segment.waypoints[segment.current_waypoint_idx];
    
    // Path Vector (A -> B)
    double ab_x = wp.x - ship.last_wp_x;
    double ab_y = wp.y - ship.last_wp_y;
    double ab_z = wp.z - ship.last_wp_z;
    double ab_len_sq = ab_x*ab_x + ab_y*ab_y + ab_z*ab_z;
    
    if (ab_len_sq < 1.0) return; // Leg too short

    // Ship Vector relative to start (A -> P)
    double ap_x = ship.manifesto.pos_x - ship.last_wp_x;
    double ap_y = ship.manifesto.pos_y - ship.last_wp_y;
    double ap_z = ship.manifesto.pos_z - ship.last_wp_z;

    // Cross Track Error calculation (perpendicular distance to line)
    // t = (AP . AB) / |AB|^2
    double t = (ap_x*ab_x + ap_y*ab_y + ap_z*ab_z) / ab_len_sq;
    
    // Nearest point on line
    double np_x = ship.last_wp_x + t * ab_x;
    double np_y = ship.last_wp_y + t * ab_y;
    double np_z = ship.last_wp_z + t * ab_z;
    
    double dx = ship.manifesto.pos_x - np_x;
    double dy = ship.manifesto.pos_y - np_y;
    double dz = ship.manifesto.pos_z - np_z;
    double cross_track_error = std::sqrt(dx*dx + dy*dy + dz*dz);

    double speed_m_s = std::sqrt(ship.manifesto.vel_x*ship.manifesto.vel_x + ship.manifesto.vel_y*ship.manifesto.vel_y + ship.manifesto.vel_z*ship.manifesto.vel_z);
    double allowed_error = 500.0 + (speed_m_s * 0.1);

    if (cross_track_error > allowed_error) {
        // Calculate current orientation from manifesto direction vector
        float current_heading = std::atan2(ship.manifesto.dir_x, ship.manifesto.dir_z) * 180.0f / 3.14159f;
        if (current_heading < 0) current_heading += 360.0f;
        float current_pitch = std::atan2(ship.manifesto.dir_y, std::sqrt(ship.manifesto.dir_x*ship.manifesto.dir_x + ship.manifesto.dir_z*ship.manifesto.dir_z)) * 180.0f / 3.14159f;

        // Corrective vector (P -> B)
        double pb_x = wp.x - ship.manifesto.pos_x;
        double pb_y = wp.y - ship.manifesto.pos_y;
        double pb_z = wp.z - ship.manifesto.pos_z;
        double dist_xz = std::sqrt(pb_x*pb_x + pb_z*pb_z);

        // Lateral correction if needed
        if (dist_xz > 10.0) {
            float target_heading = std::atan2(static_cast<float>(pb_x), static_cast<float>(pb_z)) * 180.0f / 3.14159f;
            if (target_heading < 0) target_heading += 360.0f;

            float h_err = std::abs(target_heading - current_heading);
            if (h_err > 180.0f) h_err = 360.0f - h_err;

            if (h_err >= 5.0f) {
                STCOrder correction;
                correction.type = STCOrderType::TurnLeftRight;
                correction.value = target_heading;
                correction.description = "ROUTE DEVIATION: Correct heading to " + format_float(target_heading);
                ship.pending_orders.push_back(correction);
            }
        }

        // Vertical correction if needed
        if (std::abs(pb_y) > 10.0) {
            float target_pitch = std::atan2(static_cast<float>(pb_y), static_cast<float>(dist_xz)) * 180.0f / 3.14159f;
            float p_err = std::abs(target_pitch - current_pitch);

            if (p_err >= 5.0f) {
                STCOrder v_corr;
                v_corr.type = STCOrderType::PitchUpDown;
                v_corr.value = target_pitch;
                v_corr.description = "ROUTE DEVIATION: Correct pitch to " + format_float(target_pitch);
                ship.pending_orders.push_back(v_corr);
            }
        }
    }
}

} // namespace galaxy
