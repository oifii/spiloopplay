FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "rock-kit_HiHat_STBLMO_V01.wav" 1800 0.5
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
