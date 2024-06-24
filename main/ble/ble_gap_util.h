#pragma once

#include <array>

template <typename... Arrays>
constexpr std::size_t total_size()
{
    return (std::tuple_size_v<std::remove_reference_t<Arrays>> + ...);
}

template <std::size_t... Is, typename... Arrays>
constexpr auto concatenate_impl(std::index_sequence<Is...>, Arrays &&...arrays)
{
    constexpr std::size_t totalSize = total_size<Arrays...>();

    std::array<uint8_t, totalSize> result{};
    std::size_t offset = 0;

    auto copy_array = [&result, &offset](auto &&array)
    {
        for (auto &elem : array)
        {
            result[offset++] = elem;
        }
    };

    (copy_array(arrays), ...);

    return result;
}

template <typename... Arrays>
consteval auto concatenate(Arrays &&...arrays)
{
    return concatenate_impl(std::make_index_sequence<total_size<Arrays...>()>{}, std::forward<Arrays>(arrays)...);
}

template <std::size_t N>
consteval auto tag_data(int tag, std::array<int, N> list)
{
    std::array<uint8_t, N + 2> out{};
    out[0] = N + 1;
    out[1] = static_cast<uint8_t>(tag);
    for (int i = 2; i < N + 2; i++)
    {
        out[i] = list[i - 2];
    }

    return out;
}

template <std::size_t N>
consteval std::array<uint8_t, N + 1> device_name(const char (&name)[N])
{
    constexpr uint8_t DEVICE_NAME_TAG = 0x09;

    // array size - 1 null term + 1 size + 1 tag
    std::array<uint8_t, N + 1> out{};
    out[0] = N;
    out[1] = DEVICE_NAME_TAG;

    for (int i = 2; i < out.size(); i++)
    {
        out[i] = static_cast<uint8_t>(name[i - 2]);
    }

    return out;
}