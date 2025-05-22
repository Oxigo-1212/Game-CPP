#pragma once

namespace WeaponConfig {
    // Pistol configuration
    namespace Pistol {
        static constexpr int DAMAGE = 3;
        static constexpr float FIRE_RATE = 0.25f;     // Time between shots in seconds
        static constexpr float RELOAD_TIME = 1.0f;
        static constexpr int MAX_AMMO = 12;
        static constexpr float BULLET_SPEED = 800.0f;
    }

    // Rifle configuration
    namespace Rifle {
        static constexpr int DAMAGE = 2;
        static constexpr float FIRE_RATE = 0.1f;      // Faster fire rate for auto
        static constexpr float RELOAD_TIME = 1.0f;
        static constexpr int MAX_AMMO = 50;
        static constexpr float BULLET_SPEED = 1000.0f;
    }

    // Shotgun configuration
    namespace Shotgun {
        static constexpr int PELLET_DAMAGE = 1;
        static constexpr int PELLET_COUNT = 8;
        static constexpr float SPREAD_ANGLE = 45.0f;
        static constexpr float FIRE_RATE = 0.8f;      // Slower fire rate
        static constexpr float RELOAD_TIME = 1.5f;
        static constexpr int MAX_AMMO = 8;
        static constexpr float BULLET_SPEED = 600.0f;
        
        // Knockback configuration
        static constexpr float KNOCKBACK_FORCE = 400.0f;
        static constexpr float KNOCKBACK_MULTIPLIER = 0.5f;
        static constexpr float KNOCKBACK_DURATION = 0.2f;
    }
}
