#pragma once

#include "galaxylib/coords.hpp"
#include "galaxylib/system_body.hpp"
#include <memory>

namespace galaxy {

class Sector;
class STC;

/// Mirrors `GalaxyE.Galaxy.StarSpectralClass` / `StarLuminosityClass` / `StarAgeClass`.
enum class StarSpectralClass : int { O = 0, B = 1, A = 2, F = 3, G = 4, K = 5, M = 6 };

enum class StarLuminosityClass : int {
    Ia = 0,
    Ib = 1,
    II = 2,
    III = 3,
    IV = 4,
    V = 5,
    VI = 6,
    VII = 7
};

enum class StarAgeClass : int { MainSequence = 0, MainEndSequence = 1, Giant = 2, Dead = 3 };

/// Economy focus of a star system.
enum class SystemEconomyType : int {
    None = 0,
    Industrial = 1,
    Agricultural = 2,
    Mining = 3,
    HighTech = 4,
    Military = 5,
    Consumer = 6,
    Refinery = 7,
    Extraction = 8
};

/// Procedural star system (`StarSystem.cs` internal constructor + `Generate`).
class StarSystem {
public:
    /// Builds one generated system (`Generate_Systems` loop).
    StarSystem(const Sector& sector, int sys_num);

    /// Catalogue / known system (`StarSystem` ctor with `typeCode` in C#).
    StarSystem(const Sector& sector, int sys_num, std::string_view type_code, std::string_view name,
               int sx, int sy, int sz);

    ~StarSystem();

    StarSystem(StarSystem&&) noexcept;
    StarSystem& operator=(StarSystem&&) noexcept;

    // Delete copy operations as they are incompatible with unique_ptr
    StarSystem(const StarSystem&) = delete;
    StarSystem& operator=(const StarSystem&) = delete;

    int sys_num() const { return sys_num_; }
    std::uint64_t system_id() const { return system_id_; }

    const std::string& name() const { return name_; }
    const std::string& star_type() const { return star_type_; }

    StarSpectralClass spectral_class() const { return spectral_class_; }
    StarLuminosityClass luminosity_class() const { return luminosity_class_; }
    int magnitude() const { return magnitude_; }
    float mass() const { return mass_; }
    float habitable_zone_au() const { return h_zone_; }

    int local_x() const { return x_; }
    int local_y() const { return y_; }
    int local_z() const { return z_; }

    /// Major bodies (planets/moons); count only — cheap; safe for regional maps listing many systems.
    /// Matches C# `NumberOfBodies` / `calculate_num_bodies`.
    int num_bodies() const { return num_bodies_; }

    /// Aggregated population of all bodies in billions.
    float population_billions() const;
    /// True if the system is colonized.
    bool is_colonized() const { return colonized_; }

    SystemEconomyType economy_type() const { return economy_type_; }

    /// Builds and caches `SystemBody` instances on first call — use only after the player selects this system
    /// (or when you need orbits); avoid during coarse “sector / region” map passes.
    const std::vector<SystemBody>& bodies() const;

    /// Central simulation update. Calculates positions for all planets and stations at the given time.
    void update(double elapsed_days);
    void update(CalendarDate date);

    STC& stc();

    std::uint32_t sector_index_x() const { return sector_x_; }
    std::uint32_t sector_index_y() const { return sector_y_; }

    /// `Coords` for this system (`BuildCoords` in C#).
    Coords coordinates() const { return Coords(sector_x_, sector_y_, x_, y_, z_); }

    /// False for real-star catalogue entries (`KnownSpace`).
    bool is_generated() const { return generated_; }

private:
    void generate(std::uint64_t system_param_0, std::uint64_t system_param_1);
    void calculate_num_bodies();
    void build_from_known_catalog(std::string_view type_code);
    void ensure_bodies() const;
    void determine_economy_type();

    int sys_num_{};
    std::uint64_t system_id_{};
    std::uint64_t system_param_0_{};
    std::uint64_t system_param_1_{};

    StarAgeClass age_class_{};
    StarSpectralClass main_sequence_spectral_{};
    StarSpectralClass spectral_class_{};
    StarLuminosityClass luminosity_class_{};

    int magnitude_{};
    float mass_{};
    float h_zone_{};

    int x_{};
    int y_{};
    int z_{};

    std::uint8_t multiple_{};

    std::string name_;
    std::string star_type_;

    int num_bodies_{};

    std::uint32_t sector_x_{};
    std::uint32_t sector_y_{};

    bool colonized_{false};
    float population_billions_{0.0f};
    SystemEconomyType economy_type_{SystemEconomyType::None};

    bool generated_{true};

    mutable std::vector<SystemBody> bodies_;
    mutable bool bodies_cached_{false};

    mutable std::unique_ptr<STC> stc_;
};

}  // namespace galaxy
