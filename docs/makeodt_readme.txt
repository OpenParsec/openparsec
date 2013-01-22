
------------------------------------------------------------------------------
makeodt: The Parsec 3d Model Utility
------------------------------------------------------------------------------
mailto:parsec@parsec.org                               http://www.parsec.org/
------------------------------------------------------------------------------


------------------------------------------------------------------------------
1. INTRODUCTION
------------------------------------------------------------------------------

This distribution is geared towards an artistic/technical audience that wants
to design their own spacecraft and other 3d objects and import them in Parsec.

It contains the utility we are using for converting 3d models into the binary
object format Parsec uses (.od2). The tool is called makeodt, since the
original format was called .odt ("object data format"), but by now Parsec
mostly uses its second incarnation, consequently called .od2 (and the
supplied version of makeodt exclusively generates .od2 files).

Please bear in mind right from the start that designing, converting, and
importing your own 3d models into Parsec is by no means an easy task.

There are quite some shortcomings and tricky parts to the conversion process,
and especially on the front end side we could have done a lot better. For
exactly this reason, we are not only providing binaries for Win32, Linux,
and MacOS X, but also the complete source code (licensed under the LGPL,
see below). If you would like to have additional input formats, provide
more object information, build a graphical front end, or anything else you
find wanting, please, by all means, feel free to go ahead and plug it right
in.

Actually, makeodt is an extremely tiny front end to a quite powerful library
called BspLib, that we have developed some time ago. The main purpose of this
library was compilation of BSP trees, but this functionality is not exported
by makeodt, since the current Parsec models do not employ BSP trees at all.
If you have a look at the BspLib source and ask yourself why it is that
large, well, it's able to do a lot more than just convert objects, and it
also contains quite some legacy stuff (especially with regard to input file
formats).

The primary input format that makeodt is able to read is VRML V1.0, so the
main requirement for building your own models is that you are somehow able to
export your mesh in this format. Next, your textures have to be in either
.tga, .jpg, or .3df format, since makeodt needs to read the dimensions of your
textures during model conversion, and it knows only these image file formats.

As soon as you have converted your model into an .od2 file, you will have to
write a Parsec console script for loading it within the game. Especially for
spacecraft (with the corresponding locations for firing missiles, lasers,
and the like) this is not a task accomplished in a mere five minutes.

Finally, for redistribution, you should package all the data (the model
itself, the textures, and the console script) for your model(s) into a single
file, which you can easily give to your friends and make available on the net.


------------------------------------------------------------------------------
2. LICENSE
------------------------------------------------------------------------------

The license for the BspLib and makeodt sources is the GNU LGPL, see the
included files called COPYING.

QvLib, the VRML 1.0 parsing library by Paul S. Strauss of SGI, which
BspLib is using in order to read VRML1 files, can be used, modified,
and distributed freely. The included version has been modified slightly.
The original version can be found at
ftp://ftp.sgi.com/sgi/inventor/2.0/qv1.0.tar.Z
for instance.


------------------------------------------------------------------------------
3. MODEL CONVERSION OVERVIEW
------------------------------------------------------------------------------

To plunge right in, these are the nine steps we usually run through in order
to get a model (a 3d object) into Parsec:

1. design the model in some external editor
2. export it in VRML V1.0 format, or convert an exported format into VRML1
3. put the .wrl file together with all textures in a single directory
4. fix the bugs the VRML exporter has put in (your mileage may vary ;)
5. invoke makeodt in this directory (this generates the .od2 file)
6. copy the textures and the .od2 file into a single directory
7. either turn this directory into a package or copy it into the parsec dir
8. write the console script for loading the model, textures, and shaders
9. execute the loading script in the parsec console (usually automatically)

As you can see, makeodt is only part of the whole process (namely, step 5).
Everything else is concerned with design, file management, and putting
together additional information (in the console script).

If you want to distribute your model, or have everything nice and tidy after
you have finalized it, you put all the corresponding files in a single
package, see section 7 below.


------------------------------------------------------------------------------
4. COMMAND LINE OPTIONS
------------------------------------------------------------------------------

makeodt itself only takes two input parameters, the name of the file you want
to convert, and an optional scale factor:

1. input file specification: "-i <filename_no_wrl_extension>"
2. scale factor specification: "-s <float>"

We determine the exact scale factor we want to use for each model by trial
and error.

makeodt requires all textures to be in the same directory as the .wrl file,
in either .tga, .jpg, or .3df format, and generates a corresponding .od2 file.
Parsec itself is also able to read textures from these three image file
formats.


------------------------------------------------------------------------------
5. LOADING A MODEL IN PARSEC
------------------------------------------------------------------------------

