#include <File/FileUtils.h>
#include <Log/Log.h>

#include <array>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace DX
{
    std::optional<std::string> ReadAssetTextFile(const std::string& fileName)
    {
        auto fileNamePath = GetAssetPath() / fileName;
        if (!std::filesystem::exists(fileNamePath))
        {
            DX_LOG(Error, "FileUtils", "Filename path % s does not exist.", fileNamePath.generic_string().c_str());
            return std::nullopt;
        }

        if (std::ifstream file(fileNamePath);
            file.is_open())
        {
            std::ostringstream stringBuffer;
            stringBuffer << file.rdbuf();

            file.close();
            return stringBuffer.str();
        }
        else
        {
            DX_LOG(Error, "FileUtils", "Filename path %s failed to open.", fileNamePath.generic_string().c_str());
            return std::nullopt;
        }
    }

    std::optional<std::vector<uint8_t>> ReadAssetBinaryFile(const std::string& fileName)
    {
        auto fileNamePath = GetAssetPath() / fileName;
        if (!std::filesystem::exists(fileNamePath))
        {
            DX_LOG(Error, "FileUtils", "Filename path % s does not exist.", fileNamePath.generic_string().c_str());
            return std::nullopt;
        }

        // Open binary file seeking to the end of the stream immediately
        if (std::ifstream file(fileNamePath, std::ios::binary | std::ios::ate);
            file.is_open())
        {
            // Get file size and seek to the beginning of the file
            const std::streamsize fileSize = file.tellg();
            file.seekg(std::streamoff(0), std::ios_base::beg);

            // Read entire file
            std::vector<uint8_t> buffer(fileSize);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize))
            {
                file.close();
                DX_LOG(Error, "FileUtils", "Filename path %s failed to read.", fileNamePath.generic_string().c_str());
                return std::nullopt;
            }

            file.close();
            return buffer;
        }
        else
        {
            DX_LOG(Error, "FileUtils", "Filename path %s failed to open.", fileNamePath.generic_string().c_str());
            return std::nullopt;
        }
    }

    std::filesystem::path GetAssetPath()
    {
        auto execPath = GetExecutablePath();
        execPath /= "Assets";
        if (std::filesystem::exists(execPath))
        {
            return execPath;
        }

        // If the Assets folder is not in the same location as the executable,
        // that could be because the executable is being run from a build folder
        // (for example from Visual Studio). Try to find the 'build' folder to
        // extract the project path and use that to look for the Assets folder.
        if (auto it = execPath.generic_string().find("build");
            it != std::string::npos)
        {
            std::filesystem::path projectPath = execPath.generic_string().substr(0, it);
            projectPath /= "Assets";
            if (std::filesystem::exists(projectPath))
            {
                return projectPath;
            }
        }

        DX_LOG(Error, "FileUtils", "Assets path not found.");
        return {};
    }

    std::filesystem::path GetExecutablePath()
    {
#ifdef _WIN32
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        std::filesystem::path execPath(path);
        return execPath.remove_filename();
#else
        #error "Renderer::GetExecutablePath: Unsupported platform."
        return {};
#endif
    }
} // namespace DX
