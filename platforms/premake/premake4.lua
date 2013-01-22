--[[
	usage example:
		premake4 gmake && make config=release server

	only works with Linux for now
]]


local paths = {
	root = path.getabsolute("../../"), -- root openparsec folder
	premakefolder = path.getabsolute(""),
}

paths.parsecroot = path.join(paths.root, "parsec_root")


local srcpaths = {
	libraries = "../../src/libraries",
	libparsec = "../../src/libparsec",
	parsec = "../../src/parsec",
	parsecserver = "../../src/parsec_server",
}


solution "ParsecSolution"
	language "C++"
	platforms { "x32" }
	includedirs { srcpaths.libraries }
	
	configurations { "Debug", "Release" }
	
	configuration "Debug"
		defines { "DEBUG" }
		flags "Symbols"
			
	configuration "Release"
		flags "Optimize"
	
	configuration "linux"
		includedirs { "/usr/include", "/usr/local/include", "/opt/include" }
	

project "client"
	kind "WindowedApp"
	location "build/client"
	targetdir "build/client"
	targetname "Parsec"
	
	defines { "PARSEC_CLIENT" }
	
	includedirs {
		srcpaths.libparsec.."/include",
		srcpaths.parsec.."/include",
	}
	
	files {
		srcpaths.libraries.."/**",
		srcpaths.libparsec.."/**",
		srcpaths.parsec.."/**",
	}
	
	configuration "not windows"
		-- These are apparently windows thingies
		excludes { srcpaths.parsec.."/**.rc" }
	
	configuration "linux"
		targetname "parsec"
		links { "SDL", "SDL_mixer", "GL" }
		postbuildcommands {
			("cp parsec %q"):format(paths.parsecroot.."/client/"),
		}


project "server"
	kind "ConsoleApp"
	location "build/server"
	targetdir "build/server"
	targetname "parsec_server"
	
	defines { "PARSEC_SERVER" }
	
	includedirs {
		srcpaths.libparsec.."/include",
		srcpaths.parsecserver.."/include",
	}
	
	files {
		srcpaths.libparsec.."/**",
		srcpaths.parsecserver.."/**",
	}

	configuration "linux"
		links { "ncurses" }
		postbuildcommands {
			("cp parsec_server %q"):format(paths.parsecroot.."/server/"),
		}

