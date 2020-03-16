#include "view.h"
#include "scene.h"

#include "sgviewcamera.h"

int main(int argc, char **argv)
{
    View::InitializeGlfw();

    {
        //Force Scene&View destroyed before View::FreeGlfw();
        SmartScene scene;

        if (argc > 1)
        {
            scene.setStreamPluginPath(argv[1]);
        }

        View view;
        //view.viewCamera()->setFillMode(true);
        view.attach(&scene);
        view.create("96board view");

        scene.startStream();

        View::exec();
    }

    View::FreeGlfw();
}
