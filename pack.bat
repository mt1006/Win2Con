:: Packs files to zips and generates checksums

@echo off

md packed
del win2con_win_x86_32.zip
del win2con_win_x86_64.zip 

cd Release
7z a -tzip win2con_win_x86_32.zip win2con.exe -ir!*.dll ..\LICENSE.txt
cd ..
move Release\win2con_win_x86_32.zip packed

cd x64\Release
7z a -tzip win2con_win_x86_64.zip win2con.exe -ir!*.dll ..\..\LICENSE.txt
cd ..\..
move x64\Release\win2con_win_x86_64.zip packed

ch -g "packed\win2con_win_x86_32.zip" "packed\win2con_win_x86_64.zip" > packed\win2con_win_sha256.txt

pause