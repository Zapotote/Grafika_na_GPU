# Particle System Implementation - GPU Graphics Course

## Overview

A particle-only OpenGL scene for the GPU graphics course project. The system uses CPU-side particle simulation and GPU-instanced billboard rendering to display several particle presets: fire, bubbles, snow, rain, smoke, mist, sparks, fountain, explosion, lightning, and magic.

The demo starts with the fire effect. There are no solid objects in the scene.

## Features Implemented

### 1. Particle Emitter (`ParticleEmitter` class)

- Per-frame particle emission with configurable emission rate.
- Configurable particle lifetime.
- Per-effect emission behavior instead of a single shared cone emitter.
- Automatic removal of dead particles to maintain performance.
- Per-particle physical properties:
  - Position
  - Velocity
  - Lifetime and normalized age
  - Size
  - Stretch
  - Rotation
  - Color
  - Shape/mask type
- Effect-specific acceleration, drag, turbulence, and emission placement.

### 2. Particle Presets

| Key | Particle effect | Description |
| --- | --- | --- |
| `1` | Fire | Red outer flame particles and yellow inner flame particles emitted from the bottom of the window. |
| `2` | Bubbles | Sparse, slow-rising bath-like bubbles with pale soft colors. |
| `3` | Snow | Small white particles falling gently with light drift. |
| `4` | Rain | Fast blue stretched drops. |
| `5` | Smoke | Soft gray particles with natural fading. |
| `6` | Mist | Large slow white/light-gray particles with very soft edges. |
| `7` | Sparks | Bright short-lived particles shooting outward. |
| `8` | Fountain | Darker light-blue water drops moving upward and sideways before falling with gravity. |
| `9` | Explosion | Fast outward burst of hot particles. |
| `0` | Lightning | Connected bolt segments with small side branches and bright inner glow. |
| `M` | Magic | Colorful glowing particles. |
| `Enter` | Reset camera | Restores the camera position. |

### 3. Particle Rendering System

- **GPU Instancing**: Uses instanced rendering for efficient batch processing.
- **Billboard Geometry**: One quad per particle, facing the camera.
- **Dynamic Vertex Buffers**: Particle instance data is updated each frame.
- **Per-particle transform data**:
  - Position
  - Size
  - Stretch
  - Rotation
- **Per-particle appearance data**:
  - Normalized age
  - Color
  - Shape/mask selector
  - Maximum age

### 4. Shaders

#### Vertex Shader (`particle.vertex.glsl`)

- Generates camera-facing billboards.
- Applies per-particle size and stretch.
- Applies per-particle rotation for drops, flames, sparks, and lightning segments.
- Passes particle age, color, texture coordinates, and shape type to the fragment shader.

#### Fragment Shader (`particle.fragment.glsl`)

- Uses procedural masks instead of external particle textures.
- Supports multiple particle shapes:
  - Soft circular particles for bubbles, snow, smoke, and magic.
  - Smooth mist mask with very soft edges.
  - Stretched flame mask for fire.
  - Drop-like masks for rain and fountain.
  - Thin streak masks for sparks and lightning.
- Uses alpha blending for transparency and soft particle overlap.
- Adds effect-specific color behavior, such as brighter lightning cores and warm flame colors.

### 5. Rendering Integration

- Blending enabled with `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`.
- Depth testing remains enabled.
- Particles are rendered as transparent instanced billboards.
- Real-time delta time tracking is used for smooth particle movement.
- Keyboard input switches the active particle preset.

## Key Implementation Details

### Particle Data

The current particle instance data is richer than the original fire/smoke-only version. It stores both simulation state and render parameters.

```cpp
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float age;
    float maxAge;
    float size;
    float stretch;
    float shape;
    glm::vec3 color;
    float rotation;
};
```

Instance data sent to the GPU contains the values needed by the shaders for each billboard:

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

### Particle Physics

- Each effect chooses its own starting position, velocity, lifetime, size, and color.
- Gravity-like acceleration is used for falling effects such as rain, snow, and fountain drops.
- Drag slows effects such as smoke, mist, bubbles, and sparks.
- Turbulence adds small natural movement where appropriate.
- Lightning is generated as short connected segment chains rather than independent drifting particles.

### Effect Behavior Notes

#### Fire

Fire emits from the bottom of the window and uses two particle layers:

- Red outer particles start wider, more toward the sides, and usually rise higher.
- Yellow inner particles are more central and rise high, but not as high as the red outer layer.

