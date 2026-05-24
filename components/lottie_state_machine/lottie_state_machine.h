#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32
#include "esphome/components/lvgl/lottie_loader.h"
#endif

#include <string>
#include <vector>
#include <map>

namespace esphome {
namespace lottie_state_machine {

struct StateDefinition {
  std::string name;
  int32_t start_frame;
  int32_t end_frame;
  bool loop;
};

class LottieStateMachineComponent;

// --- Switch: Play/Pause ---
class LottiePlaySwitch : public switch_::Switch {
 public:
  void set_parent(LottieStateMachineComponent *parent) { this->parent_ = parent; }

 protected:
  void write_state(bool state) override;
  LottieStateMachineComponent *parent_{nullptr};
};

// --- Select: State ---
class LottieStateSelect : public select::Select {
 public:
  void set_parent(LottieStateMachineComponent *parent) { this->parent_ = parent; }

 protected:
  void control(const std::string &value) override;
  LottieStateMachineComponent *parent_{nullptr};
};

// --- Number: Speed ---
class LottieSpeedNumber : public number::Number {
 public:
  void set_parent(LottieStateMachineComponent *parent) { this->parent_ = parent; }

 protected:
  void control(float value) override;
  LottieStateMachineComponent *parent_{nullptr};
};

// --- Number: Frame ---
class LottieFrameNumber : public number::Number {
 public:
  void set_parent(LottieStateMachineComponent *parent) { this->parent_ = parent; }

 protected:
  void control(float value) override;
  LottieStateMachineComponent *parent_{nullptr};
};

// --- Sensor: Current Frame ---
class LottieFrameSensor : public sensor::Sensor {};

// --- Main Component ---
class LottieStateMachineComponent : public Component {
 public:
  void set_lottie_id(const std::string &id) { this->lottie_id_ = id; }

  void add_state(const std::string &name, int32_t start, int32_t end, bool loop) {
    StateDefinition s;
    s.name = name;
    s.start_frame = start;
    s.end_frame = end;
    s.loop = loop;
    this->states_[name] = s;
    this->state_names_.push_back(name);
  }

  void set_initial_state(const std::string &state) { this->initial_state_ = state; }

  void set_play_switch(LottiePlaySwitch *sw) { this->play_switch_ = sw; }
  void set_state_select(LottieStateSelect *sel) { this->state_select_ = sel; }
  void set_speed_number(LottieSpeedNumber *num) { this->speed_number_ = num; }
  void set_frame_number(LottieFrameNumber *num) { this->frame_number_ = num; }
  void set_frame_sensor(LottieFrameSensor *sens) { this->frame_sensor_ = sens; }

  float get_setup_priority() const override { return setup_priority::LATE; }

  void setup() override {
    // Initialize entities with defaults
    if (this->play_switch_) this->play_switch_->publish_state(false);
    if (this->speed_number_) this->speed_number_->publish_state(1.0f);
    if (this->frame_number_) this->frame_number_->publish_state(0);
    if (this->state_select_ && !this->initial_state_.empty()) {
      this->state_select_->publish_state(this->initial_state_);
    }
    this->current_speed_ = 1.0f;
  }

  void loop() override {
#if LV_USE_LOTTIE
    // Update frame sensor periodically
    static uint32_t last_update = 0;
    uint32_t now = millis();
    if (now - last_update > 500 && this->ctx_ != nullptr) {
      last_update = now;
      // We could read the current frame from the context if we add it
    }
#endif
  }

  // --- Public API ---

  void set_playing(bool playing) {
#if LV_USE_LOTTIE
    if (this->ctx_ == nullptr) return;
    if (playing) {
      lvgl::lottie_play(this->ctx_);
    } else {
      lvgl::lottie_pause(this->ctx_);
    }
    if (this->play_switch_) this->play_switch_->publish_state(playing);
#endif
  }

