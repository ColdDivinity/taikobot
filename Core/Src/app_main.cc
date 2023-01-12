#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

#include "main.h"

extern "C" {

extern TIM_HandleTypeDef htim14;

void AppMain();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* tim);
}

namespace {

#define TAIKO_LEFT_DON 1
#define TAIKO_RIGHT_DON 2
#define TAIKO_LEFT_KATSU 3
#define TAIKO_RIGHT_KATSU 4

struct TaikoHit {
  int time_ms;
  int key;
};

constexpr auto kYuugen = std::to_array<TaikoHit>({
#include "yuugen.inc"
});

constexpr int64_t kTimerPeriodMs = 1;

void Panic() {
  while (true)
    ;
}

}  // namespace

void AppMain() {
  if (HAL_TIM_Base_Start_IT(&htim14) != HAL_OK) {
    Panic();
  }
  while (true)
    ;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* tim) {
  if (tim->Instance == TIM14) {
    static int64_t time_ms = 0;
    static bool started = false;

    if (!started && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
      started = true;
    }

    if (!started) {
      return;
    }

    time_ms += kTimerPeriodMs;

    auto iter = std::lower_bound(kYuugen.rbegin(), kYuugen.rend(), time_ms,
                                 [](const TaikoHit& hit, int64_t time_ms) {
                                   return hit.time_ms > time_ms;
                                 });
    if (iter != kYuugen.rend()) {
      uint16_t gpio_pin;
      switch (iter->key) {
        case TAIKO_LEFT_DON:
        case TAIKO_RIGHT_DON:
          gpio_pin = GPIO_PIN_0;
          break;
        case TAIKO_LEFT_KATSU:
        case TAIKO_RIGHT_KATSU:
          gpio_pin = GPIO_PIN_1;
          break;
        default:
          Panic();
      }
      if (time_ms - iter->time_ms <= 25) {
        HAL_GPIO_WritePin(GPIOA, gpio_pin, GPIO_PIN_SET);
      } else {
        HAL_GPIO_WritePin(GPIOA, gpio_pin, GPIO_PIN_RESET);
      }
    } else {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    }
  }
}
