#pragma once

#include <cmath>
#include <algorithm>

namespace galaxy {

/**
 * @brief Mathematical curves used to drive galactic distribution logic.
 * 
 * These functions provide smooth, continuous transitions for population, 
 * resource availability, and economy types based on distance from Sol.
 */
class GalaxyCurves {
public:
    /**
     * @brief Standard Exponential Decay.
     * Starts at max_val and drops towards 0 as distance increases.
     * Used for: Population density, Colonization probability.
     */
    static float exponential_decay(float distance, float max_val, float falloff_rate) {
        if (falloff_rate <= 0.0f) return 0.0f;
        return max_val * std::exp(-distance / falloff_rate);
    }

    /**
     * @brief Inverse Exponential (Growth).
     * Starts at min_val and rises towards max_val as distance increases.
     * Used for: Raw material availability.
     */
    static float inverse_exponential(float distance, float min_val, float max_val, float growth_rate) {
        if (growth_rate <= 0.0f) return min_val;
        return min_val + (max_val - min_val) * (1.0f - std::exp(-distance / growth_rate));
    }

    /**
     * @brief Sigmoid / S-Curve (Logistic Function).
     * Returns a weight between 1.0 and 0.0.
     * Returns 1.0 near the center, 0.5 at the midpoint, and 0.0 far away.
     * Used for: Blending between Core and Periphery economy weights.
     */
    static float sigmoid_influence(float distance, float midpoint, float sharpness) {
        if (sharpness <= 0.0f) return distance < midpoint ? 1.0f : 0.0f;
        // Standard logistic function: 1 / (1 + e^(k * (x - x0)))
        return 1.0f / (1.0f + std::exp((distance - midpoint) / sharpness));
    }

    /**
     * @brief Linear remap helper.
     */
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
};

} // namespace galaxy