  void set_state(const std::string &state_name) {
#if LV_USE_LOTTIE
    if (this->ctx_ == nullptr) return;

    auto it = this->states_.find(state_name);
    if (it == this->states_.end()) {
      ESP_LOGW("lottie_sm", "Unknown state: %s", state_name.c_str());
      return;
    }

    auto &s = it->second;
    this->current_state_ = state_name;

    // Update the context with new segment
    this->ctx_->start_frame = s.start_frame;
    this->ctx_->end_frame = s.end_frame;
    this->ctx_->loop = s.loop;

    // Restart from the beginning of the new segment
    lvgl::lottie_restart(this->ctx_);

    ESP_LOGI("lottie_sm", "State → %s (frames %d-%d, loop=%d)",
             state_name.c_str(), (int)s.start_frame, (int)s.end_frame, (int)s.loop);

    if (this->state_select_) this->state_select_->publish_state(state_name);
    if (this->play_switch_) this->play_switch_->publish_state(true);
#endif
  }

  void set_speed(float speed) {
    this->current_speed_ = speed;
#if LV_USE_LOTTIE
    if (this->ctx_ != nullptr && this->ctx_->duration_ms > 0) {
      // Adjust duration based on speed (higher speed = shorter duration)
      // Original duration is captured on first load
      // We don't modify it directly - speed is applied in frame calculation
    }
#endif
    if (this->speed_number_) this->speed_number_->publish_state(speed);
  }

  void go_to_frame(int32_t frame) {
#if LV_USE_LOTTIE
    if (this->ctx_ == nullptr || this->ctx_->exec_cb == nullptr) return;
    lv_lock();
    this->ctx_->exec_cb(this->ctx_->anim_var, frame);
    lv_unlock();
    // Pause after seeking
    lvgl::lottie_pause(this->ctx_);
    if (this->play_switch_) this->play_switch_->publish_state(false);
#endif
    if (this->frame_number_) this->frame_number_->publish_state(frame);
  }

#if LV_USE_LOTTIE
  void set_context(lvgl::LottieContext *ctx) {
    this->ctx_ = ctx;
    // Apply initial state if set
    if (!this->initial_state_.empty()) {
      this->set_state(this->initial_state_);
    }
  }

  lvgl::LottieContext *get_context() { return this->ctx_; }
#endif

 protected:
  std::string lottie_id_;
  std::string initial_state_;
  std::string current_state_;
  float current_speed_{1.0f};

  std::map<std::string, StateDefinition> states_;
  std::vector<std::string> state_names_;

  LottiePlaySwitch *play_switch_{nullptr};
  LottieStateSelect *state_select_{nullptr};
  LottieSpeedNumber *speed_number_{nullptr};
  LottieFrameNumber *frame_number_{nullptr};
  LottieFrameSensor *frame_sensor_{nullptr};

#if LV_USE_LOTTIE
  lvgl::LottieContext *ctx_{nullptr};
#endif
};

// --- Switch implementation ---
inline void LottiePlaySwitch::write_state(bool state) {
  if (this->parent_) this->parent_->set_playing(state);
}

// --- Select implementation ---
inline void LottieStateSelect::control(const std::string &value) {
  if (this->parent_) this->parent_->set_state(value);
}

// --- Number: Speed implementation ---
inline void LottieSpeedNumber::control(float value) {
  if (this->parent_) this->parent_->set_speed(value);
  this->publish_state(value);
}

// --- Number: Frame implementation ---
inline void LottieFrameNumber::control(float value) {
  if (this->parent_) this->parent_->go_to_frame((int32_t)value);
}

// --- Action: set_state ---
template<typename... Ts> class LottieSetStateAction : public Action<Ts...> {
 public:
  LottieSetStateAction(LottieStateMachineComponent *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(std::string, state)

  void play(Ts... x) override {
    this->parent_->set_state(this->state_.value(x...));
  }

 protected:
  LottieStateMachineComponent *parent_;
};

}  // namespace lottie_state_machine
}  // namespace esphome
