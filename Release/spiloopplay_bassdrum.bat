FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "rolling-ice-kit_bd_rauschen1(stereo).wav" 1800 0.5
  rem start spiloopplay "rolling-ice-kit_bd_rauschen1(stereo).wav" 1800 5.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
