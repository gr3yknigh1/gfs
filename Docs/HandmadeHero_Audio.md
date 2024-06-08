# Handmade Hero Audio

> This is notes about audio from Handmade hero streams.

## How we write to audio buffer?

```text

  Positive (+)
  
  |> Period begins (defined in Hz)
  |> Sample begins
  |
  |   |> Sample ends 
  |   |     
  |   |           |> Period ends
  |   |           |
  V   V           V       
  .___.___.       .___.___.       .___.___.
          |       |       |       |       |
          |       |       |       |       |
  ------------------------------------------------>
          |       |       |       |       |
          |___.___|       |___.___|       |___.___
          
  Negative (-)
  
  
  Audio ring buffer:
  
  How we calculate how much bytes we can write to buffer.
  
                byte to lock = (running sample index * bytes per sample) % audio buffer size
  
  (1)
  
          Don't touch. It's playing
          VVVVVVVVVVVVVVVV
  [+++++++--------++++++++--------]
   ^      ^
   |      |> play cursor
   |> write cursor
  
                byte to write = play cursor - byte to lock 
   
  (2)
  
   Don't touch. It's playing
   VVVVVVVVVVVVVVV  
  [+++++++--------++++++++--------]
   ^              ^
   |              |> write cursor
   |> play cursor
  
                byte to write = %TODO% 

```
