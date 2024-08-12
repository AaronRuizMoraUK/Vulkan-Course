#pragma once

#include "Math/Vector3.h"

#include "mathfu/vector.h"

namespace Math
{
    // Color size might differ depending on platform and build configuration of mathfu (SIMD, padding, etc).
    // To serialize and deserialize as a flat array, use ColorPacked, which is a POD version of Color.
    using Color = mathfu::Vector<float, 4>;
    using ColorPacked = mathfu::VectorPacked<float, 4>;

    namespace Colors
    {
        // Basic Colors (CSS 1 standard)
        static const Color White{ 1.000f, 1.000f, 1.000f, 1.000f }; // RGB: (255, 255, 255)
        static const Color Silver{ 0.753f, 0.753f, 0.753f, 1.000f }; // RGB: (192, 192, 192)
        static const Color Gray{ 0.500f, 0.500f, 0.500f, 1.000f }; // RGB: (128, 128, 128)
        static const Color Black{ 0.000f, 0.000f, 0.000f, 1.000f }; // RGB: (0, 0, 0)
        static const Color Red{ 1.000f, 0.000f, 0.000f, 1.000f }; // RGB: (255, 0, 0)
        static const Color Maroon{ 0.500f, 0.000f, 0.000f, 1.000f }; // RGB: (128, 0, 0)
        static const Color Lime{ 0.000f, 1.000f, 0.000f, 1.000f }; // RGB: (0, 255, 0)
        static const Color Green{ 0.000f, 0.500f, 0.000f, 1.000f }; // RGB: (0, 128, 0)
        static const Color Blue{ 0.000f, 0.000f, 1.000f, 1.000f }; // RGB: (0, 0, 255)
        static const Color Navy{ 0.000f, 0.000f, 0.500f, 1.000f }; // RGB: (0, 0, 128)
        static const Color Yellow{ 1.000f, 1.000f, 0.000f, 1.000f }; // RGB: (255, 255, 0)
        static const Color Orange{ 1.000f, 0.647f, 0.000f, 1.000f }; // RGB: (255, 165, 0)
        static const Color Olive{ 0.500f, 0.500f, 0.000f, 1.000f }; // RGB: (128, 128, 0)
        static const Color Purple{ 0.500f, 0.000f, 0.500f, 1.000f }; // RGB: (128, 0, 128)
        static const Color Fuchsia{ 1.000f, 0.000f, 1.000f, 1.000f }; // RGB: (255, 0, 255)
        static const Color Teal{ 0.000f, 0.500f, 0.500f, 1.000f }; // RGB: (0, 128, 128)
        static const Color Aqua{ 0.000f, 1.000f, 1.000f, 1.000f }; // RGB: (0, 255, 255)
        // CSS3 colors
        // Reds
        static const Color IndianRed{ 0.804f, 0.361f, 0.361f, 1.000f }; // RGB: (205, 92, 92)
        static const Color LightCoral{ 0.941f, 0.502f, 0.502f, 1.000f }; // RGB: (240, 128, 128)
        static const Color Salmon{ 0.980f, 0.502f, 0.447f, 1.000f }; // RGB: (250, 128, 114)
        static const Color DarkSalmon{ 0.914f, 0.588f, 0.478f, 1.000f }; // RGB: (233, 150, 122)
        static const Color LightSalmon{ 1.000f, 0.627f, 0.478f, 1.000f }; // RGB: (255, 160, 122)
        static const Color Crimson{ 0.863f, 0.078f, 0.235f, 1.000f }; // RGB: (220, 20, 60)
        static const Color FireBrick{ 0.698f, 0.133f, 0.133f, 1.000f }; // RGB: (178, 34, 34)
        static const Color DarkRed{ 0.545f, 0.000f, 0.000f, 1.000f }; // RGB: (139, 0, 0)
        // Pinks                                                              
        static const Color Pink{ 1.000f, 0.753f, 0.796f, 1.000f }; // RGB: (255, 192, 203)
        static const Color LightPink{ 1.000f, 0.714f, 0.757f, 1.000f }; // RGB: (255, 182, 193)
        static const Color HotPink{ 1.000f, 0.412f, 0.706f, 1.000f }; // RGB: (255, 105, 180)
        static const Color DeepPink{ 1.000f, 0.078f, 0.576f, 1.000f }; // RGB: (255, 20, 147)
        static const Color MediumVioletRed{ 0.780f, 0.082f, 0.522f, 1.000f }; // RGB: (199, 21, 133)
        static const Color PaleVioletRed{ 0.859f, 0.439f, 0.576f, 1.000f }; // RGB: (219, 112, 147)
        // Oranges
        static const Color Coral{ 1.000f, 0.498f, 0.314f, 1.000f }; // RGB: (255, 127, 80)
        static const Color Tomato{ 1.000f, 0.388f, 0.278f, 1.000f }; // RGB: (255, 99, 71)
        static const Color OrangeRed{ 1.000f, 0.271f, 0.000f, 1.000f }; // RGB: (255, 69, 0)
        static const Color DarkOrange{ 1.000f, 0.549f, 0.000f, 1.000f }; // RGB: (255, 140, 0)
        // Yellows
        static const Color Gold{ 1.000f, 0.843f, 0.000f, 1.000f }; // RGB: (255, 215, 0)
        static const Color LightYellow{ 1.000f, 1.000f, 0.878f, 1.000f }; // RGB: (255, 255, 224)
        static const Color LemonChiffon{ 1.000f, 0.980f, 0.804f, 1.000f }; // RGB: (255, 250, 205)
        static const Color LightGoldenrodYellow{ 0.980f, 0.980f, 0.824f, 1.000f }; // RGB: (250, 250, 210)
        static const Color PapayaWhip{ 1.000f, 0.937f, 0.835f, 1.000f }; // RGB: (255, 239, 213)
        static const Color Moccasin{ 1.000f, 0.894f, 0.710f, 1.000f }; // RGB: (255, 228, 181)
        static const Color PeachPuff{ 1.000f, 0.855f, 0.725f, 1.000f }; // RGB: (255, 218, 185)
        static const Color PaleGoldenrod{ 0.933f, 0.910f, 0.667f, 1.000f }; // RGB: (238, 232, 170)
        static const Color Khaki{ 0.941f, 0.902f, 0.549f, 1.000f }; // RGB: (240, 230, 140)
        static const Color DarkKhaki{ 0.741f, 0.718f, 0.420f, 1.000f }; // RGB: (189, 183, 107)
        // Purples                                                                    
        static const Color Lavender{ 0.902f, 0.902f, 0.980f, 1.000f }; // RGB: (230, 230, 250)
        static const Color Thistle{ 0.847f, 0.749f, 0.847f, 1.000f }; // RGB: (216, 191, 216)
        static const Color Plum{ 0.867f, 0.627f, 0.867f, 1.000f }; // RGB: (221, 160, 221)
        static const Color Violet{ 0.933f, 0.510f, 0.933f, 1.000f }; // RGB: (238, 130, 238)
        static const Color Orchid{ 0.855f, 0.439f, 0.839f, 1.000f }; // RGB: (218, 112, 214)
        static const Color Magenta{ 1.000f, 0.000f, 1.000f, 1.000f }; // RGB: (255, 0, 255)
        static const Color MediumOrchid{ 0.729f, 0.333f, 0.827f, 1.000f }; // RGB: (186, 85, 211)
        static const Color MediumPurple{ 0.576f, 0.439f, 0.859f, 1.000f }; // RGB: (147, 112, 219)
        static const Color BlueViolet{ 0.541f, 0.169f, 0.886f, 1.000f }; // RGB: (138, 43, 226)
        static const Color DarkViolet{ 0.580f, 0.000f, 0.827f, 1.000f }; // RGB: (148, 0, 211)
        static const Color DarkOrchid{ 0.600f, 0.196f, 0.800f, 1.000f }; // RGB: (153, 50, 204)
        static const Color DarkMagenta{ 0.545f, 0.000f, 0.545f, 1.000f }; // RGB: (139, 0, 139)
        static const Color RebeccaPurple{ 0.400f, 0.200f, 0.600f, 1.000f }; // RGB: (102, 51, 153)
        static const Color Indigo{ 0.294f, 0.000f, 0.510f, 1.000f }; // RGB: (75, 0, 130)
        static const Color MediumSlateBlue{ 0.482f, 0.408f, 0.933f, 1.000f }; // RGB: (123, 104, 238)
        static const Color SlateBlue{ 0.416f, 0.353f, 0.804f, 1.000f }; // RGB: (106, 90, 205)
        static const Color DarkSlateBlue{ 0.282f, 0.239f, 0.545f, 1.000f }; // RGB: (72, 61, 139)
        // Greens
        static const Color GreenYellow{ 0.678f, 1.000f, 0.184f, 1.000f }; // RGB: (173, 255, 47)
        static const Color Chartreuse{ 0.498f, 1.000f, 0.000f, 1.000f }; // RGB: (127, 255, 0)
        static const Color LawnGreen{ 0.486f, 0.988f, 0.000f, 1.000f }; // RGB: (124, 252, 0)
        static const Color LimeGreen{ 0.196f, 0.804f, 0.196f, 1.000f }; // RGB: (50, 205, 50)
        static const Color PaleGreen{ 0.596f, 0.984f, 0.596f, 1.000f }; // RGB: (152, 251, 152)
        static const Color LightGreen{ 0.565f, 0.933f, 0.565f, 1.000f }; // RGB: (144, 238, 144)
        static const Color MediumSpringGreen{ 0.000f, 0.980f, 0.604f, 1.000f }; // RGB: (0, 250, 154)
        static const Color SpringGreen{ 0.000f, 1.000f, 0.498f, 1.000f }; // RGB: (0, 255, 127)
        static const Color MediumSeaGreen{ 0.235f, 0.702f, 0.443f, 1.000f }; // RGB: (60, 179, 113)
        static const Color SeaGreen{ 0.180f, 0.545f, 0.341f, 1.000f }; // RGB: (46, 139, 87)
        static const Color ForestGreen{ 0.133f, 0.545f, 0.133f, 1.000f }; // RGB: (34, 139, 34)
        static const Color DarkGreen{ 0.000f, 0.392f, 0.000f, 1.000f }; // RGB: (0, 100, 0)
        static const Color YellowGreen{ 0.604f, 0.804f, 0.196f, 1.000f }; // RGB: (154, 205, 50)
        static const Color OliveDrab{ 0.420f, 0.557f, 0.137f, 1.000f }; // RGB: (107, 142, 35)
        static const Color DarkOliveGreen{ 0.333f, 0.420f, 0.184f, 1.000f }; // RGB: (85, 107, 47)
        static const Color MediumAquamarine{ 0.400f, 0.804f, 0.667f, 1.000f }; // RGB: (102, 205, 170)
        static const Color DarkSeaGreen{ 0.561f, 0.737f, 0.561f, 1.000f }; // RGB: (143, 188, 143)
        static const Color LightSeaGreen{ 0.125f, 0.698f, 0.667f, 1.000f }; // RGB: (32, 178, 170)
        static const Color DarkCyan{ 0.000f, 0.545f, 0.545f, 1.000f }; // RGB: (0, 139, 139)
        // Blues
        static const Color Cyan{ 0.000f, 1.000f, 1.000f, 1.000f }; // RGB: (0, 255, 255)
        static const Color LightCyan{ 0.878f, 1.000f, 1.000f, 1.000f }; // RGB: (224, 255, 255)
        static const Color PaleTurquoise{ 0.686f, 0.933f, 0.933f, 1.000f }; // RGB: (175, 238, 238)
        static const Color Aquamarine{ 0.498f, 1.000f, 0.831f, 1.000f }; // RGB: (127, 255, 212)
        static const Color Turquoise{ 0.251f, 0.878f, 0.816f, 1.000f }; // RGB: (64, 224, 208)
        static const Color MediumTurquoise{ 0.282f, 0.820f, 0.800f, 1.000f }; // RGB: (72, 209, 204)
        static const Color DarkTurquoise{ 0.000f, 0.808f, 0.820f, 1.000f }; // RGB: (0, 206, 209)
        static const Color CadetBlue{ 0.373f, 0.620f, 0.627f, 1.000f }; // RGB: (95, 158, 160)
        static const Color SteelBlue{ 0.275f, 0.510f, 0.706f, 1.000f }; // RGB: (70, 130, 180)
        static const Color LightSteelBlue{ 0.690f, 0.769f, 0.871f, 1.000f }; // RGB: (176, 196, 222)
        static const Color PowderBlue{ 0.690f, 0.878f, 0.902f, 1.000f }; // RGB: (176, 224, 230)
        static const Color LightBlue{ 0.678f, 0.847f, 0.902f, 1.000f }; // RGB: (173, 216, 230)
        static const Color SkyBlue{ 0.529f, 0.808f, 0.922f, 1.000f }; // RGB: (135, 206, 235)
        static const Color LightSkyBlue{ 0.529f, 0.808f, 0.980f, 1.000f }; // RGB: (135, 206, 250)
        static const Color DeepSkyBlue{ 0.000f, 0.749f, 1.000f, 1.000f }; // RGB: (0, 191, 255)
        static const Color DodgerBlue{ 0.118f, 0.565f, 1.000f, 1.000f }; // RGB: (30, 144, 255)
        static const Color CornflowerBlue{ 0.392f, 0.584f, 0.929f, 1.000f }; // RGB: (100, 149, 237)
        static const Color RoyalBlue{ 0.255f, 0.412f, 0.882f, 1.000f }; // RGB: (65, 105, 225)
        static const Color MediumBlue{ 0.000f, 0.000f, 0.804f, 1.000f }; // RGB: (0, 0, 205)
        static const Color DarkBlue{ 0.000f, 0.000f, 0.545f, 1.000f }; // RGB: (0, 0, 139)
        static const Color MidnightBlue{ 0.098f, 0.098f, 0.439f, 1.000f }; // RGB: (25, 25, 112)
        // Browns
        static const Color Cornsilk{ 1.000f, 0.973f, 0.863f, 1.000f }; // RGB: (255, 248, 220)
        static const Color BlanchedAlmond{ 1.000f, 0.922f, 0.804f, 1.000f }; // RGB: (255, 235, 205)
        static const Color Bisque{ 1.000f, 0.894f, 0.769f, 1.000f }; // RGB: (255, 228, 196)
        static const Color NavajoWhite{ 1.000f, 0.871f, 0.678f, 1.000f }; // RGB: (255, 222, 173)
        static const Color Wheat{ 0.961f, 0.871f, 0.702f, 1.000f }; // RGB: (245, 222, 179)
        static const Color BurlyWood{ 0.871f, 0.722f, 0.529f, 1.000f }; // RGB: (222, 184, 135)
        static const Color Tan{ 0.824f, 0.706f, 0.549f, 1.000f }; // RGB: (210, 180, 140)
        static const Color RosyBrown{ 0.737f, 0.561f, 0.561f, 1.000f }; // RGB: (188, 143, 143)
        static const Color SandyBrown{ 0.957f, 0.643f, 0.376f, 1.000f }; // RGB: (244, 164, 96)
        static const Color Goldenrod{ 0.855f, 0.647f, 0.125f, 1.000f }; // RGB: (218, 165, 32)
        static const Color DarkGoldenrod{ 0.722f, 0.525f, 0.043f, 1.000f }; // RGB: (184, 134, 11)
        static const Color Peru{ 0.804f, 0.522f, 0.247f, 1.000f }; // RGB: (205, 133, 63)
        static const Color Chocolate{ 0.824f, 0.412f, 0.118f, 1.000f }; // RGB: (210, 105, 30)
        static const Color SaddleBrown{ 0.545f, 0.271f, 0.075f, 1.000f }; // RGB: (139, 69, 19)
        static const Color Sienna{ 0.627f, 0.322f, 0.176f, 1.000f }; // RGB: (160, 82, 45)
        static const Color Brown{ 0.647f, 0.165f, 0.165f, 1.000f }; // RGB: (165, 42, 42)
        // Whites
        static const Color Snow{ 1.000f, 0.980f, 0.980f, 1.000f }; // RGB: (255, 250, 250)
        static const Color Honeydew{ 0.941f, 1.000f, 0.941f, 1.000f }; // RGB: (240, 255, 240)
        static const Color MintCream{ 0.961f, 1.000f, 0.980f, 1.000f }; // RGB: (245, 255, 250)
        static const Color Azure{ 0.941f, 1.000f, 1.000f, 1.000f }; // RGB: (240, 255, 255)
        static const Color AliceBlue{ 0.941f, 0.973f, 1.000f, 1.000f }; // RGB: (240, 248, 255)
        static const Color GhostWhite{ 0.973f, 0.973f, 1.000f, 1.000f }; // RGB: (248, 248, 255)
        static const Color WhiteSmoke{ 0.961f, 0.961f, 0.961f, 1.000f }; // RGB: (245, 245, 245)
        static const Color Seashell{ 1.000f, 0.961f, 0.933f, 1.000f }; // RGB: (255, 245, 238)
        static const Color Beige{ 0.961f, 0.961f, 0.863f, 1.000f }; // RGB: (245, 245, 220)
        static const Color OldLace{ 0.992f, 0.961f, 0.902f, 1.000f }; // RGB: (253, 245, 230)
        static const Color FloralWhite{ 1.000f, 0.980f, 0.941f, 1.000f }; // RGB: (255, 250, 240)
        static const Color Ivory{ 1.000f, 1.000f, 0.941f, 1.000f }; // RGB: (255, 255, 240)
        static const Color AntiqueWhite{ 0.980f, 0.922f, 0.843f, 1.000f }; // RGB: (250, 235, 215)
        static const Color Linen{ 0.980f, 0.941f, 0.902f, 1.000f }; // RGB: (250, 240, 230)
        static const Color LavenderBlush{ 1.000f, 0.941f, 0.961f, 1.000f }; // RGB: (255, 240, 245)
        static const Color MistyRose{ 1.000f, 0.894f, 0.882f, 1.000f }; // RGB: (255, 228, 225)
        // Grays
        static const Color Gainsboro{ 0.863f, 0.863f, 0.863f, 1.000f }; // RGB: (220, 220, 220)
        static const Color LightGray{ 0.827f, 0.827f, 0.827f, 1.000f }; // RGB: (211, 211, 211)
        static const Color LightGrey{ 0.827f, 0.827f, 0.827f, 1.000f }; // RGB: (211, 211, 211)
        static const Color DarkGray{ 0.663f, 0.663f, 0.663f, 1.000f }; // RGB: (169, 169, 169)
        static const Color DarkGrey{ 0.663f, 0.663f, 0.663f, 1.000f }; // RGB: (169, 169, 169)
        static const Color Grey{ 0.502f, 0.502f, 0.502f, 1.000f }; // RGB: (128, 128, 128)
        static const Color DimGray{ 0.412f, 0.412f, 0.412f, 1.000f }; // RGB: (105, 105, 105)
        static const Color DimGrey{ 0.412f, 0.412f, 0.412f, 1.000f }; // RGB: (105, 105, 105)
        static const Color LightSlateGray{ 0.467f, 0.533f, 0.600f, 1.000f }; // RGB: (119, 136, 153)
        static const Color LightSlateGrey{ 0.467f, 0.533f, 0.600f, 1.000f }; // RGB: (119, 136, 153)
        static const Color SlateGray{ 0.439f, 0.502f, 0.565f, 1.000f }; // RGB: (112, 128, 144)
        static const Color SlateGrey{ 0.439f, 0.502f, 0.565f, 1.000f }; // RGB: (112, 128, 144)
        static const Color DarkSlateGray{ 0.184f, 0.310f, 0.310f, 1.000f }; // RGB: (47, 79, 79)
        static const Color DarkSlateGrey{ 0.184f, 0.310f, 0.310f, 1.000f }; // RGB: (47, 79, 79)
    }

    // Create color passing float values in the range [0.0f, 1.0f]
    inline Color CreateColor(float r, float g, float b, float a = 1.0f)
    {
        return Color(r, g, b, a);
    }

    // Create color passing float values using a vector in the range [0.0f, 1.0f]
    inline Color CreateColor(const Vector3& rgb, float a = 1.0f)
    {
        return Color(rgb, a);
    }

    // Create color passing byte values in the range [0, 255]
    inline Color CreateColor(std::byte r, std::byte g, std::byte b, std::byte a = static_cast<std::byte>(255))
    {
        return Color(static_cast<float>(r), static_cast<float>(g),
                     static_cast<float>(b), static_cast<float>(a)) / 255.0f;
    }
}  // namespace Math
