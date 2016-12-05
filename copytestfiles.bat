rem Mount the disk image file as drive y. If you
rem want to use a different drive letter, change it 
rem in the line below and in the line at the end of the file.
imdisk -a -t file -f uodos.img -o rem -m y:

rem Copy the Testing Directory.
xcopy /e /v %~dp0\Testing y:Testing /i 
copy HiTest.txt y:HiTest.txt

rem
rem  Add commands here to copy test files to disk y
rem
imdisk -D -m y:
