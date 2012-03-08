@echo off

setlocal
set FLATZINC_CMD=fz
minizinc -I "%GECODEDIR%\mznlib" %*
