#include "galaxylib/star_system.hpp"

#include "galaxylib/galaxy_utils.hpp"
#include "galaxylib/sector.hpp"
#include "galaxylib/star_system_tables.hpp"
#include "galaxylib/galaxy_curves.hpp"
#include "galaxylib/stc.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <random>
#include <regex>
#include <sstream>
#include <string>

namespace galaxy {

namespace {

void trim_inplace(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(0, 1);
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
}

int spectral_index_from_char(char c) {
    static constexpr char k[] = {'O', 'B', 'A', 'F', 'G', 'K', 'M'};
    for (int i = 0; i < 7; ++i) {
        if (k[i] == c) {
            return i;
        }
    }
    return -1;
}


char spectral_char(StarSpectralClass s) {
    static constexpr char k[] = {'O', 'B', 'A', 'F', 'G', 'K', 'M'};
    const int i = static_cast<int>(s);
    if (i < 0 || i >= 7) {
        return '?';
    }
    return k[i];
}

const char* luminosity_string(StarLuminosityClass L) {
    static constexpr const char* k[] = {"Ia", "Ib", "II", "III", "IV", "V", "VI", "VII"};
    const int i = static_cast<int>(L);
    if (i < 0 || i >= 8) {
        return "?";
    }
    return k[i];
}

void jingle_unique_id(std::uint64_t& uniqueid) {
    uniqueid ^= ((uniqueid >> 11) ^ (uniqueid << 21));
    uniqueid ^= ((uniqueid >> 4) ^ (uniqueid << 28));
    uniqueid ^= ((uniqueid >> 11) ^ (uniqueid << 21));
}

std::string get_common_name_a(std::uint32_t coordx, std::uint32_t coordy, int sysnum, bool two_words) {
    coordx += static_cast<std::uint32_t>(sysnum);
    coordy += coordx;
    coordx = rotl(coordx, 3);
    coordx += coordy;
    coordy = rotl(coordy, 5);
    coordy += coordx;
    coordy = rotl(coordy, 4);
    coordx = rotl(coordx, sysnum);
    coordx += coordy;

    std::uint64_t uniqueid =
        (static_cast<std::uint64_t>(sysnum) << 26) +
        (static_cast<std::uint64_t>(coordx) << 13) +
        static_cast<std::uint64_t>(coordy);
    jingle_unique_id(uniqueid);
    (void)uniqueid;

    std::string dest = tables::k_name_part_start[(coordx >> 2) & 127];
    coordx = rotr(coordx, 5);
    dest += tables::k_name_part_end[(coordx >> 2) & 255];

    if (two_words) {
        coordy = rotr(coordy, 5);
        dest += ' ';
        dest += tables::k_name_part_start[(coordy >> 2) & 127];
        coordy = rotr(coordy, 5);
        dest += tables::k_name_part_end[(coordy >> 2) & 255];
    }
    return dest;
}

std::string get_system_name_c(std::uint32_t xl, std::uint32_t yl, int sysnum) {
    std::uint64_t uniqueid =
        (static_cast<std::uint64_t>(sysnum) << 26) +
        (static_cast<std::uint64_t>(xl) << 13) +
        static_cast<std::uint64_t>(yl);
    jingle_unique_id(uniqueid);

    std::string dest;
    dest += static_cast<char>('A' + static_cast<int>(uniqueid % 26));
    uniqueid /= 26;
    dest += static_cast<char>('A' + static_cast<int>(uniqueid % 26));
    uniqueid /= 26;
    dest += static_cast<char>('A' + static_cast<int>(uniqueid % 26));
    uniqueid /= 26;

    std::ostringstream oss;
    oss << dest << '-' << std::setw(4) << std::setfill('0') << (uniqueid & 8191u);
    return oss.str();
}

}  // namespace

StarSystem::StarSystem(const Sector& sector, int sys_num, std::string_view type_code,
                         std::string_view name, int sx, int sy, int sz)
    : sys_num_(sys_num), sector_x_(sector.index_x()), sector_y_(sector.index_y()), generated_(false) {
    name_ = std::string(name);
    x_ = sx;
    y_ = sy;
    z_ = sz;
    system_id_ =
        (static_cast<std::uint64_t>(sys_num) << 26) +
        (static_cast<std::uint64_t>(sector.index_x()) << 13) +
        static_cast<std::uint64_t>(sector.index_y());

    std::string tc(type_code);
    const std::size_t plus = tc.find('+');
    const int parts = plus == std::string::npos ? 1 : static_cast<int>(std::count(tc.begin(), tc.end(), '+')) + 1;
    multiple_ = static_cast<std::uint8_t>(parts);

    build_from_known_catalog(type_code);
    mass_ = tables::k_star_mass[static_cast<int>(main_sequence_spectral_)][static_cast<int>(luminosity_class_)];
    h_zone_ = tables::k_star_hz[static_cast<int>(spectral_class_)][static_cast<int>(luminosity_class_)];
    calculate_num_bodies();

    // Determine colonization based on distance to Sol (4111, 6303).
    const Coords sol_coords(4111, 6303, 0, 0, 0);
    const float dist_ly = static_cast<float>(coordinates().distance_to(sol_coords));
    
    // Bubble Model: Smooth colonization dropoff using Exponential Decay
    // Near Sol: ~98%. Mid-range (250 LY): ~36%. Far (1000 LY): ~1.8%.
    float colonization_prob = GalaxyCurves::exponential_decay(dist_ly, 0.98f, 250.0f);

    // Boost prob for G, F, K stars.
    if (spectral_class_ == StarSpectralClass::G || spectral_class_ == StarSpectralClass::F || spectral_class_ == StarSpectralClass::K) {
        colonization_prob *= 1.5f;
    }

    const std::uint32_t col_seed = static_cast<std::uint32_t>(system_id_ >> 32);
    std::mt19937 col_gen(col_seed);
    std::uniform_real_distribution<float> col_dist(0.0f, 1.0f);
    
    if (name_ == "Sol" || col_dist(col_gen) < colonization_prob) {
        colonized_ = true;
    }

    determine_economy_type();

    // Population is calculated after bodies are generated or requested.
    // However, for total population, we can either sum it now or lazy load it.
    // Given we need population to decide if we have ports, we should probably 
    // calculate it here or in StarSystem::bodies().
}

StarSystem::StarSystem(const Sector& sector, int sys_num)
    : sys_num_(sys_num), sector_x_(sector.index_x()), sector_y_(sector.index_y()), generated_(true) {
    const auto pr = sector.get_system_params(sys_num);
    system_param_0_ = pr[0];
    system_param_1_ = pr[1];
    system_id_ =
        (static_cast<std::uint64_t>(sys_num) << 26) +
        (static_cast<std::uint64_t>(sector.index_x()) << 13) +
        static_cast<std::uint64_t>(sector.index_y());
    generate(system_param_0_, system_param_1_);
    calculate_num_bodies();

    // Determine colonization based on distance to Sol (4111, 6303).
    const Coords sol_coords(4111, 6303, 0, 0, 0);
    const float dist_ly = static_cast<float>(coordinates().distance_to(sol_coords));
    
    // Bubble Model: Smooth colonization dropoff using Exponential Decay
    // Near Sol: ~98%. Mid-range (250 LY): ~36%. Far (1000 LY): ~1.8%.
    float colonization_prob = GalaxyCurves::exponential_decay(dist_ly, 0.98f, 250.0f);

    if (spectral_class_ == StarSpectralClass::G || spectral_class_ == StarSpectralClass::F || spectral_class_ == StarSpectralClass::K) {
        colonization_prob *= 1.5f;
    }

    const std::uint32_t col_seed = static_cast<std::uint32_t>(system_id_ & 0xFFFFFFFFu);
    std::mt19937 col_gen(col_seed);
    std::uniform_real_distribution<float> col_dist(0.0f, 1.0f);
    
    if (col_dist(col_gen) < colonization_prob) {
        colonized_ = true;
    }

    determine_economy_type();
}

StarSystem::~StarSystem() = default;

StarSystem::StarSystem(StarSystem&&) noexcept = default;
StarSystem& StarSystem::operator=(StarSystem&&) noexcept = default;

void StarSystem::determine_economy_type() {
    if (!colonized_) {
        economy_type_ = SystemEconomyType::None;
        return;
    }

    if (name_ == "Sol") {
        economy_type_ = SystemEconomyType::HighTech;
        return;
    }

    const Coords sol_coords(4111, 6303, 0, 0, 0);
    const float dist_ly = static_cast<float>(coordinates().distance_to(sol_coords));

    // Calculate "Bubble Influence" (1.0 at Sol, 0.5 at 80 LY, 0.0 far away)
    float bubble_influence = GalaxyCurves::sigmoid_influence(dist_ly, 80.0f, 15.0f);

    const std::uint32_t eco_seed = static_cast<std::uint32_t>(system_id_ ^ 0xFEEDC0DEu);
    std::mt19937 gen(eco_seed);
    std::uniform_int_distribution<int> dist(1, 100);
    int roll = dist(gen);

    // Blended probability weights based on bubble influence
    // HighTech: 35% in core -> 5% in periphery
    float ht_weight = GalaxyCurves::lerp(5.0f, 35.0f, bubble_influence);
    // Industrial: 30% in core -> 5% in periphery
    float ind_weight = GalaxyCurves::lerp(5.0f, 30.0f, bubble_influence);
    // Extraction: 5% in core -> 40% in periphery
    float ext_weight = GalaxyCurves::lerp(40.0f, 5.0f, bubble_influence);
    // Mining: 5% in core -> 25% in periphery
    float min_weight = GalaxyCurves::lerp(25.0f, 5.0f, bubble_influence);

    if (roll <= ht_weight) economy_type_ = SystemEconomyType::HighTech;
    else if (roll <= ht_weight + ind_weight) economy_type_ = SystemEconomyType::Industrial;
    else if (roll <= ht_weight + ind_weight + ext_weight) economy_type_ = SystemEconomyType::Extraction;
    else if (roll <= ht_weight + ind_weight + ext_weight + min_weight) economy_type_ = SystemEconomyType::Mining;
    else if (roll <= 85) economy_type_ = SystemEconomyType::Refinery;
    else if (roll <= 92) economy_type_ = SystemEconomyType::Agricultural;
    else if (roll <= 97) economy_type_ = SystemEconomyType::Military;
    else economy_type_ = SystemEconomyType::Consumer;
}

void StarSystem::generate(std::uint64_t system_param_0, std::uint64_t system_param_1) {
    z_ = ((((static_cast<int>(system_param_0 & 0xFFFF0000u) >> 16) & 0x3FF) - 0x1FF) << 1);
    y_ = ((static_cast<int>(system_param_0 >> 8) & 0x7F) - 0x3F);
    x_ = ((static_cast<int>((system_param_0 & 0x0001FEu) >> 1) & 0x7F) - 0x3F);
    multiple_ = static_cast<std::uint8_t>(tables::k_star_chance_multiples[system_param_1 & 0x1fu]);

    age_class_ = static_cast<StarAgeClass>(tables::k_age_chance[(system_param_0 >> 2) & 0x1fu]);

    main_sequence_spectral_ = static_cast<StarSpectralClass>(
        tables::k_base_spectral_class_chance[(static_cast<int>((system_param_0 >> 4) & 0x1fu)) & 0x1fu]);

    spectral_class_ = static_cast<StarSpectralClass>(
        tables::k_age_spectral_evolution[static_cast<int>(main_sequence_spectral_)][static_cast<int>(age_class_)]);

    luminosity_class_ = static_cast<StarLuminosityClass>(
        tables::k_age_luminosity_evolution[static_cast<int>(main_sequence_spectral_)][static_cast<int>(age_class_)]);

    mass_ = tables::k_star_mass[static_cast<int>(spectral_class_)][static_cast<int>(luminosity_class_)];
    magnitude_ = tables::k_star_magnitude[static_cast<int>(spectral_class_)][static_cast<int>(luminosity_class_)];
    h_zone_ = tables::k_star_hz[static_cast<int>(spectral_class_)][static_cast<int>(luminosity_class_)];

    {
        std::ostringstream oss;
        oss << spectral_char(spectral_class_) << magnitude_ << luminosity_string(luminosity_class_);
        star_type_ = oss.str();
    }

    const int li = static_cast<int>(luminosity_class_);
    if (li <= 4) {
        name_ = get_common_name_a(sector_x_, sector_y_, sys_num_, true);
    } else if (li < 6) {
        name_ = get_common_name_a(sector_x_, sector_y_, sys_num_, false);
    } else {
        name_ = get_system_name_c(sector_x_, sector_y_, sys_num_);
    }
}

void StarSystem::build_from_known_catalog(std::string_view type_code_sv) {
    std::string first;
    {
        std::string tc(type_code_sv);
        const std::size_t plus = tc.find('+');
        first = plus == std::string::npos ? tc : tc.substr(0, plus);
    }
    trim_inplace(first);
    if (first.empty()) {
        spectral_class_ = StarSpectralClass::G;
        main_sequence_spectral_ = spectral_class_;
        luminosity_class_ = StarLuminosityClass::V;
        magnitude_ = 5;
        star_type_ = std::string(1, 'G') + "5" + luminosity_string(luminosity_class_);
        return;
    }

    const int si = spectral_index_from_char(first[0]);
    std::regex lum_re("[IVXab]+");
    std::smatch lm;
    if (si < 0 || !std::regex_search(first, lm, lum_re)) {
        spectral_class_ = StarSpectralClass::G;
        main_sequence_spectral_ = spectral_class_;
        luminosity_class_ = StarLuminosityClass::V;
        magnitude_ = 5;
        star_type_ = "G5V";
        return;
    }

    const std::string lum_part = lm[0].str();
    static constexpr const char* k_lum_names[] = {"Ia", "Ib", "II", "III", "IV", "V", "VI", "VII"};
    int lj = -1;
    for (int j = 0; j < 8; ++j) {
        if (lum_part == k_lum_names[j]) {
            lj = j;
            break;
        }
    }
    if (lj < 0) {
        spectral_class_ = StarSpectralClass::G;
        main_sequence_spectral_ = spectral_class_;
        luminosity_class_ = StarLuminosityClass::V;
        magnitude_ = 5;
        star_type_ = "G5V";
        return;
    }

    std::string mag_str = std::regex_replace(first, std::regex("[A-Za-z\\-]+"), std::string(""));
    try {
        magnitude_ = mag_str.empty() ? 0 : static_cast<int>(std::lround(std::stof(mag_str)));
    } catch (...) {
        magnitude_ = 0;
    }

    main_sequence_spectral_ = static_cast<StarSpectralClass>(si);
    spectral_class_ = main_sequence_spectral_;
    luminosity_class_ = static_cast<StarLuminosityClass>(lj);

    {
        std::ostringstream oss;
        oss << spectral_char(spectral_class_) << magnitude_ << luminosity_string(luminosity_class_);
        star_type_ = oss.str();
    }
}

void StarSystem::calculate_num_bodies() {
    if (!generated_) {
        if (name_ == "Sol") {
            num_bodies_ = 9;
            return;
        }
    }
    const std::uint32_t seed = static_cast<std::uint32_t>(system_id_ & 0xFFFFFFFFu);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(1, 13);
    num_bodies_ = dist(gen);
}

void StarSystem::ensure_bodies() const {
    if (bodies_cached_) {
        return;
    }
    bodies_.clear();
    bodies_.reserve(static_cast<std::size_t>(num_bodies_));
    
    float total_pop = 0.0f;
    for (int i = 0; i < num_bodies_; ++i) {
        bodies_.emplace_back(*this, i);
        total_pop += bodies_.back().population_millions();
    }
    
    const_cast<StarSystem*>(this)->population_billions_ = total_pop / 1000.0f;
    bodies_cached_ = true;
}

const std::vector<SystemBody>& StarSystem::bodies() const {
    ensure_bodies();
    return bodies_;
}

void StarSystem::update(double elapsed_days) {
    ensure_bodies();
    for (auto& body : bodies_) {
        body.update_system_location(elapsed_days);
    }
}

void StarSystem::update(CalendarDate date) {
    update(days_since_orbit_reference(date));
}

float StarSystem::population_billions() const {
    ensure_bodies();
    return population_billions_;
}

STC& StarSystem::stc() {
    if (!stc_) {
        stc_ = std::make_unique<STC>(*this);
        stc_->init();
    }
    return *stc_;
}

}  // namespace galaxy
