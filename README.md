# org-simple
Simple utility library in modern standard C++ with a minimum of STL

| Symbol | Description
| ---- | ---- |
| *INPUTS* | is_complex of channel_buffer_samples channels
| *OUTPUTS* | is_complex of output channels
| *BS* | Block Size: Samples per block to process
| *PC* | Processing channels or frame-capacity
| *PS* | Processing samples = *PC* * *BS* 

```mermaid
graph LR
subgraph "Input translation"
IC0["Channel buffer (BS)"]
labelI("0 &hellip; (INPUTS - 1)")
ICN["Channel buffer (BS)"]
MMI("Input Mixer Matrix (INPUTS x PC)")
IC0 --> MMI
labelI-.-MMI
ICN --> MMI
linkStyle 1 stroke-width:0
end
subgraph "Output translation"
OC0["Channel buffer (BS)"]
labelO("0 &hellip; (OUTPUTS - 1)")
OCN["Channel buffer (BS)"]
MMO("Output Mixer Matrix (PC x OUTPUTS)")
MMO-->OC0
MMO-.-labelO
MMO-->OCN
linkStyle 4 stroke-width:0
end
subgraph "Processing"
PBI["Process channel_buffer_samples buffer (Interleaved PS)"]
PBO["Process output buffer (Interleaved PS)"]
PBI-->PROC(Processing)-->PBO
end

MMI-->PBI
PBO-->MMO
style labelI stroke-width:0,fill:none,text-align:left  
style labelO stroke-width:0,fill:none  
```
