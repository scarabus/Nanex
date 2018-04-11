# NANEX

## Header files and binaries downloaded from NANEX
http://www.nanex.net/downloads/NxCoreAPI.h
http://www.nanex.net/downloads/NxCoreAPI.dll
http://www.nanex.net/downloads/NxCoreAPI64.dll
http://www.nanex.net/downloads/libnx.so.3.1.130.tar.gz

## Documentation 

The NxCore API is available at http://nxcoreapi.com/doc/
Username:  nanexuser
Password:  access2017

## Projects

vwap.c 
Basic VWAP calculator. Takes in all the trades for the day.
Outputs a CSV with just symbol and VWAP value, which it uploads to both Storage and BigQuery.
Note that it doesn't create the Crux file or table resources yet.
Takes a Storage bucket and BigQuery dataset as arguments or env variables.
 


