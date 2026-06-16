#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "vertex.hpp"
#include "mesh_object.hpp"
#include "ogl_geometry_construction.hpp"
#include "ogl_geometry_factory.hpp"

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

	Particle(glm::vec3 pos, glm::vec3 vel, float life, float s, float particleStretch, float particleShape, glm::vec3 col, float particleRotation = 0.0f)
		: position(pos), velocity(vel), age(0.0f), maxAge(life), size(s), stretch(particleStretch), shape(particleShape), rotation(particleRotation), color(col)
	{}

	bool isAlive() const {
		return age < maxAge;
	}

	float getLifePercent() const {
		return glm::clamp(age / maxAge, 0.0f, 1.0f);
	}
};

struct ParticleInstanceData {
	glm::vec3 position;
	float age;			// x component
	glm::vec3 color;
	float maxAge;		// w component
	float size;
	float stretch;
	float shape;
	float rotation;
};

enum class ParticleEffect {
	Fire,
	Bubbles,
	Snow,
	Rain,
	Smoke,
	Mist,
	Sparks,
	Fountain,
	Explosion,
	Lightning,
	Magic,
};

class ParticleEmitter {
public:
	ParticleEmitter(glm::vec3 position, float emissionRate = 50.0f, float particleLife = 2.0f)
		: mPosition(position), mEmissionRate(emissionRate), mParticleLife(particleLife),
		  mEmissionCounter(0.0f), mEffect(ParticleEffect::Fire)
	{
		applyEffect(ParticleEffect::Fire);
	}

	void update(float deltaTime) {
		// Update existing particles
		for (auto &particle : mParticles) {
			if (particle.isAlive()) {
				particle.age += deltaTime;
				particle.velocity += mAcceleration * deltaTime;
				particle.velocity *= std::pow(mDrag, deltaTime);
				particle.velocity.x += std::sin((particle.age + particle.position.y) * mTurbulenceFrequency) * mTurbulence * deltaTime;
				particle.velocity.z += std::cos((particle.age + particle.position.x) * mTurbulenceFrequency) * mTurbulence * deltaTime;
				particle.position += particle.velocity * deltaTime;
			}
		}

		// Remove dead particles
		mParticles.erase(
			std::remove_if(mParticles.begin(), mParticles.end(),
				[](const Particle &p) { return !p.isAlive(); }),
			mParticles.end()
		);

		// Emit new particles
		mEmissionCounter += mEmissionRate * deltaTime;
		int particlesToEmit = static_cast<int>(mEmissionCounter);
		mEmissionCounter -= particlesToEmit;

		for (int i = 0; i < particlesToEmit; ++i) {
			emitParticle();
		}
	}

	void emitParticle() {
		if (mEffect == ParticleEffect::Fire) {
			emitFireParticle();
			return;
		}
		if (mEffect == ParticleEffect::Fountain) {
			emitFountainParticle();
			return;
		}
		if (mEffect == ParticleEffect::Explosion) {
			emitExplosionParticle();
			return;
		}
		if (mEffect == ParticleEffect::Lightning) {
			emitLightningParticle();
			return;
		}
		mParticles.emplace_back(spawnPosition(), spawnVelocity(), randomRange(mMinLife, mMaxLife), randomRange(mMinSize, mMaxSize), mStretch, mShape, spawnColor());
	}

	std::vector<Particle> &getParticles() {
		return mParticles;
	}

	const std::vector<Particle> &getParticles() const {
		return mParticles;
	}

	void setPosition(glm::vec3 pos) {
		mPosition = pos;
	}

	void setEffect(ParticleEffect effect) {
		applyEffect(effect);
		mParticles.clear();
		mEmissionCounter = 0.0f;
	}

	ParticleEffect getEffect() const {
		return mEffect;
	}

protected:
	float random01() const {
		return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	}

	float randomRange(float minValue, float maxValue) const {
		return minValue + (maxValue - minValue) * random01();
	}

	glm::vec3 randomDisk(float radius) const {
		const float angle = randomRange(0.0f, 6.2831853f);
		const float distance = std::sqrt(random01()) * radius;
		return glm::vec3(std::cos(angle) * distance, 0.0f, std::sin(angle) * distance);
	}

