#include "galaxylib/base36.hpp"
#include "galaxylib/galaxy_api.hpp"
#include "galaxylib/sector.hpp"
#include "galaxylib/star_date.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef GALAXYLIB_ASSETS_DIR
#define GALAXYLIB_ASSETS_DIR "."
#endif

namespace {

void galaxy_test_terminate_handler() {
    std::cerr << "std::terminate() — uncaught exception, double-throw, or noexcept violation\n";
    try {
        const std::exception_ptr ep = std::current_exception();
        if (ep) {
            std::rethrow_exception(ep);
        }
    } catch (const std::exception& e) {
        std::cerr << "what(): " << e.what() << '\n';
    } catch (...) {
        std::cerr << "non-standard exception type\n";
    }
    std::abort();
}

std::vector<std::string> galaxy_bin_path_candidates() {
    std::vector<std::string> paths;
    paths.push_back(std::string(GALAXYLIB_ASSETS_DIR) + "/Galaxy.bin");
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) != 0) {
        std::filesystem::path exe(buf);
        paths.push_back((exe.parent_path() / "Galaxy.bin").string());
    }
#endif
    return paths;
}

}  // namespace

int main() {
    std::set_terminate(galaxy_test_terminate_handler);
    try {
    const auto ver = galaxy::api::library_version();
    std::cout << "GalaxyLib " << ver.major << '.' << ver.minor << '.' << ver.patch << '\n';

    bool loaded = false;
    std::string tried;
    for (const std::string& galaxy_path : galaxy_bin_path_candidates()) {
        if (!tried.empty()) {
            tried += "; ";
        }
        tried += galaxy_path;
        if (galaxy::api::initialize({galaxy_path})) {
            loaded = true;
            break;
        }
    }
    if (!loaded) {
        std::cerr << "Failed to load galaxy data (tried: " << tried << ")\n";
        return 1;
    }

    const galaxy::StarDate sd_2012 = galaxy::StarDate::from_earth_utc(2012, 1, 1);
    std::cout << "StarDate (Earth 2012-01-01 UTC -> in-universe): " << sd_2012.to_string() << "  s_date=" << sd_2012.s_date()
              << '\n';

    // Same sector as C# TestApp: new Sector((uint)Base36.Decode("368"), (uint)Base36.Decode("4V5"))
    const std::uint32_t sx = static_cast<std::uint32_t>(galaxy::base36_decode("368"));
    const std::uint32_t sy = static_cast<std::uint32_t>(galaxy::base36_decode("4V5"));

    const galaxy::Sector sector(sx, sy);

    std::cout << "Sector (decimal): " << sx << ", " << sy << '\n';
    std::cout << "Sector (base36):  " << sector.to_string_base36() << '\n';
    std::cout << "Num systems:      " << sector.num_systems() << '\n';

    const auto& systems = sector.systems();
    for (std::size_t i = 0; i < systems.size(); ++i) {
        const auto& s = systems[i];
        const galaxy::Coords pos = s.coordinates();
        const galaxy::Coords sol_coords(4111, 6303, 0, 0, 0);
        float dist = static_cast<float>(pos.distance_to(sol_coords));
        float influence = galaxy::GalaxyCurves::sigmoid_influence(dist, 80.0f, 15.0f);

        std::cout << "  [" << i << "] " << s.name() << " (" << dist << " LY) influence=" << influence
                  << " economy=" << static_cast<int>(s.economy_type()) << " pop=" << s.population_billions() << "B\n";
        const auto& bodies = s.bodies();
        for (std::size_t b = 0; b < bodies.size() && b < 20; ++b) {
            const auto& p = bodies[b];
            std::cout << "      orbit " << b << ": " << p.name() << "  " << p.body_class_ext()
                      << "  " << p.orbital_radius_au() << " AU  temp " << p.temperature_c() << " C  pop=" 
                      << p.population_millions() << "M\n";
            for (const auto& port : p.ports()) {
                if (port.type == galaxy::SpacePortType::SurfacePort) {
                    std::cout << "          [Port] " << port.name << " (" << port.latitude << ", " << port.longitude << ")\n";
                } else {
                    double rx, ry, rz;
                    // Current date
                    port.get_relative_location(4383.0, 6371.0f * static_cast<float>(p.radius()), rx, ry, rz);
                    std::cout << "          [Station] " << port.name << " (Alt: " << port.altitude_km << "km, Period: " << port.orbital_period_hours << "h, Rel: " << rx/1000.0 << "," << ry/1000.0 << "km)\n";
                    // 1 hour later
                    port.get_relative_location(4383.0 + (1.0/24.0), 6371.0f * static_cast<float>(p.radius()), rx, ry, rz);
                    std::cout << "            (+1h): " << rx/1000.0 << "," << ry/1000.0 << "km\n";
                }
            }
        }
        if (bodies.size() > 20) {
            std::cout << "      ... (" << (bodies.size() - 20) << " more)\n";
        }
    }

    if (!systems.empty()) {
        galaxy::Coords roundtrip;
        if (galaxy::Coords::try_parse(systems[0].coordinates().to_string(), roundtrip, true)) {
            std::cout << "TryParse round-trip: "
                      << (roundtrip == systems[0].coordinates() ? "ok" : "mismatch") << '\n';
        }
    }

    std::cout << "\n--- Sol sector (catalogue) ---\n";
    const galaxy::Sector sol(4111u, 6303u);
    std::cout << "Procedural density count: " << sol.num_systems() << '\n';
    const auto& sol_sys = sol.systems();
    std::cout << "Systems in sector: " << sol_sys.size() << '\n';
    for (std::size_t i = 0; i < sol_sys.size(); ++i) {
        auto s = sol_sys[i]; // Not a const reference so we can update it
        s.update(4383.0);    // Update the entire system at once (2024 epoch)

        std::cout << "  [" << i << "] " << s.name() << "  (" << s.star_type() << ")  generated="
                  << (s.is_generated() ? "yes" : "no") << "  bodies " << s.num_bodies() << '\n';
        if (s.name() == "Sol") {
            for (const auto& p : s.bodies()) {
                std::cout << "      " << p.name() << "  " << p.body_class_ext() << "  "
                          << p.orbital_radius_au() << " AU  period " << p.orbital_period_years()
                          << " y  pop " << p.population_millions() << "M\n";
                for (const auto& port : p.ports()) {
                    if (port.type == galaxy::SpacePortType::SurfacePort) {
                        std::cout << "          [Port] " << port.name << " (" << port.latitude << ", " << port.longitude << ")\n";
                    } else {
                        // Position was already calculated by s.update()
                        std::cout << "          [Station] " << port.name << " (Alt: " << port.altitude_km << "km, Rel: " << port.rel_x_m/1000.0 << "," << port.rel_y_m/1000.0 << "km)\n";
                    }
                }
            }
        }
    }

    return 0;
    } catch (const std::exception& e) {
        std::cerr << "galaxy_test error: " << e.what() << '\n';
        return 3;
    } catch (...) {
        std::cerr << "galaxy_test: unknown error\n";
        return 3;
    }
}
