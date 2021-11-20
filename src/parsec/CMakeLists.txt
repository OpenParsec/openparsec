##
##  This is the cmake intro
cmake_minimum_required(VERSION 3.10)
##
##  Abort if in source build
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

project(parsec CXX)
##
##  Load values from the os-release information.
if (EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" OSINFO)
    foreach(NameAndValue ${OSINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        string(REPLACE "\"" "" Value ${Value})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
else()
    message(WARNING "os-release info not found. Assuming a windows build.")
endif()
##
##  Load openparsec version number
if (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
    file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION VINFO)
    foreach(NameAndValue ${VINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
    set(OPENPARSEC_VERSION_MAJOR "${VERSION_MAJOR}")
    set(OPENPARSEC_VERSION_MINOR "${VERSION_MINOR}")
    set(OPENPARSEC_VERSION_PATCH "${VERSION_PATCH}")
else()
    message(WARNING "version info not found")
    set(OPENPARSEC_VERSION_MAJOR "0")
    set(OPENPARSEC_VERSION_MINOR "2")
    set(OPENPARSEC_VERSION_PATCH "1")
endif()
##
##  Here we look for the OpenGL support.
set(OpenGL_GL_PREFERENCE GLVND)
find_package (OpenGL COMPONENTS OpenGL REQUIRED)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(OPENPARSEC_GL_LIB ${OPENGL_gl_LIBRARY})
    set(OPENPARSEC_GLU_LIB ${OPENGL_glu_LIBRARY})
else()
    set(OPENPARSEC_TARGETS OpenGL::OpenGL OpenGL::GLU Freetype::Freetype)
endif()

##
##  Here we try to find the freetype library support.

set(OPENPARSEC_FREETYPE_LIB "freetype")

find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
)
##
##  This is the application install dir. Set fixed for now. Should be changed.
set (XDG_APPS_INSTALL_DIR /usr/local/bin)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(FeatureSummary)
include(GNUInstallDirs)


add_feature_info(FREETYPE_FOUND "libfreetype" "FreeType lib is needed to build the openparsec project" "http://freetype.sf.net" FALSE "" "")
##
##  
include_directories(${FREETYPE_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})

add_compile_options (-O0 -g -Wall -std=c++14 -fPIC -DPARSEC_CLIENT)
link_libraries(SDL2_mixer)

include_directories(${CMAKE_SOURCE_DIR} ../libparsec/include include ../libraries)
##  Static linkage to the povmodeler sub-systems.
##  Using --start-group/--end-group to have a recursive symbol resolution.

add_executable(${PROJECT_NAME}
    aud_bkgn.cpp
    aud_comm.cpp
    aud_game.cpp
    aud_glob.cpp
    aud_sdlmixer.cpp
    aud_supp.cpp
    con_act.cpp
    con_aux.cpp
    con_cani.cpp
    con_com.cpp
    con_conf.cpp
    con_ext.cpp
    con_geom.cpp
    con_info.cpp
    con_int.cpp
    con_kmap.cpp
    con_list.cpp
    con_load.cpp
    con_main.cpp
    con_path.cpp
    con_rc.cpp
    con_say.cpp
    con_shad.cpp
    con_std.cpp
    con_tani.cpp
    d_glob.cpp
    d_misc.cpp
    do_bmap.cpp
    do_font.cpp
    do_iter.cpp
    do_misc.cpp
    do_patch.cpp
    e_callbk.cpp
    e_colani.cpp
    e_color.cpp
    e_demo.cpp
    e_draw.cpp
    e_events.cpp
    e_getopt.cpp
    e_global.cpp
    e_level.cpp
    e_loader.cpp
    e_mouse.cpp
    e_record.cpp
    e_replay.cpp
    e_shader.cpp
    e_supp.cpp
    e_texani.cpp
    e_vtxani.cpp
    g_boot.cpp
    g_bot_cl.cpp
    g_camera.cpp
    g_cloud.cpp
    g_explod.cpp
    g_gameloop.cpp
    g_global.cpp
    g_laser.cpp
    g_planet.cpp
    g_sfx.cpp
    g_shkwav.cpp
    g_stars.cpp
    g_stgate.cpp
    g_supp.cpp
    g_swarm.cpp
    g_vapor.cpp
    g_wfx.cpp
    h_cockpt.cpp
    h_drwhud.cpp
    h_frmplt.cpp
    h_global.cpp
    h_radar.cpp
    h_strmap.cpp
    h_supp.cpp
    h_text.cpp
    img_3df.cpp
    img_api.cpp
    img_conv.cpp
    img_jpg.cpp
    img_load.cpp
    img_supp.cpp
    img_tga.cpp
    inp_comm.cpp
    inp_glob.cpp
    inp_init.cpp
    inp_sdl.cpp
    inp_user.cpp
    isdl_joy.cpp
    isdl_mouse.cpp
    isdl_supp.cpp
    m_button.cpp
    m_list.cpp
    m_logo.cpp
    m_main.cpp
    m_option.cpp
    m_status.cpp
    m_viewer.cpp
    net_conn.cpp
    net_game.cpp
    net_game_gmsv.cpp
    net_game_peer.cpp
    net_glob.cpp
    net_gmsv.cpp
    net_pckt.cpp
    net_pckt_peer.cpp
    net_peer.cpp
    net_prediction.cpp
    net_rmev.cpp
    net_rmev_gmsv.cpp
    net_rmev_peer.cpp
    net_serv.cpp
    net_udp.cpp
    net_univ.cpp
    net_util_cl.cpp
    nl_udp.cpp
    nw_udp.cpp
    obj_ani.cpp
    obj_coll.cpp
    obj_comm.cpp
    obj_ctrl.cpp
    obj_expl.cpp
    obj_game.cpp
    obj_iter.cpp
    obj_part.cpp
    obj_vani.cpp
    obj_xtra.cpp
    part_ani.cpp
    part_api.cpp
    part_def.cpp
    part_sys.cpp
    r_glob.cpp
    ro_api.cpp
    ro_obj.cpp
    ro_part.cpp
    ro_patch.cpp
    ro_perf.cpp
    ro_poly.cpp
    ro_sfx.cpp
    ro_supp.cpp
    sl_err.cpp
    sl_main.cpp
    sl_msg.cpp
    sl_opt.cpp
    snd_api.cpp
    snd_wav.cpp
    sys_bind.cpp
    sys_conv.cpp
    sys_glob.cpp
    t_pack.cpp
    vid_glob.cpp
    vid_init.cpp
    vid_plug.cpp
    vid_supp.cpp
    vsdl_buffo.cpp
    vsdl_inito.cpp
    vsdl_ogl.cpp
    vsdl_suppo.cpp


    ../libparsec/net_stream.cpp
    ../libparsec/con_arg.cpp
    ../libparsec/g_extra.cpp
    ../libparsec/utl_bsp.cpp
    ../libparsec/e_relist.cpp
    ../libparsec/obj_cust.cpp
    ../libparsec/g_emp.cpp
    ../libparsec/g_telep.cpp
    ../libparsec/sl_path.cpp
    ../libparsec/utl_cull.cpp
    ../libparsec/net_swap.cpp
    ../libparsec/utl_clpo.cpp
    ../libparsec/net_pckt_gmsv.cpp
    ../libparsec/utl_math2.cpp
    ../libparsec/obj_type.cpp
    ../libparsec/con_tab.cpp
    ../libparsec/con_vald.cpp
    ../libparsec/obj_creg.cpp
    ../libparsec/obj_odt.cpp
    ../libparsec/sys_date.cpp
    ../libparsec/debug.cpp
    ../libparsec/sys_swap.cpp
    ../libparsec/net_util.cpp
    ../libparsec/sw_path.cpp
    ../libparsec/sys_file.cpp
    ../libparsec/e_modulemanager.cpp
    ../libparsec/utl_clip.cpp
    ../libparsec/net_wrap.cpp
    ../libparsec/utl_logfile.cpp
    ../libparsec/sw_timer.cpp
    ../libparsec/obj_clas.cpp
    ../libparsec/sl_timer.cpp
    ../libparsec/utl_math.cpp
    )

target_link_libraries(${PROJECT_NAME} OpenGL::OpenGL GLEW::GLEW SDL2::SDL2)
##
##
##  Here we describe the files and positions of
##  to be installed files.
## install( PROGRAMS ${CMAKE_BINARY_DIR}/povmodeler DESTINATION  ${CMAKE_INSTALL_BINDIR} )
##
##  desktop file
## install( FILES povmodeler.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
##
##  application specific files
## install( FILES povraydocmap.xml baseinsertrules.xml questionmark.png DESTINATION  ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler)
##
## Examples and libraries.
## install( DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler/examples )
## install( DIRECTORY library/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler/library )
##
##  Icon files in various resolutions
## install( FILES hi48-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps/ RENAME povmodeler.png)
## install( FILES hi32-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps/ RENAME povmodeler.png)
## install( FILES hi22-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/22x22/apps/ RENAME povmodeler.png)
## install( FILES hi16-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps/ RENAME povmodeler.png)
##
##  Packaging setup.
##
##  Only supported on unix systems.
## IF (UNIX)
##
##  Debian specific switches
## SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
## SET (CPACK_DEBIAN_PACKAGE_DEBUG ON)
##
##  RPM specific switches
## set (CPACK_RPM_PACKAGE_DEBUG OFF)
## set (CPACK_RPM_PACKAGE_AUTOREQ ON)
##
##  Some systems do not like it to have the directories in the files to create
## set (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/applications"
##                                                    "/usr/share/icons"
##                                                    "/usr/share/icons/hicolor"
##                                                    "/usr/share/icons/hicolor/16x16"
##                                                    "/usr/share/icons/hicolor/16x16/apps"
##                                                    "/usr/share/icons/hicolor/22x22"
##                                                    "/usr/share/icons/hicolor/22x22/apps"
##                                                    "/usr/share/icons/hicolor/32x32"
##                                                    "/usr/share/icons/hicolor/32x32/apps"
##                                                    "/usr/share/icons/hicolor/48x48"
##                                                   "/usr/share/icons/hicolor/48x48/apps"
##                                                    )

##
##
## SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
## SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is the former KPovModeler now PovModeler because stripped off the KDE stuff")
## SET(CPACK_PACKAGE_VENDOR "Simulated-Universe")
## SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
## SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
## SET(CPACK_PACKAGE_VERSION_MAJOR ${POVMODELER_VERSION_MAJOR})
## SET(CPACK_PACKAGE_VERSION_MINOR ${POVMODELER_VERSION_MINOR})
## set(CPACK_PACKAGE_VERSION_PATCH ${POVMODELER_VERSION_PATCH})
## SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
## SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
## SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${ID}-${VERSION_ID}")
## SET(CPACK_PACKAGE_CONTACT "Hans-Juergen Lange <hjl@simulated-universe.de>")
## SET(CPACK_PACKAGE_SECTION "graphics")
## INCLUDE(CPack)
## ENDIF (UNIX)
