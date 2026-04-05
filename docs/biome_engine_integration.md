# Integration Guide: High-Performance Biome Rendering

This document provides architectural strategies for 3D engines to efficiently utilize the `get_biome_at(lat, lon)` logic from the Galaxy library. To maintain high frame rates, avoid calling this CPU logic per-pixel or per-frame during the rendering pass.

## Strategy 1: The Biome Lookup Texture (Baking)

This is the most recommended approach for modern 3D engines.

1.  **Baking Phase:** When a planet is loaded, iterate through a 2D grid (e.g., 1024x512) and call `get_biome_at` for each coordinate.
2.  **Storage:** Store the result in a low-resolution **Data Texture** (R8 format is sufficient, where the red channel represents the `BiomeType` enum index).
3.  **GPU Usage:** In your fragment shader, sample this texture to determine which "Splat Map" textures (grass, sand, rock, snow) to blend.
4.  **Performance:** This shifts the cost from a per-frame CPU calculation to a one-time setup cost. GPU texture sampling is extremely fast and hardware-accelerated.

## Strategy 2: Vertex-Level Assignment (Procedural Mesh)

Ideal if your engine uses a Quad-Sphere or Geodesic Sphere for planet geometry.

1.  **Generation Phase:** During the procedural generation of the planet's mesh, call `get_biome_at` for every **Vertex Coordinate**.
2.  **Attribute Storage:** Store the biome index (or a normalized weight) in a **Vertex Attribute** (e.g., `TEXCOORD1` or `COLOR`).
3.  **Displacement:** Use this vertex data in your Vertex Shader to apply different height displacement maps (e.g., Mountains get high displacement, Plains stay flat).
4.  **Performance:** The GPU automatically interpolates these values across the surface of the triangles, making the biome data available "for free" in the fragment shader.

## Strategy 3: GPU-Side Logic (Compute Shaders)

If you require "infinite" detail or real-time biome transitions without texture memory overhead.

1.  **Data Transfer:** Pass the `PlanetZone` array for the current planet into a **Uniform Buffer (UBO)** or **Structured Buffer (SSBO)**.
2.  **Shader Implementation:** Implement the distance-influence logic directly in GLSL/HLSL.
3.  **Haversine Math:** Use dot products to calculate the spherical distance between the fragment's normal vector and the zone center vectors.
4.  **Performance:** Since most planets have fewer than 10-15 zones, the GPU can calculate this influence for millions of pixels in parallel with negligible impact on performance.

## Strategy 4: Gameplay & Physics Caching

For non-visual logic (e.g., "Is the player taking heat damage from a Volcanic biome?"):

1.  **Distance Threshold:** Only re-query `get_biome_at` when the player or object moves a significant distance (e.g., > 10 meters).
2.  **State Caching:** Store the current biome in the entity's state. 
3.  **Rate Limiting:** Run biome-dependent logic (like temperature effects or resource scanning) on a slower "Think" tick (e.g., 5-10 times per second) rather than every frame.

---

### Implementation Tip: Spherical Coordinates
The library uses `float lat` (-90 to 90) and `float lon` (-180 to 180). Ensure your engine's UV mapping or spherical-to-cartesian conversions align with these ranges to avoid "seams" at the poles or the 180/-180 meridian.
