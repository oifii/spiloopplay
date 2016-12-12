FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "D:\Program Files\Native Instruments\Sample Libraries\Kontakt 3 Library\World\Z - Samples\Tonbak Tar Davul Kit Samples" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
