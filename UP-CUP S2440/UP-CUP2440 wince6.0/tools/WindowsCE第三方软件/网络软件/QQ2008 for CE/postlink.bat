xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\*.png %_FLATRELEASEDIR%
xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\qq.lnk %_FLATRELEASEDIR%
xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\qq.exe %_FLATRELEASEDIR%

xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\*.png "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"
xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\qq.llk "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"
xcopy /q /y %_WINCEROOT%\3rdparty\Tencent\files\qq.exe "%_PROJECTROOT%\cesysgen\oak\target\%_TGTCPU%\%WINCEDEBUG%"
