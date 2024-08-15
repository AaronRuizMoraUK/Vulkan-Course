#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <optional>

namespace DX
{
    // Reads the content of a text file.
    // The filename is relative to the assets folder.
    std::optional<std::string> ReadAssetTextFile(const std::string& fileName);

    // Reads the content of a binary file.
    // The filename is relative to the assets folder.
    std::optional<std::vector<uint8_t>> ReadAssetBinaryFile(const std::string& fileName);

    // Returns the path to the assets folder.
    std::filesystem::path GetAssetPath();

    // Returns the path to the executable folder.
    std::filesystem::path GetExecutablePath();
} // namespace DX
