#pragma once

#include <cstdint>
#include <array>

#include "color_util.h"

namespace ws2812
{
    struct Pixel final
    {
        const uint8_t g;
        const uint8_t r;
        const uint8_t b;

        constexpr Pixel() : g{0}, r{0}, b{0} {}
        constexpr Pixel(uint8_t r, uint8_t g, uint8_t b) : g{g}, r{r}, b{b} {}

        static constexpr Pixel pixelFromHsv(const uint32_t h, const uint32_t s, const uint32_t v)
        {
            std::array<uint8_t, 3> colors{};
            util::led_strip_hsv2rgb(h, s, v, &colors[0], &colors[1], &colors[2]);
            return Pixel{colors[0], colors[1], colors[2]};
        }
    };
}