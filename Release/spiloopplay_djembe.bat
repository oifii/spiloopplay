FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "djembe-kit_DJK_BSS_04.wav" 1800 1.0
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
