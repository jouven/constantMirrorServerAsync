//20170914

//server/"source": the program has a configuration of several local paths which shares through tcp. It runs two tcp servers
//one to serve a list of the file paths with it's hash+size and another one to send the files.
//1 or more clients/"destination" (explained later) programs can connect to it and ask for list request or the file request
//also this program does requests to clients/"destination" programs to tell them there has been changes.
//right now it's command-line-always-running-process

//TODO-FUTURE 20200319 this program can be improved a lot adding a GUI also, doing an algorithm that fills the download queue depending on the speed and
//the queue of files to download the size of each file, e.g., if there are a lot of small files queue up more downloads since their size won't cap the network speed

#include "mirrorConfig.hpp"

#include "signalso/signal.hpp"

#include "essentialQtso/essentialQt.hpp"

#include <QCoreApplication>

#ifdef DEBUGJOUVEN
#include <iostream>
#endif

namespace
{

}

int main(int argc, char *argv[])
{
    MACRO_signalHandler

    //thread for the main program qt stuff
    //IMPORTANT this detached is necessary because were the QCoreApplication stuff on the main
    //it would get hang when a signal happened, signal hangs main() but not detached threads
    signalso::launchThread_f([&]()
    {
        QCoreApplication qtapp(argc, argv);

        QTimer::singleShot(0, std::bind(&mirrorConfig_c::initialSetup_f, &mirrorConfig_ext));

        returnValue_ext = qtapp.exec();
    });

    while (not signalso::isTheEnd_f())
    {}

    return returnValue_ext;
}
