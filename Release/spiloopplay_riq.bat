FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "riq-kit_RIQ_DUN_OPN_20.wav" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
