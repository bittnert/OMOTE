// OMOTE Hardware Abstraction
// 2023 Matthew Colvin

#pragma once
#include <functional>
#include <lvgl.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "BatteryInterface.h"

class HardwareAbstract {
public:

  struct batteryStatus {
      /// @brief Percent of battery remaining (0-100]
      int percentage;
      /// @brief True - Battery is Charging
      ///        False - Battery discharging
      bool isCharging;
  };
  virtual std::optional<batteryStatus> getBatteryStatus();

  HardwareAbstract(std::shared_ptr<BatteryInterface> aBattery = nullptr);

  virtual void init() = 0;
  virtual void sendIR() = 0;
  virtual void MQTTPublish(const char *topic, const char *payload) = 0;
  virtual void debugPrint(std::string message) = 0;

  // Didn't actually implement this but would need to set up something to intermittently notify of batteryChange.
  void notifyBatteryChange(batteryStatus aStatus);
  void onBatteryChange(std::function<void(batteryStatus)> onBatteryStatusChangeHandler);

  private:
    std::shared_ptr<BatteryInterface> mBattery;
    std::vector<std::function<void(batteryStatus)>> mBatteryEventHandlers;

};