The fire mask is stretched and sharper than smoke, so it reads more like flame and less like round bubbles.

#### Mist

Mist uses large, slow particles with a very soft procedural mask. The goal is to make particle edges blend into each other so the viewer does not see clear rectangular or circular particle boundaries.

#### Smoke

Smoke uses gray colors and a soft round mask. It avoids green/brown tinting and avoids hard wave-like outlines.

#### Fountain

Fountain particles are darker light-blue drops. They move upward and outward to the sides, then fall under gravity.

#### Lightning

Lightning is emitted as one connected bolt at a time. A bolt is built from connected line segments, with occasional short side branches. Segment endpoints continue from the previous segment so the bolt does not appear as disconnected parallel lines.

## Buffer Management

- VBO[0]: Quad geometry (static)
- VBO[1]: Index buffer (static)
- VBO[2]: Instance data (dynamic, updated every frame)
- `GL_DYNAMIC_DRAW` is used for the instance buffer because particle data changes every frame.

## Vertex Attributes

| Location | Attribute | Usage |
| --- | --- | --- |
| 0 | `in_vert` | Quad vertex position |
| 1 | `in_normal` | Quad normal |
| 2 | `in_texCoord` | Quad texture coordinates |
| 3 | `in_position` | Instance particle position |
| 4 | `in_age` | Particle age |
| 5 | `in_color` | Particle color |
| 6 | `in_maxAge` | Particle lifetime |
| 7 | `in_size` | Particle size |
| 8 | `in_stretch` | Particle stretch multiplier |
| 9 | `in_shape` | Procedural shape selector |
| 10 | `in_rotation` | Particle rotation |

## Files

### C++ Files

- `particle_system.hpp` - Particle data, preset configuration, emission logic, simulation update, buffers, and draw call.
- `main.cpp` - Application setup, camera handling, keyboard controls, and active effect switching.
- `renderer.hpp` - Rendering state integration.

### GLSL Shaders

- `shaders/particle.vertex.glsl` - Billboard generation, particle transform, rotation, stretching, and varying output.
- `shaders/particle.fragment.glsl` - Procedural particle masks, transparency, glow, and color shaping.
- `shaders/particle.program` - Shader program configuration.

### Build Configuration

- `00_Particle_system/CMakeLists.txt` - Project configuration for the particle system demo.

## Usage

### Controls

| Key | Action |
| --- | --- |
| `1` | Fire |
| `2` | Bubbles |
| `3` | Snow |
| `4` | Rain |
| `5` | Smoke |
| `6` | Mist |
| `7` | Sparks |
| `8` | Fountain |
| `9` | Explosion |
| `0` | Lightning |
| `M` | Magic |
| `Enter` | Reset camera |
| Left mouse drag | Orbit camera |

## Performance Characteristics

### Memory Usage

- Per particle: position, velocity, age, lifetime, size, stretch, shape, color, and rotation.
- Instance buffer is rebuilt from living particles each frame.
- Particle count depends on the active preset's emission rate and lifetime.

### GPU Efficiency

- One instanced draw call renders all living particles.
- Billboard quads keep geometry cost low.
- Procedural masks avoid texture loading and texture memory.
- Per-particle instance attributes allow many visual variations without changing meshes.

## Future Enhancements

1. GPU particle simulation with transform feedback or compute shaders.
2. Texture-based particle animation with sprite sheets.
3. Depth sorting for improved transparency.
4. Multiple simultaneous emitters.
5. Particle collision against scene geometry.
6. Preset editor UI for emission rate, lifetime, size, gravity, and color.
7. Optional additive blending for sparks, magic, fire, and lightning.

## Technical Notes

### Why Instancing?

- Reduces draw call overhead.
- Keeps particle rendering efficient as particle count grows.
- Allows each particle to use a different transform and appearance.

### Why Billboards?

- They face the camera automatically.
- They require only four vertices per particle.
- They are well suited for transparent effects such as smoke, mist, fire, rain, and sparks.

### Why Procedural Masks?

- No external texture files are required.
- Each effect can have a custom look directly in the fragment shader.
- Smooth masks make smoke and mist blend better.
- Streak and flame masks allow effects such as lightning, sparks, rain, and fire to avoid looking like bubbles.

## Building

```bash
cmake --build cmake-build-debug-visual-studio --target 00_Particle_system
```

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