Loading a model in Parsec (from a console script) is usually done in the
following order:

  1. load all textures. they must be available before the actual model gets
     loaded. if they are not, the model won't load.

  2. load the model (the actual object). this might actually consist of
     loading several objects (one for each level of detail), together with
     specifying switching thresholds in terms of viewing distance.

  3. define all shaders (if any). this allows to create texture and color
     animations, specify transparency, etc.

  4. attach the shaders to the respective parts of the model.

If the model is a spacecraft, the following steps also apply:

  5. designate the model as a spacecraft, so players will actually be able to
     select it in the spacecraft viewer. this also allows to specify a texture
     for the little image appearing in the monitors on the upper left and
     right corners of the cockpit.

  6. specify a description text for the spacecraft, that will be displayed in
     the spacecraft viewer.

  7. specify properties like maximum damage, number of missiles the ship can
     carry, and so on; but also purely geometric properties that are not
     contained in the model, like the positions where lasers and missiles
     should appear when fired.

We will elaborate a bit on each of these steps and illustrate the
corresponding console commands by using examples:

  1. "load texture texname file texname.tga"
     this will load a texture called "texname" from a file called
     "texname.tga". these two names are completely unrelated, within the
     engine only the former matters.

  2. "load object firebird file firebird.od2"
     this will load an object class called "firebird" from a file called
     "firebird.od2". these two names are completely unrelated, within the
     engine only the former matters.

     "load object test file (t0.od2 t1.od2) lodmag (110) lodmin (160)"
     this will load an object class called "test" containing two levels
     of detail from the files "t0.od2" and "t1.od2". the switching thresholds
     are distances and are specified as a hysteresis. i.e., magnification
     should occur closer (smaller value) to the viewer than minification
     (larger value). this avoids nervous toggling at a certain fixed viewing
     distance.

  3. "shader.def ex_energy_tube shader (iter_texrgb iter_alphablend)"
     shaders are beings of their own, please see chapter 11 in the
     parsec thesis distribution for a discussion.

  4. "shader.set class (test) lod -2 shader test_hull texture test"
     same as 3.

For further examples, please refer to the included example console scripts.


------------------------------------------------------------------------------
6. MODEL SHADERS
------------------------------------------------------------------------------

Shaders are mostly important for animated objects. Simple objects, and static
spacecraft do not necessarily need them.

Please refer to chapter 11 (Shaders) and chapter 14 (The Command Console) in
the parsec thesis distribution for a detailed discussion (see section 10
below for download location).


------------------------------------------------------------------------------
7. BUILDING AND REGISTERING A DATA PACKAGE
------------------------------------------------------------------------------

