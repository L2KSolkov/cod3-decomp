#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

unsigned int AeHash(const char* str);

namespace fs = std::filesystem;

static std::uint32_t read_u32(const std::vector<std::uint8_t>& bytes,
                              std::size_t offset)
{
    return static_cast<std::uint32_t>(bytes[offset])
         | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8)
         | (static_cast<std::uint32_t>(bytes[offset + 2]) << 16)
         | (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

static bool has_marker(const std::vector<std::uint8_t>& bytes,
                       std::size_t offset, const char* marker)
{
    return bytes[offset] == static_cast<std::uint8_t>(marker[0])
        && bytes[offset + 1] == static_cast<std::uint8_t>(marker[1])
        && bytes[offset + 2] == static_cast<std::uint8_t>(marker[2])
        && bytes[offset + 3] == static_cast<std::uint8_t>(marker[3]);
}

int main(int argc, char** argv)
{
    const fs::path root = argc > 1 ? fs::path(argv[1]) : fs::path("GameData");
    if (!fs::is_directory(root))
    {
        std::cerr << "gamedata root is not a directory: " << root << '\n';
        return 2;
    }

    std::unordered_set<std::uint32_t> path_hashes;
    std::size_t files = 0;
    std::uint64_t total_bytes = 0;
    std::vector<std::string> failures;

    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".cod")
            continue;

        ++files;
        const fs::path relative = fs::relative(entry.path(), root);
        std::string asset_name = relative.generic_string();
        std::transform(asset_name.begin(), asset_name.end(), asset_name.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        const std::uint32_t name_hash = AeHash(asset_name.c_str());
        if (!path_hashes.insert(name_hash).second)
            failures.push_back(asset_name + ": AeHash collision");

        std::ifstream stream(entry.path(), std::ios::binary);
        const std::vector<std::uint8_t> bytes(
            (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        total_bytes += bytes.size();

        if (bytes.size() < 32)
        {
            failures.push_back(asset_name + ": truncated KAPF header");
            continue;
        }
        if (!has_marker(bytes, 0, "KAPF"))
            failures.push_back(asset_name + ": missing KAPF signature");
        if (read_u32(bytes, 4) != 0x4003D70Au)
            failures.push_back(asset_name + ": unexpected KAPF format word");

        const std::uint32_t header_size = read_u32(bytes, 8);
        const std::uint32_t section_count = read_u32(bytes, 12);
        if ((header_size != 44 && header_size != 48) || section_count < 1
            || section_count > 3)
        {
            failures.push_back(asset_name + ": invalid header dimensions");
            continue;
        }
        if (!has_marker(bytes, 28, "YAMA"))
            failures.push_back(asset_name + ": missing YAMA marker");

        std::uint32_t previous = header_size;
        for (std::uint32_t i = 0; i < section_count; ++i)
        {
            const std::uint32_t offset = read_u32(bytes, 16 + i * 4);
            if (offset < previous || offset > bytes.size())
            {
                failures.push_back(asset_name + ": invalid section offset");
                break;
            }
            previous = offset;
        }
    }

    std::cout << "validated_cod_files=" << files
              << " total_bytes=" << total_bytes
              << " failures=" << failures.size() << '\n';
    for (const std::string& failure : failures)
        std::cerr << failure << '\n';
    return failures.empty() && files != 0 ? 0 : 1;
}
