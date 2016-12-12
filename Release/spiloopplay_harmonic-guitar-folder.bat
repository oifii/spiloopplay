FOR /L %%A IN (1,1,20) DO (
  ECHO %%A
  start spiloopplay "D:\Program Files\Native Instruments\Sample Libraries\Kontakt 3 Library\Band\Z - Samples\5 - Guitar Samples\Harmonic Guitar Samples" 1800 2.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)


