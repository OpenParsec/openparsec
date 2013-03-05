To build OpenParsec:

On Linux:
- Install premake or grab the premake4 binary and put it somewhere in your $PATH (http://industriousone.com/premake/download)
- cd into platforms/premake/
- run: premake4 gmake
- To build the client: make client
- To build the server: make server
- To build both targets: make


NOTES ON LINUX:
- There is no install script yet
- binaries get copied into parsec_root/{client,server}
- You will need a copy of the openparsec-assets to run the client (https://github.com/OpenParsec/openparsec-assets)

