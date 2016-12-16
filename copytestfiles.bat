rem Mount the disk image file as drive y. If you
rem want to use a different drive letter, change it 
rem in the line below and in the line at the end of the file.
imdisk -a -t file -f uodos.img -o rem -m y:

rem Copy the Testing Directory.
xcopy /e /v %~dp0\Testing y:Testing /i 
copy HiTest.txt y:HiTest.txt
echo Hello, world1 >y:1file.txt
echo Hello, world2 >y:2file.txt
echo Hello, world3 >y:3file.txt
echo Hello, world4 >y:4file.txt
echo Hello, world5 >y:5file.txt
echo Hello, world6 >y:6file.txt
echo Hello, world7 >y:7file.txt
echo Hello, world8 >y:8file.txt
echo Hello, world9 >y:9file.txt
echo Hello, world10 >y:10file.txt
echo Hello, world11 >y:11file.txt
echo Hello, world12 >y:12file.txt
echo Hello, world13 >y:13file.txt
echo Hello, world14 >y:14file.txt

rem
rem  Add commands here to copy test files to disk y
rem
imdisk -D -m y:
