#pragma once

#include "esphome/components/switch/switch.h"
#include "as7331.h"

namespace esphome {
namespace as7331 {

class AS7331Switch : public switch_::Switch {
 public:
  void set_parent(AS7331Component *parent) {
    parent_ = parent;
    publish_state(true);  // default ON
  }

 protected:
  void write_state(bool state) override {
    if (parent_ != nullptr) {
      parent_->set_measurement_enabled(state);
    }
    publish_state(state);
  }

  AS7331Component *parent_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
