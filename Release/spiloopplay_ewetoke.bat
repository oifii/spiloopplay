FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "ewe-toke-kit_EBL_HI_OPN_09.wav" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)