# Particle System

## Assignment Description

The goal of this project was to implement a simple particle system representing several visual effects such as fire, smoke, rain, snow, sparks, fountain, lightning, and magic.

Particles are rendered as GPU-instanced billboards. Each particle is simulated on the CPU and then rendered as a camera-facing sprite on the GPU.

The implementation is based on the shader-oriented rendering structure from the course examples.

---

## Implemented Features

### Particle Simulation

The particle system supports per-frame particle emission and update.

Each particle stores:

```cpp
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float age;
    float maxAge;
    float size;
    float stretch;
    float shape;
    float rotation;
    glm::vec3 color;
};
```

Particles are updated using:

* Velocity
* Acceleration
* Drag
* Turbulence
* Lifetime-based removal

Dead particles are automatically removed from the system.

---

## Particle Effects

The project implements multiple particle presets:

| Key | Effect    |
| --- | --------- |
| `1` | Fire      |
| `2` | Bubbles   |
| `3` | Snow      |
| `4` | Rain      |
| `5` | Smoke     |
| `6` | Mist      |
| `7` | Sparks    |
| `8` | Fountain  |
| `9` | Explosion |
| `0` | Lightning |
| `M` | Magic     |

Each effect has its own emission rate, lifetime, particle size, velocity, acceleration, drag, color, and behavior. While some effects use custom shapes, four of the effects basically reuse the same basic procedural sprite shape. Most of the variants come to an existence simply because it was fun to try different settings and see how they look.

---

## Rendering

Particles are rendered using instanced billboard quads.

A single quad is reused for all particles, and per-particle data is sent to the GPU as instance attributes.

The instance data contains:

```cpp
struct ParticleInstanceData {
    glm::vec3 position;
    float age;
    glm::vec3 color;
    float maxAge;
    float size;
    float stretch;
    float shape;
    float rotation;
};
```

The vertex shader rotates each quad so that it faces the camera. It also applies particle size, stretch, and rotation.

The fragment shader creates the particle appearance procedurally using masks instead of external textures.

---

## Controls

| Key             | Function                      |
| --------------- | ----------------------------- |
| `1`             | Switch to fire                |
| `2`             | Switch to bubbles             |
| `3`             | Switch to snow                |
| `4`             | Switch to rain                |
| `5`             | Switch to smoke               |
| `6`             | Switch to mist                |
| `7`             | Switch to sparks              |
| `8`             | Switch to fountain            |
| `9`             | Switch to explosion           |
| `0`             | Switch to lightning           |
| `M`             | Switch to magic               |
| `Enter`         | Reset camera                  |
| Left Mouse Drag | Orbit camera around the scene |

---

## Files

### Added / Modified Files

* `particle_system.hpp`

  * Particle data structures
  * Particle emitter
  * Particle effect presets
  * Particle simulation
  * Instanced billboard buffer generation
  * Particle system scene object

* `renderer.hpp`
* `main.cpp`

* `shaders/particle.vertex.glsl`

  * Billboard generation
  * Per-particle size, stretch, and rotation
  * Passing particle data to the fragment shader

* `shaders/particle.fragment.glsl`

  * Procedural particle masks
  * Alpha fading
  * Color and glow behavior

* `shaders/particle.program`

---

## AI Assistance

Artificial Intelligence tools were used as a development aid.

The main uses were:

* Debugging assistance during implementation
* Help with organizing the particle effect structure
* Help with formatting and structuring this documentation

The implementation, integration, testing, and final project decisions were performed manually by the author - once again me.
