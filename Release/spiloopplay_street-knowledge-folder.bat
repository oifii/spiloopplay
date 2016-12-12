FOR /L %%A IN (1,1,20) DO (
  ECHO %%A
  start spiloopplay "D:\Program Files\Native Instruments\Sample Libraries\Kontakt 3 Library\Band\Z - Samples\7 - Drum Kit Samples\Street Knowledge Kit Samples" 1800 0.5
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)