	glm::vec3 randomRing(float innerRadius, float outerRadius) const {
		const float angle = randomRange(0.0f, 6.2831853f);
		const float distance = randomRange(innerRadius, outerRadius);
		return glm::vec3(std::cos(angle) * distance, 0.0f, std::sin(angle) * distance);
	}

	glm::vec3 randomSphere(float radius) const {
		glm::vec3 direction(
			randomRange(-1.0f, 1.0f),
			randomRange(-0.65f, 1.0f),
			randomRange(-1.0f, 1.0f)
		);
		if (glm::length(direction) < 0.001f) {
			direction = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		return glm::normalize(direction) * randomRange(0.0f, radius);
	}

	void emitFireParticle() {
		const bool outerFlame = random01() < 0.25f;
		if (outerFlame) {
			glm::vec3 pos = mPosition + glm::vec3(0.0f, -0.78f, 0.12f) + randomRing(0.2f, 0.48f);
			glm::vec3 vel(randomRange(-0.035f, 0.035f), randomRange(0.9f, 1.55f), randomRange(-0.02f, 0.055f));
			glm::vec3 color(1.0f, randomRange(0.05f, 0.22f), 0.0f);
			mParticles.emplace_back(pos, vel, randomRange(0.9f, 1.45f), randomRange(0.05f, 0.13f), mStretch, mShape, color);
		} else {
			glm::vec3 pos = mPosition + glm::vec3(0.0f, -0.78f, -0.03f) + randomDisk(0.18f);
			glm::vec3 vel(randomRange(-0.025f, 0.025f), randomRange(0.72f, 1.22f), randomRange(-0.025f, 0.025f));
			glm::vec3 color(1.0f, randomRange(0.72f, 1.0f), randomRange(0.04f, 0.22f));
			mParticles.emplace_back(pos, vel, randomRange(0.72f, 1.12f), randomRange(0.04f, 0.105f), mStretch * 0.88f, mShape, color);
		}
	}

	void emitExplosionParticle() {
		glm::vec3 offset = randomSphere(0.09f);
		glm::vec3 direction = glm::normalize(offset + glm::vec3(randomRange(-0.35f, 0.35f), randomRange(-0.12f, 0.48f), randomRange(-0.35f, 0.35f)));
		glm::vec3 pos = mPosition + glm::vec3(0.0f, -0.25f, 0.0f) + offset;
		glm::vec3 vel = direction * randomRange(1.2f, 3.2f);
		const bool hot = random01() < 0.62f;
		glm::vec3 color = hot
			? glm::vec3(1.0f, randomRange(0.55f, 0.95f), randomRange(0.04f, 0.18f))
			: glm::vec3(1.0f, randomRange(0.14f, 0.32f), 0.0f);
		mParticles.emplace_back(pos, vel, randomRange(0.38f, 0.9f), randomRange(0.035f, 0.12f), mStretch, mShape, color);
	}

	void emitFountainParticle() {
		glm::vec3 pos = mPosition + glm::vec3(0.0f, -0.88f, 0.0f) + randomDisk(0.12f);
		glm::vec3 vel(randomRange(-0.72f, 0.72f), randomRange(2.65f, 3.35f), randomRange(-0.5f, 0.5f));
		glm::vec3 color(randomRange(0.38f, 0.62f), randomRange(0.68f, 0.88f), 1.0f);
		mParticles.emplace_back(pos, vel, randomRange(1.95f, 2.65f), randomRange(0.015f, 0.038f), randomRange(1.2f, 1.75f), mShape, color);
	}

	void emitLightningParticle() {
		const int boltCount = 1;
		for (int bolt = 0; bolt < boltCount; ++bolt) {
			const float sideDrift = randomRange(-0.45f, 0.45f);
			glm::vec3 direction = glm::normalize(glm::vec3(sideDrift, -1.0f, 0.0f));

			const glm::vec3 perpendicular = glm::normalize(glm::vec3(-direction.y, direction.x, 0.0f));
			const float length = randomRange(1.55f, 2.15f);
			const int segments = 9 + (std::rand() % 5);
			glm::vec3 start = mPosition + glm::vec3(randomRange(-0.85f, 0.85f), randomRange(0.75f, 1.35f), randomRange(-0.06f, 0.02f));

			glm::vec3 previousPoint = start;
			for (int i = 0; i < segments; ++i) {
				const float t1 = static_cast<float>(i + 1) / static_cast<float>(segments);
				const float jag = std::sin(t1 * 22.0f + randomRange(-0.35f, 0.35f)) * 0.04f + randomRange(-0.014f, 0.014f);
				glm::vec3 currentPoint = start + direction * (length * t1) + perpendicular * jag;
				glm::vec3 segment = currentPoint - previousPoint;
				float segmentLength = glm::length(segment);
				glm::vec3 segmentDirection = segmentLength > 0.001f ? segment / segmentLength : direction;
				float rotation = std::atan2(-segmentDirection.x, segmentDirection.y);
				glm::vec3 pos = (previousPoint + currentPoint) * 0.5f;
				glm::vec3 vel(randomRange(-0.05f, 0.05f), randomRange(-0.08f, 0.08f), randomRange(-0.02f, 0.02f));
				glm::vec3 color(randomRange(0.68f, 0.92f), randomRange(0.86f, 1.0f), 1.0f);
				const float particleSize = randomRange(0.018f, 0.026f);
				const float particleStretch = glm::clamp(segmentLength / particleSize * 1.75f, 10.0f, 28.0f);
				mParticles.emplace_back(pos, vel, randomRange(0.12f, 0.2f), particleSize, particleStretch, mShape, color, rotation);
				if (i > 1 && i < segments - 1 && random01() < 0.22f) {
					glm::vec3 branchDirection = glm::normalize(segmentDirection * randomRange(0.18f, 0.42f) + perpendicular * (random01() < 0.5f ? -1.0f : 1.0f));
					if (branchDirection.y > -0.04f) {
						branchDirection.y = -0.04f;
						branchDirection = glm::normalize(branchDirection);
					}
					const float branchLength = randomRange(0.18f, 0.34f);
					const float branchSize = randomRange(0.012f, 0.018f);
					const float branchStretch = glm::clamp(branchLength / branchSize * 1.55f, 8.0f, 18.0f);
					const float branchRotation = std::atan2(-branchDirection.x, branchDirection.y);
					const glm::vec3 branchPos = currentPoint + branchDirection * (branchLength * 0.5f);
					mParticles.emplace_back(branchPos, vel, randomRange(0.08f, 0.15f), branchSize, branchStretch, mShape, color, branchRotation);
				}
				previousPoint = currentPoint;
			}
		}
	}

	glm::vec3 spawnPosition() const {
		switch (mEffect) {
		case ParticleEffect::Fire:
			return mPosition + glm::vec3(0.0f, -0.78f, 0.0f) + randomDisk(0.34f);
		case ParticleEffect::Bubbles:
			return mPosition + randomDisk(0.55f) + glm::vec3(0.0f, -1.2f, 0.0f);
		case ParticleEffect::Snow:
			return mPosition + glm::vec3(randomRange(-3.0f, 3.0f), 2.0f, randomRange(-1.0f, 1.0f));
		case ParticleEffect::Rain:
			return mPosition + glm::vec3(randomRange(-3.2f, 3.2f), 2.2f, randomRange(-1.0f, 1.0f));
		case ParticleEffect::Smoke:
			return mPosition + randomDisk(0.35f);
		case ParticleEffect::Mist:
			return mPosition + glm::vec3(randomRange(-2.6f, 2.6f), randomRange(-0.85f, -0.45f), randomRange(-0.85f, 0.85f));
		case ParticleEffect::Sparks:
			return mPosition + glm::vec3(0.0f, -0.68f, 0.0f) + randomDisk(0.16f);
		case ParticleEffect::Fountain:
			return mPosition + glm::vec3(0.0f, -0.88f, 0.0f) + randomDisk(0.12f);
		case ParticleEffect::Explosion:
			return mPosition + randomSphere(0.08f);
		case ParticleEffect::Lightning:
			return mPosition + glm::vec3(randomRange(-0.16f, 0.16f), randomRange(-0.65f, 0.65f), randomRange(-0.04f, 0.04f));
		case ParticleEffect::Magic:
			return mPosition + randomRing(0.05f, 0.58f) + glm::vec3(0.0f, randomRange(-0.38f, 0.18f), 0.0f);
		}
		return mPosition;
	}

	glm::vec3 spawnVelocity() const {
		switch (mEffect) {
		case ParticleEffect::Fire:
			return glm::vec3(randomRange(-0.045f, 0.045f), randomRange(0.52f, 1.05f), randomRange(-0.045f, 0.045f));
		case ParticleEffect::Bubbles:
			return glm::vec3(randomRange(-0.08f, 0.08f), randomRange(0.32f, 0.68f), randomRange(-0.08f, 0.08f));
		case ParticleEffect::Snow:
			return glm::vec3(randomRange(-0.18f, 0.18f), randomRange(-0.35f, -0.12f), randomRange(-0.08f, 0.08f));
		case ParticleEffect::Rain:
			return glm::vec3(randomRange(-0.08f, 0.08f), randomRange(-5.3f, -4.1f), randomRange(-0.04f, 0.04f));
		case ParticleEffect::Smoke:
			return glm::vec3(randomRange(-0.18f, 0.18f), randomRange(0.22f, 0.54f), randomRange(-0.18f, 0.18f));
		case ParticleEffect::Mist:
			return glm::vec3(randomRange(-0.08f, 0.08f), randomRange(0.13f, 0.26f), randomRange(-0.04f, 0.04f));
		case ParticleEffect::Sparks:
			return glm::vec3(randomRange(-0.5f, 0.5f), randomRange(1.25f, 2.25f), randomRange(-0.42f, 0.42f));
		case ParticleEffect::Fountain:
			return glm::vec3(randomRange(-0.72f, 0.72f), randomRange(2.65f, 3.35f), randomRange(-0.5f, 0.5f));
		case ParticleEffect::Explosion:
			return glm::normalize(randomSphere(1.0f) + glm::vec3(0.0f, 0.25f, 0.0f)) * randomRange(1.2f, 3.2f);
		case ParticleEffect::Lightning:
			return glm::vec3(randomRange(-0.5f, 0.5f), randomRange(-0.15f, 0.15f), randomRange(-0.05f, 0.05f));
		case ParticleEffect::Magic:
			return glm::vec3(randomRange(-0.42f, 0.42f), randomRange(0.25f, 0.85f), randomRange(-0.42f, 0.42f));
		}
		return glm::vec3(0.0f);
	}

	glm::vec3 spawnColor() const {
		switch (mEffect) {
		case ParticleEffect::Fire: {
			const int colorType = std::rand() % 4;
			if (colorType == 0) return glm::vec3(1.0f, randomRange(0.08f, 0.18f), 0.0f);
			if (colorType == 1) return glm::vec3(1.0f, randomRange(0.28f, 0.48f), 0.0f);
			if (colorType == 2) return glm::vec3(1.0f, randomRange(0.58f, 0.78f), randomRange(0.02f, 0.08f));
			return glm::vec3(1.0f, randomRange(0.86f, 1.0f), randomRange(0.14f, 0.25f));
		}
		case ParticleEffect::Bubbles: {
			const int colorType = std::rand() % 3;
			if (colorType == 0) return glm::vec3(randomRange(0.86f, 1.0f), randomRange(0.92f, 1.0f), 1.0f);
			if (colorType == 1) return glm::vec3(randomRange(0.72f, 0.9f), randomRange(0.86f, 1.0f), 1.0f);
			return glm::vec3(randomRange(0.88f, 1.0f), randomRange(0.88f, 1.0f), randomRange(0.88f, 1.0f));
		}
		case ParticleEffect::Snow:
			return glm::vec3(randomRange(0.82f, 1.0f), randomRange(0.88f, 1.0f), 1.0f);
		case ParticleEffect::Rain:
			return glm::vec3(randomRange(0.18f, 0.34f), randomRange(0.42f, 0.68f), 1.0f);
		case ParticleEffect::Smoke: {
			const float grey = randomRange(0.34f, 0.74f);
			return glm::vec3(grey);
		}
		case ParticleEffect::Mist: {
			const float grey = randomRange(0.78f, 1.0f);
			return glm::vec3(grey, grey, randomRange(grey, 1.0f));
		}
		case ParticleEffect::Sparks: {
			const int colorType = std::rand() % 3;
			if (colorType == 0) return glm::vec3(1.0f, randomRange(0.74f, 1.0f), randomRange(0.08f, 0.2f));
			if (colorType == 1) return glm::vec3(1.0f, randomRange(0.34f, 0.55f), 0.0f);
			return glm::vec3(1.0f, 1.0f, randomRange(0.48f, 0.74f));
		}
		case ParticleEffect::Fountain:
			return glm::vec3(randomRange(0.38f, 0.62f), randomRange(0.68f, 0.88f), 1.0f);
		case ParticleEffect::Explosion: {
			const int colorType = std::rand() % 4;
			if (colorType == 0) return glm::vec3(1.0f, 0.95f, randomRange(0.36f, 0.58f));
			if (colorType == 1) return glm::vec3(1.0f, randomRange(0.42f, 0.68f), 0.02f);
			if (colorType == 2) return glm::vec3(1.0f, randomRange(0.12f, 0.28f), 0.0f);
			return glm::vec3(randomRange(0.42f, 0.65f));
		}
		case ParticleEffect::Lightning:
			return glm::vec3(randomRange(0.62f, 0.9f), randomRange(0.82f, 1.0f), 1.0f);
		case ParticleEffect::Magic: {
			const int colorType = std::rand() % 4;
			if (colorType == 0) return glm::vec3(0.9f, 0.34f, 1.0f);
			if (colorType == 1) return glm::vec3(0.35f, 0.95f, 1.0f);
			if (colorType == 2) return glm::vec3(1.0f, 0.86f, 0.28f);
			return glm::vec3(0.55f, 0.42f, 1.0f);
		}
		}
		return glm::vec3(1.0f);
	}

	void applyEffect(ParticleEffect effect) {
		mEffect = effect;
		switch (mEffect) {
		case ParticleEffect::Fire:
			mEmissionRate = 220.0f;
			mMinLife = 0.65f;
			mMaxLife = 1.15f;
			mMinSize = 0.04f;
			mMaxSize = 0.11f;
			mStretch = 9.5f;
			mShape = 1.0f;
			mAcceleration = glm::vec3(0.0f, 0.32f, 0.0f);
			mDrag = 0.82f;
			mTurbulence = 0.08f;
			mTurbulenceFrequency = 8.5f;
			break;
		case ParticleEffect::Bubbles:
			mEmissionRate = 80.0f;
			mMinLife = 3.0f;
			mMaxLife = 5.0f;
			mMinSize = 0.075f;
			mMaxSize = 0.2f;
			mStretch = 1.0f;
			mShape = 0.0f;
			mAcceleration = glm::vec3(0.0f, 0.055f, 0.0f);
			mDrag = 0.82f;
			mTurbulence = 0.14f;
			mTurbulenceFrequency = 3.2f;
			break;
		case ParticleEffect::Snow:
			mEmissionRate = 180.0f;
			mMinLife = 5.5f;
			mMaxLife = 7.5f;
			mMinSize = 0.012f;
			mMaxSize = 0.035f;
			mStretch = 1.0f;
			mShape = 0.0f;
			mAcceleration = glm::vec3(0.0f, -0.08f, 0.0f);
			mDrag = 0.96f;
			mTurbulence = 0.42f;
			mTurbulenceFrequency = 2.6f;
			break;
		case ParticleEffect::Rain:
			mEmissionRate = 420.0f;
			mMinLife = 0.75f;
			mMaxLife = 1.15f;
			mMinSize = 0.025f;
			mMaxSize = 0.045f;
			mStretch = 7.0f;
			mShape = 0.0f;
			mAcceleration = glm::vec3(0.0f, -2.2f, 0.0f);
			mDrag = 0.99f;
			mTurbulence = 0.08f;
			mTurbulenceFrequency = 5.0f;
			break;
		case ParticleEffect::Smoke:
			mEmissionRate = 95.0f;
			mMinLife = 3.6f;
			mMaxLife = 6.2f;
			mMinSize = 0.28f;
			mMaxSize = 0.7f;
			mStretch = 1.8f;
			mShape = 2.0f;
			mAcceleration = glm::vec3(0.0f, 0.1f, 0.0f);
			mDrag = 0.86f;
			mTurbulence = 0.62f;
			mTurbulenceFrequency = 2.6f;
			break;
		case ParticleEffect::Mist:
			mEmissionRate = 260.0f;
			mMinLife = 7.0f;
			mMaxLife = 11.0f;
			mMinSize = 0.48f;
			mMaxSize = 1.05f;
			mStretch = 0.62f;
			mShape = 3.0f;
			mAcceleration = glm::vec3(0.0f, 0.035f, 0.0f);
			mDrag = 0.96f;
			mTurbulence = 0.14f;
			mTurbulenceFrequency = 1.2f;
			break;
		case ParticleEffect::Sparks:
			mEmissionRate = 180.0f;
			mMinLife = 0.45f;
			mMaxLife = 1.05f;
			mMinSize = 0.012f;
			mMaxSize = 0.035f;
			mStretch = 3.6f;
			mShape = 4.0f;
			mAcceleration = glm::vec3(0.0f, -1.85f, 0.0f);
			mDrag = 0.9f;
			mTurbulence = 0.16f;
			mTurbulenceFrequency = 10.0f;
			break;
		case ParticleEffect::Fountain:
			mEmissionRate = 210.0f;
			mMinLife = 1.95f;
			mMaxLife = 2.65f;
			mMinSize = 0.015f;
			mMaxSize = 0.038f;
			mStretch = 1.45f;
			mShape = 0.0f;
			mAcceleration = glm::vec3(0.0f, -2.85f, 0.0f);
			mDrag = 0.95f;
			mTurbulence = 0.1f;
			mTurbulenceFrequency = 5.2f;
			break;
		case ParticleEffect::Explosion:
			mEmissionRate = 560.0f;
			mMinLife = 0.38f;
			mMaxLife = 0.9f;
			mMinSize = 0.035f;
			mMaxSize = 0.12f;
			mStretch = 1.4f;
			mShape = 4.0f;
			mAcceleration = glm::vec3(0.0f, -0.45f, 0.0f);
			mDrag = 0.78f;
			mTurbulence = 0.18f;
			mTurbulenceFrequency = 8.0f;
			break;
		case ParticleEffect::Lightning:
			mEmissionRate = 1.35f;
			mMinLife = 0.11f;
			mMaxLife = 0.22f;
			mMinSize = 0.012f;
			mMaxSize = 0.03f;
			mStretch = 8.0f;
			mShape = 5.0f;
			mAcceleration = glm::vec3(0.0f);
			mDrag = 0.82f;
			mTurbulence = 0.18f;
			mTurbulenceFrequency = 18.0f;
			break;
		case ParticleEffect::Magic:
			mEmissionRate = 150.0f;
			mMinLife = 2.0f;
			mMaxLife = 3.8f;
			mMinSize = 0.035f;
			mMaxSize = 0.095f;
			mStretch = 1.0f;
			mShape = 6.0f;
			mAcceleration = glm::vec3(0.0f, 0.04f, 0.0f);
			mDrag = 0.92f;
			mTurbulence = 0.75f;
			mTurbulenceFrequency = 4.8f;
			break;
		}
	}

	std::vector<Particle> mParticles;
	glm::vec3 mPosition;
	float mEmissionRate;
	float mParticleLife;
	float mEmissionCounter;
	ParticleEffect mEffect;
	glm::vec3 mAcceleration;
	float mMinLife;
	float mMaxLife;
	float mMinSize;
	float mMaxSize;
	float mStretch;
	float mShape;
	float mDrag;
	float mTurbulence;
	float mTurbulenceFrequency;
};

inline IndexedBuffer
generateParticleBuffers(const std::vector<Particle> &aParticles) {
	IndexedBuffer buffers{
		createVertexArray(),
	};
	buffers.vbos.push_back(createBuffer());
	buffers.vbos.push_back(createBuffer());
	buffers.vbos.push_back(createBuffer());

	// Create a simple quad geometry for billboards
	std::vector<VertexNormTex> vertices;
	std::vector<unsigned int> indices;

	// Single quad that will be instanced for each particle
	vertices.push_back(VertexNormTex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)));
	vertices.push_back(VertexNormTex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)));
	vertices.push_back(VertexNormTex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)));
	vertices.push_back(VertexNormTex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)));

	indices = { 0, 1, 2, 0, 2, 3 };

	GL_CHECK(glBindVertexArray(buffers.vao.get()));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers.vbos[0].get()));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(VertexNormTex) * vertices.size(), vertices.data(), GL_STATIC_DRAW));

	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.vbos[1].get()));
	GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW));

	// Position attribute
	GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)0));
	GL_CHECK(glEnableVertexAttribArray(0));

	// Normal attribute
	GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)(sizeof(glm::vec3))));
	GL_CHECK(glEnableVertexAttribArray(1));

	// Texture coordinate attribute
	GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexNormTex), (void*)(2 * sizeof(glm::vec3))));
	GL_CHECK(glEnableVertexAttribArray(2));

	// INSTANCE ATTRIBUTES
	std::vector<ParticleInstanceData> instanceData;
	for (const auto &particle : aParticles) {
		instanceData.push_back({
			particle.position,
			particle.age,
			particle.color,
			particle.maxAge,
			particle.size,
			particle.stretch,
			particle.shape,
			particle.rotation
		});
	}

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers.vbos[2].get()));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleInstanceData) * instanceData.size(), instanceData.data(), GL_DYNAMIC_DRAW));

	// Position attribute
	GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, position)));
	GL_CHECK(glEnableVertexAttribArray(3));
	GL_CHECK(glVertexAttribDivisor(3, 1));

	// Age attribute
	GL_CHECK(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, age)));
	GL_CHECK(glEnableVertexAttribArray(4));
	GL_CHECK(glVertexAttribDivisor(4, 1));

	// Color attribute
	GL_CHECK(glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, color)));
	GL_CHECK(glEnableVertexAttribArray(5));
	GL_CHECK(glVertexAttribDivisor(5, 1));

	// MaxAge attribute
	GL_CHECK(glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, maxAge)));
	GL_CHECK(glEnableVertexAttribArray(6));
	GL_CHECK(glVertexAttribDivisor(6, 1));

	// Size attribute
	GL_CHECK(glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, size)));
	GL_CHECK(glEnableVertexAttribArray(7));
	GL_CHECK(glVertexAttribDivisor(7, 1));

	// Vertical stretch attribute
	GL_CHECK(glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, stretch)));
	GL_CHECK(glEnableVertexAttribArray(8));
	GL_CHECK(glVertexAttribDivisor(8, 1));

	// Shape attribute
	GL_CHECK(glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, shape)));
	GL_CHECK(glEnableVertexAttribArray(9));
	GL_CHECK(glVertexAttribDivisor(9, 1));

	// Rotation attribute
	GL_CHECK(glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)offsetof(ParticleInstanceData, rotation)));
	GL_CHECK(glEnableVertexAttribArray(10));
	GL_CHECK(glVertexAttribDivisor(10, 1));

	// Unbind VAO
	GL_CHECK(glBindVertexArray(0));

	buffers.indexCount = static_cast<unsigned>(indices.size());
	buffers.instanceCount = static_cast<unsigned>(aParticles.size());
	buffers.mode = GL_TRIANGLES;
	return buffers;
}

