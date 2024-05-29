#pragma once

#include <stdint.h>
#include <array>

#include "color_util.h"

namespace led
{
    struct Pixel final
    {
        static const size_t size = sizeof(uint8_t) * 3;

        const uint8_t r;
        const uint8_t g;
        const uint8_t b;

        constexpr Pixel(uint8_t r, uint8_t g, uint8_t b) : r{r}, g{g}, b{b} {}

        constexpr std::array<uint8_t, 3> asGRB8Bit() const
        {
            std::array<uint8_t, 3> grb{this->g, this->r, this->b};
            return grb;
        }

        static constexpr Pixel pixelFromHsv(const uint32_t h, const uint32_t s, const uint32_t v)
        {
            std::array<uint8_t, 3> colors{};
            led_strip_hsv2rgb(h, s, v, &colors[0], &colors[1], &colors[2]);
            return Pixel{colors[0], colors[1], colors[2]};
        }
    };
}