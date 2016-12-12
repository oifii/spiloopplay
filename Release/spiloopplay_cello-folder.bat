FOR /L %%A IN (1,1,10) DO (
  ECHO %%A
  start spiloopplay "D:\oifii-org\httpdocs\ns-org\nsd\ar\cp\audio_spi\spimidisamplerwin32\03 Cello ensemble - 8(A-D-E)" 1800 0.5
  PING 1.1.1.1 -n 1 -w 4000 >NUL
)