class ParticleSystem : public MeshObject {
public:
	ParticleSystem(std::shared_ptr<ParticleEmitter> aEmitter)
		: mEmitter(aEmitter), mGeometry(nullptr)
	{}

	virtual std::shared_ptr<AGeometry> getGeometry(GeometryFactory &aGeometryFactory, RenderStyle aRenderStyle) {
		// Create new geometry each time (particles change every frame)
		mGeometry = std::make_shared<OGLGeometry>(generateParticleBuffers(mEmitter->getParticles()));
		return mGeometry;
	}

	void prepareRenderData(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) override {
		for (auto &mode : mRenderInfos) {
			mode.second.shaderProgram = aMaterialFactory.getShaderProgram(mode.second.materialParams.mMaterialName);
			getTextures(mode.second.materialParams.mParameterValues, aMaterialFactory);
			mode.second.geometry = getGeometry(aGeometryFactory, mode.second.materialParams.mRenderStyle);
		}
	}

	void update(float deltaTime) {
		mEmitter->update(deltaTime);
		// Recreate geometry for next render
		if (mGeometry) {
			for (auto &mode : mRenderInfos) {
				mode.second.geometry = getGeometry(*(GeometryFactory*)nullptr, mode.second.materialParams.mRenderStyle);
			}
		}
	}

	std::shared_ptr<ParticleEmitter> &getEmitter() {
		return mEmitter;
	}

protected:
	std::shared_ptr<ParticleEmitter> mEmitter;
	std::shared_ptr<OGLGeometry> mGeometry;
};

