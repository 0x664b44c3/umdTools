# TSL Decoder / Dump tool
This tool allows you dumping TSL UMD protocol v3.x frames to stdout.

Useful for debuging applications using this de-facto industry standard-protocol.


## Example output
    $ ./tsl-monitor -p ttyUSB4 
    TSL-Monitor listening on ttyUSB4
    [2021-01-24 14:20:23.772] A004 B3 [0000]  Cam 5          
    [2021-01-24 14:20:23.772] A005 B3 [0000]  Cam 6          
    [2021-01-24 14:20:23.805] A006 B3 [0000]  Cam 7          
    [2021-01-24 14:20:24.004] A007 B3 [0000]  Cam 8          
    [2021-01-24 14:20:24.204] A000 B3 [0000]  Cam 1          
    [2021-01-24 14:20:24.405] A001 B3 [0000]  Cam 2          
    [2021-01-24 14:20:24.604] A002 B3 [0000]  Cam 3          
    [2021-01-24 14:20:24.805] A003 B3 [0000]  Cam 4          
    [2021-01-24 14:20:25.004] A004 B3 [0000]  Cam 5          
    [2021-01-24 14:20:25.204] A005 B3 [0000]  Cam 6          
    [2021-01-24 14:20:25.405] A006 B3 [0000]  Cam 7          
    [2021-01-24 14:20:25.604] A007 B3 [0000]  Cam 8          
    [2021-01-24 14:20:25.805] A000 B3 [0000]  Cam 1          
    [2021-01-24 14:20:26.005] A001 B3 [0000]  Cam 2          
    [2021-01-24 14:20:26.204] A002 B3 [0000]  Cam 3          
    [2021-01-24 14:20:26.404] A003 B3 [0000]  Cam 4          
    [2021-01-24 14:20:26.604] A004 B3 [0000]  Cam 5          
    [2021-01-24 14:20:26.804] A005 B3 [0000]  Cam 6          
    [2021-01-24 14:20:27.005] A006 B3 [0000]  Cam 7          
    [2021-01-24 14:20:27.204] A007 B3 [0000]  Cam 8
    [2021-01-24 14:20:27.404] A000 B3 [0000]  Cam 1 
