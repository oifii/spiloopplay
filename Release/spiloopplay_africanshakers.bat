FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "african-shakers-kit_DAB_SHK_03.wav" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