In order to avoid clobbering your Parsec directory (and everybody else's)
with dozens or even hundreds of small files, we strongly suggest packaging
all your stuff into Parsec packages, similar to the original distribution.

A Parsec package is simply a collection of files stored in a single file
with a simple header tacked onto it. The package creator/extractor is part
of the Parsec executable itself.

In order to extract the files contained in a Parsec package (pscdata0.dat,
pscdata2.dat, or pscdata3.dat, for instance) you invoke the Parsec executable
with the following command line parameters:

-getpack --pack <packname> --list <listname>

This will extract <packname> into the current directory and also store a
list of the extracted files in <listname>. The "--list" option is actually
optional, but it's handy to have this list available, especially if you
want to repackage the extracted files, for which you will need just such
a list.

In order to create a package you invoke Parsec with these parameters:

-makepack --pack <packname> --list <listname>

which creates the package <packname> by packaging all the files listed in
<listname> into it (they won't be compressed, just put into a single file,
just like a .tar package, although in a different binary format).

When dealing with packages, you always have to keep in mind that the file
names of the packaged files must not be longer than fifteen characters.
In the future we will probably lift this legacy restriction, but for the
time being, keep your file names short.

When you have packaged your files, you of course want to be able to have
Parsec load files from it. This won't happen automatically, but there are
multiple (and not too complicated) ways for registering your new package
with Parsec. The method you actually use usually depends on whether you
are testing your package, or whether you want to read data from a finalized
package (which probably also means a person other than you wants to do this).

First, you can list all already registered packages in the command console
by using the "listdata packages" command. When Parsec is already running
you can add additional packages to this list by using the
"package register <packname>" command. Be sure to include the extension.

You can also register packages on startup with the --pack command line
option.

However, both of these methods won't automatically load anything, because
no console scripts get executed when the package is registered. The package
and all the files it contains are simply added to the search path.

In order to do this, you have to register a package as a mod. What happens
when you do this is that a console script called "boot.con" will automatically
be executed for you, provided that you include such a script in your package.

When your register a mod (using either the --mod, or the --modforce command
line option, see below), you actually register a directory. This directory
must then contain a .dat file with the same name (excluding the extension)
as the directory/mod name itself.

For example, say you want to register a mod simply containing an additional
ship called "darkwing", you do this:

1. create darkwing.dat containing a boot.con script, the .od2 file (which
   need not be called darkwing.od2, the only "special" name is boot.con),
   and all the textures and maybe additional scripts for loading the ship.

2. put darkwing.dat in a darkwing subdir (making it darkwing/darkwing.dat)

3. start Parsec with --mod darkwing

Of course, boot.con has to invoke the console script loading The Darkwing.
(It's good practice not to put a lot into boot.con itself, but to simply
delegate to other scripts.) Or any additional number of ships, for that
matter.

Now for the difference between the --mod and --modforce options. For the
time being, --mod is what you usually want to use, because it adds the
mod packages _after_ the official Parsec packages. In contrast to this,
the --modforce option bypasses these packages entirely, which means your
mod package(s) have to provide _all_ the data in Parsec (or your own
totally modified version of it), for that matter.

As long as they are data-only mods (which, currently, all mods are), you can
register multiple mods by simply using multiple --mod options on the same
command line, like:

--mod darkwing --mod evilrazor

or whatever.


------------------------------------------------------------------------------
8. THE ART OF DESIGNING A PARSEC MODEL
------------------------------------------------------------------------------

Now that we have at least briefly covered the technical aspects of importing
a model into Parsec, there are lots of additional issues involved. We will
try to mention some of them here.

First, textures are costly. Both in terms of memory and in terms of state
switches during rendering. If the latter doesn't strike a bell, just let it
suffice that the fewer different textures an object uses, the better.
From five to ten textures per spacecraft, and just one or two for a
power-up should do the trick.

Also, all textures have to be a power of two in width and height, e.g.,
256x128, 128x32, and so on.

The older Parsec models are all using a maximum texture resolution of 256x256
which is a good compromise between memory consumption and quality. However,
all the original artwork was done in four times the in-game resolution in
order to prepare for hardware that is able to handle larger textures.

Indeed, with the advent of texture compression, textures up to 1024 along one
dimension have become feasible. So by now the latest ships of Parsec are
able to use the original artwork.

Second, the Parsec engine is not a high polygon count engine. It supports
a lot of different (graphics hardware) platforms and has been developed over
several years, which means the number of polygons you can push through a
state-of-the-art graphics card don't mean all that much to it.

Which is to say: use as few polygons as possible, a couple hundred should be
enough for one spacecraft (say, up to 600).

Finally, we would like to recommend browsing through all the existing Parsec
artwork (especially the pscdata2.dat and pscdata3.dat packages) to have
a reference how we are using all this. Especially the syntax and usage of
console commands can be best gleaned from there, since we haven't covered all
of them in this short document.

If you want to learn how to use shaders, the _powups.con console script is
the best place to start. The scripts for loading spacecraft are called
_f1_2.con, _f2_2.con, and so on.


------------------------------------------------------------------------------
9. INTERNAL STRUCTURE
------------------------------------------------------------------------------

For those of you desiring to use the source, here are a couple of words about
its structure.

First, the core of makeodt is the BspLib, which resides in tool_src/BspLib.
It's a C++ class library that is utilized by makeodt. If you're interested
in BspLib itself, there is still a web page describing some of its innards
(http://www.cg.tuwien.ac.at/~msh/bsplib/). In case you are wondering, this
source has nothing to do with the Parsec source itself whatsoever. So, you
don't have to have a look at it if you want to work on the Parsec source.
future. Parsec deals with objects, geometry, and the like in an entirely
different way.

Second, for parsing VRML V1.0 files we use a very slightly modified version
of QvLib, a public domain library for parsing such files, which was publicly
released by SGI quite some time ago. It resides in tool_src/QvLib.

Third, there are two files of the actual Parsec source, which the BspLib
needs in order to be able to save Parsec object files. These are od_geomv.h
and od_odt.h which can be found in game_src/common/include.

Last, but not least, there are a couple of files in tool_src/makeodt which
glues all of the above together in a very simple command line tool.


------------------------------------------------------------------------------
10. RESOURCES
------------------------------------------------------------------------------

The primary source for more in-depth Parsec information is the Parsec
thesis ("Design and Architecture of a Portable and Extensible Multiplayer
3D Game Engine"), which is available for download from the Parsec download
page (http://www.parsec.org/download.html "PARSEC SDK DOWNLOADS").

