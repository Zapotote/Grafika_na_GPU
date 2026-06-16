# Depth of Field

## Assignment Description

The goal of this project was to implement a Depth of Field (DoF) post-processing effect for a deferred renderer. The effect simulates camera focus by keeping objects near a selected focus distance sharp while progressively blurring objects outside the focus range.

The implementation is based on the deferred rendering example provided in the course materials.

---

## Implemented Features

### Deferred Rendering

The renderer uses a standard deferred rendering pipeline consisting of:

1. Shadow map pass
2. Geometry pass (G-buffer generation)
3. Deferred compositing pass
4. Depth of Field post-processing pass

The G-buffer stores:

* Diffuse color
* Surface normals
* World-space position
* Linear camera depth

### Intermediate Compositing Buffer

Instead of rendering the composited image directly to the screen, the compositing pass renders into a separate framebuffer texture.

This texture is then used as the input for the final Depth of Field post-processing pass.

### Depth of Field Effect

The DoF effect is implemented as a fullscreen post-processing shader.

For each fragment:

* Linear depth is read from the G-buffer.
* Distance from the focus plane is computed.
* Blur amount is calculated using the focus distance and focus range.
* Fragments inside the focus range remain sharp.
* Fragments outside the focus range are blurred using a 16-sample blur kernel.

An optional debug mode visualizes the blur intensity directly.

---

## Focus Calculation

```glsl
float focusError = max(abs(linearDepth - u_focusDistance) - u_focusRange, 0.0);
float blurAmount = clamp(focusError / u_focusRange, 0.0, 1.0);
```

Fragments close to the focus distance remain sharp, while fragments further away receive progressively stronger blur.

---

## Modified Files

### Added

* `shaders/dof.fragment.glsl`
* `shaders/dof.program`

### Modified

* `renderer.hpp`
* `shaders/material_deffered.fragment.glsl`
* `shaders/compositing.fragment.glsl`

---

## Controls

| Key             | Function                                |
| --------------- | --------------------------------------- |
| Left Mouse Drag | Orbit camera around the scene           |
| Enter           | Reset camera position                   |
| Q / E           | Decrease / increase focus distance      |
| Z / X           | Decrease / increase focus range         |
| C / V           | Decrease / increase maximum blur radius |
| B               | Toggle DoF debug view                   |

---

## Default Parameters

| Parameter           | Default Value |
| ------------------- | ------------- |
| Focus Distance      | 50.0          |
| Focus Range         | 2.5           |
| Maximum Blur Radius | 18.0          |

The default values were intentionally chosen to produce a strong and clearly visible Depth of Field effect. They are somewhat more aggressive than would typically be used in a real application, but they make the effect easier to observe and evaluate during demonstration and testing.

The parameters can be adjusted during runtime using the keyboard controls listed above.

---

## Performance Notes

The effect adds one additional fullscreen rendering pass after deferred compositing.

The blur shader performs:

* 16 scene texture samples per pixel
* Depth lookup from the G-buffer

The performance cost therefore scales primarily with screen resolution.

---

## Building

```bash
cmake --build build --config Debug --target 00_DoF
```

---

## Use of AI

Artificial Intelligence tools were used primarily as a development aid.

The main uses were:

* Debugging assistance during implementation.
* Verification of OpenGL and GLSL related issues.
* Formatting and structuring of documentation.

The design, implementation, integration into the renderer, and final testing were performed manually by the author - me.
