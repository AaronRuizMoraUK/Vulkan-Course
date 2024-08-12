#include <Application.h>
#include <Log/Log.h>

int main()
{
    //const Math::Vector2Int windowSize{ 3440, 1440 };
    //const int refreshRate = 144;
    //const bool fullScreen = true;
    //const bool vSync = true;

    const Math::Vector2Int windowSize{ 1280, 720 };
    const int refreshRate = 60;
    const bool fullScreen = false;
    const bool vSync = true;

    if (DX::Application app;
        app.Initialize(windowSize, refreshRate, fullScreen, vSync))
    {
        app.RunLoop();

        app.Terminate();
    }

    DX_LOG(Info, "Main", "Done!");
    return 0;
}
