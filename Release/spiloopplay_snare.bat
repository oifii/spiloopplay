FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "rock-kit_Snare_STBLMO_V01.wav" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
