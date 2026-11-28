#pragma once

#include <algorithm>

namespace team25 {

class BatteryStorage {
public:
    void update(float seconds, float solarOutput) {
        const float rate = usingBattery_ ? -1.0f / kDischargeSeconds
                                         : solarOutput / kChargeSeconds;
        charge_ = std::clamp(charge_ + seconds * rate, 0.0f, 1.0f);
    }

    void toggleSource() { usingBattery_ = !usingBattery_; }
    bool usingBattery() const { return usingBattery_; }
    float charge() const { return charge_; }
    bool lightsPowered(float solarOutput) const {
        return usingBattery_ ? charge_ > 0.0f : solarOutput > 0.0f;
    }

private:
    static constexpr float kChargeSeconds = 120.0f;
    static constexpr float kDischargeSeconds = 240.0f;
    float charge_ = 0.5f;
    bool usingBattery_ = false;
};

}  // namespace team25
