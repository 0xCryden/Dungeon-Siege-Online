#pragma once
// ---------------------------------------------------------
// Configurable server tick rate
// ---------------------------------------------------------
constexpr int TICKS_PER_SECOND = 128;
constexpr int TICK_MICROSECONDS = 1'000'000 / TICKS_PER_SECOND;
constexpr int TICK_MILLISECONDS = 1'000 / TICKS_PER_SECOND;
constexpr int CALC_FRUSTUM_DELAY = TICK_MILLISECONDS * 128; // Calculate frustums once per second


constexpr int SECOND = 1000;
constexpr int MINUTE = SECOND * 60;
constexpr int HOUR = MINUTE * 60;
constexpr int DAY = HOUR * 24;
constexpr int WEEK = DAY * 7;