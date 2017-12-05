# constantMirrorServerAsync
Server-side program that mirrors remote paths to the local filesystem using tcp sockets

Works with https://github.com/jouven/constantMirrorClientAsync

Compilation
-----------
Requires:

Qt library

https://github.com/jouven/signalso

https://github.com/jouven/fileHashQtso

https://github.com/jouven/baseClassQtso

https://github.com/jouven/essentialQtso

https://github.com/jouven/threadedFunctionQtso

https://github.com/jouven/qmutexUMapQtso

https://github.com/jouven/sslUtilsso

Openssl

Probably a header from https://github.com/jouven/comuso

Run (in constantMirrorServerAsync source directory or pointing to it):

    qmake

and then:

    make
