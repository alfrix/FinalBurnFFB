# Microsoft Developer Studio Project File - Name="burn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=burn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "burn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "burn.mak" CFG="burn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "burn - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "burn - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "burn - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Gr /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "burn - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Gr /MTd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "burn - Win32 Release"
# Name "burn - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Chip Emus"

# PROP Default_Filter ""
# Begin Group "Sek (68000)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sek\d68k.c
# End Source File
# Begin Source File

SOURCE=.\sek\d68k.h
# End Source File
# Begin Source File

SOURCE=.\sek\d68kconf.h
# End Source File
# Begin Source File

SOURCE=.\sek\dasm.cpp
# End Source File
# Begin Source File

SOURCE=.\sek\sek.cpp
# End Source File
# Begin Source File

SOURCE=.\sek\sek.h
# End Source File
# Begin Source File

SOURCE=.\sek\a68k.obj
# End Source File
# End Group
# Begin Group "Ym2151"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\driver.h
# End Source File
# Begin Source File

SOURCE=.\state.h
# End Source File
# Begin Source File

SOURCE=.\ym2151.c
# End Source File
# Begin Source File

SOURCE=.\ym2151.h
# End Source File
# End Group
# Begin Group "Zet (Z80)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\doze.h
# End Source File
# Begin Source File

SOURCE=.\zet.cpp
# End Source File
# Begin Source File

SOURCE=.\zet.h
# End Source File
# Begin Source File

SOURCE=.\doze.lib
# End Source File
# End Group
# End Group
# Begin Group "Tmnt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\d_tmnt.cpp
# End Source File
# Begin Source File

SOURCE=.\tmnt_inp.cpp
# End Source File
# Begin Source File

SOURCE=.\tmnt_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\tmnt_run.cpp
# End Source File
# Begin Source File

SOURCE=.\tmnt_til.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\burn.cpp
# End Source File
# Begin Source File

SOURCE=.\burn.h
# End Source File
# Begin Source File

SOURCE=.\burnint.h
# End Source File
# Begin Source File

SOURCE=.\load.cpp
# End Source File
# Begin Source File

SOURCE=.\stdrom.h
# End Source File
# End Group
# Begin Group "Capcom"

# PROP Default_Filter ""
# Begin Group "CPS Drivers"

# PROP Default_Filter ""
# Begin Group "CPS1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cps1_d\dc_1941.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_ffight.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_forgottn.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_ghouls.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_knights.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_megaman.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_mercs.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_msword.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_sf2.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_sf2t.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_sfzch.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_strider.cpp
# End Source File
# Begin Source File

SOURCE=.\cps1_d\dc_unsquad.cpp
# End Source File
# End Group
# Begin Group "CPS2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cps2_d\dc_19xx.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_armwar.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_avsp.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_batcir.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_csclub.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_cyb.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_ddsom.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_ddtod.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_input.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_msh.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_mvsc.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_qnd.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_sfa.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_sfa2.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_sfa3.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_sfz2aj.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_slam2.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_spf.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_ssf2.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_ssf2t.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_vamp.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_vsav.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_xm.cpp
# End Source File
# Begin Source File

SOURCE=.\cps2_d\dc_xmvsf.cpp
# End Source File
# End Group
# End Group
# Begin Group "CPS Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cps.cpp
# End Source File
# Begin Source File

SOURCE=.\cps.h
# End Source File
# Begin Source File

SOURCE=.\cps_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_eep.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_obj.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_rot.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_run.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_rw.cpp
# End Source File
# Begin Source File

SOURCE=.\cps_scr.cpp
# End Source File
# Begin Source File

SOURCE=.\cpsr.cpp
# End Source File
# Begin Source File

SOURCE=.\cpsrd.cpp
# End Source File
# End Group
# Begin Group "CPS Tiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cpst.cpp
# End Source File
# Begin Source File

SOURCE=.\ctv.cpp
# End Source File
# Begin Source File

SOURCE=.\ctv.h
# End Source File
# Begin Source File

SOURCE=.\ctv_do.h
# End Source File
# Begin Source File

SOURCE=.\ctv_make.cpp

!IF  "$(CFG)" == "burn - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=.\ctv_make.cpp
InputName=ctv_make

"ctv.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl $(InputName).cpp 
	$(InputName).exe > ctv.h 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "burn - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=.\ctv_make.cpp
InputName=ctv_make

"ctv.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl $(InputName).cpp 
	$(InputName).exe > ctv.h 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "CPS Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps.cpp
# End Source File
# Begin Source File

SOURCE=.\ps_a.cpp
# End Source File
# Begin Source File

SOURCE=.\ps_m.cpp
# End Source File
# Begin Source File

SOURCE=.\ps_z.cpp
# End Source File
# Begin Source File

SOURCE=.\qs.cpp
# End Source File
# Begin Source File

SOURCE=.\qs_c.cpp
# End Source File
# Begin Source File

SOURCE=.\qs_z.cpp
# End Source File
# End Group
# End Group
# Begin Group "Sega"

# PROP Default_Filter ""
# Begin Group "After Burner"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aft\aft.h
# End Source File
# Begin Source File

SOURCE=.\aft\aft_gr.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\aft_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\aft_run.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\aft_rw.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\aft_spr.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\d_aft1.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\d_aft2.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\d_lof.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\d_ra2.cpp
# End Source File
# Begin Source File

SOURCE=.\aft\d_thu.cpp
# End Source File
# End Group
# Begin Group "Out Run"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\out\d_outrun.cpp
# End Source File
# Begin Source File

SOURCE=.\out\out.h
# End Source File
# Begin Source File

SOURCE=.\out\out_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\out\out_gr.cpp
# End Source File
# Begin Source File

SOURCE=.\out\out_init.cpp
# End Source File
# Begin Source File

SOURCE=.\out\out_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\out\out_run.cpp
# End Source File
# End Group
# Begin Group "Galaxy Force"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gal\d_gal2.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\d_gloc.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\d_pow.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\d_rail.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\d_strk.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal.h
# End Source File
# Begin Source File

SOURCE=.\gal\gal_line.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal_run.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal_rw.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal_snd.cpp
# End Source File
# Begin Source File

SOURCE=.\gal\gal_spr.cpp
# End Source File
# End Group
# Begin Group "System 16"

# PROP Default_Filter ""
# Begin Group "Drivers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sys16\ds_altbeast.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\ds_goldnaxe.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\ds_shinobi.cpp
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sys16\sys_pcm.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_snd.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_z80.cpp
# End Source File
# End Group
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sys16\calc.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\calc.h
# End Source File
# Begin Source File

SOURCE=.\sys16\calcfn.h
# End Source File
# Begin Source File

SOURCE=.\sys16\sys.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys.h
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_mem.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_spr.cpp
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_spr.h
# End Source File
# Begin Source File

SOURCE=.\sys16\sys_tile.cpp
# End Source File
# End Group
# End Group
# End Group
# Begin Source File

SOURCE=..\unused\bbits.txt
# End Source File
# Begin Source File

SOURCE=..\burner\zzd.txt
# End Source File
# End Target
# End Project
