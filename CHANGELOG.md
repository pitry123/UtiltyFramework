# Infrastructure change log  
Here we will try to put what is new in each build of the infrastructure to allow better understanding 

**Note:** if the latest number increased, this means we broke computability from preious version in one or more of the libraries

# update to 1.0.17
* XMLLoader is now using BinaryParser in order to allow dymanically loading XML configurations.
  **Note:** Using this new version means that you should add the binary_parser module as a dependency in your applications' CMake file.
  
# update to 1.0.16
* BinaryParser is now allowing several json serializer especially for enumerations: 
   - FULL -  enum object containing name and value (good for debuging)  
   - VALUES - Only the value of the enum  (default)
   - LABLES - Only the lable of the enum  
 uodate interface of ToJson and TryGetJson to allow this choices and update the samples 
* In RowData Add new function 'Parser' that get the parser which already parsed the RowData buffer
* API for Database dispatcher SubscribeTable -  it is possible to subscribe to a whole table and get events everytime a row of this table updates, the function get the event rows added after the subscription.
* Additing row history, developer can now query history of a row if row was initiated to support history
* Subscribe to table, it is now possible to subscribe to a whole table and get all the rows event into one callback. also if row was added later the table subscription will add/remove the row to the subscription
* Add Log to Error Manager - upone status update the sytem log is updated accordingly 

# update to 1.0.15
* Upgrade to Rules, it is now possible to create a Rule from any DB row and does not require any midiator DB. 
* DynamicApplication Sample, implement three rules and one classic rules 
along with it. Previous rules capability is maintained. 
* modules has ability to call utils::color_print for low level print (color filter will also apply on it)
* avoid call sync() on suspended threads (caused deadlock on exception thrown from suspended dispatchers on sync functions e.g constructor)
* add is_integer() function to utils/types to simply check if a string is an integer (using regex)

* Version was increased due to the need to get the offset from the binary parser which was not avaliable.
* Video improvements
  - Windows: Upgrading to gstreamer 1.18.0
  - Windows: RTSP source/player now supports nVidia GPU hardware accelerated H264 decoding through nvh264dec when applicable 
  - Windows: RTSP source/player now supports DX11 hardware accelerated H264 decoding through dx11h264dec when applicable
  - Windows: RTSP server now supports nVidia GPU hardware accelerated H264 encoding through nvh264enc when applicable 
  - nVidia ARM based (Jetson TX/TX2, Xavier (NX)): RTSP source/player now supports hardware accelerated H264 decoding 
    through omxh264dec when applicable
  - nVidia ARM based (Jetson TX/TX2, Xavier (NX)): RTSP server now supports hardware accelerated H264 encoding 
    through omxh264enc when applicable
  - VideoServer application can now be initialized bounded to a shared memory video source (e.g. VideoPublisher),
    allowing it to stream virtually any source
  - VideoServer application can now be initialized as UDP Multicast video server which is ideal for live sources
  - VideoServer application can now be initialized with a user defined gstreamer pipeline using the '-pipeline' command line argument

# update to 1.0.14
* Adding New mechanism to ezFramework - Error Management - please review this [wiki](for more information)
* Adding Dynamic Rows, developer can now create a empty table and add row dynamicly in the code (with metadata) and DebugEnvironment will get the rows
  and if deeded also the schema of the rows. currently not supporting enums that are not recognized by DE
* Adding API to rules manager to allow adding rules on run time (not only by constructor)
* Core.hpp has new API to generate GUID
* Add validate option (FromJson) to object upon calling FromJson to a json object (default for row is to validate, default for parser is not to validate). Add TryGetJson for better undestanding what went wrong if JSON failed to parse
if TryGetJson retrun false, the JSON string will still exist, but problematic fields will be marked with <<ERROR:error description>>
* bug fixes
* fix VxWorks 7 and CLang compilation issues
* Adding SubGet function to the modernAPI to allow reading the data before subscribing
* Make sure that all invokation on db dispatcher are async that will insure that all invokation are preformed in the same order.

# update to 1.0.13
* Adding udp_datagram_protocol for receiving UDP datagram messages.  
* DE Monitor and MonitorClient has now option to connect using udp_datagram_protocol (when starting with legacyMode flag enabled).  
  This change allows us to connect to older units which do not fully comply with the DE variable length protocol.  
* Version incremented to 1.0.13 due to API break in Database::Monitor.  
  We removed some default parameters that you had to be VERY brave to set.  
  It is very unlikely that it actually broke anything to anyone but just for sake…  
* We consider adapting ‘legacyMode=true’ to be the default as it might slightly improve performance by saving some CPU cycles and IO operations performed by the current VariableLengthProtocols.  
  This might be changed in a future update.  
* Impove binary parser to work better with arrays, now it is relies only on the array offset, this cause a dynamic allocation when reading a complex array at index (pay attention)  
* fix few issues related to to_json/from_json in related to arrays  
* in order to improve performance, byte arrays that are larger than 2048 will be considered as buffers and as an array, this also need to be fixed in DebugEnvironment (which currently fails to treat large arrays)  
* In to_json - buffers larger than 5000 bytes will be trancated and reduced in the json to 5000 bytes. do not use from_json with  buffers larger than 5000  
* Add Reset function to Row, Table, Dataset, calling it (assuming a parser metadata exist) will reset the row/table/dataset to its default value.  
* Schma Load - can now avoid reset the rows if setDefaultOnLoad is set to false (default true)  
* Upon creating a parser from CreateParser function (of BinaryParserMetadata) it is now does not reset the parser (need to call Reset exlicitly)
* binary_parser constructor has reset flag to indicate whther to reset the parser upon creation (default is false)
* fix bug in Schema::Load that failed to parse hex number for enums [Bug 62762](http://tfs1:8080/tfs/LandC4ICollection/Framework_2.0/_workitems?_a=edit&id=62762&triage=true)
* Add ToJson and FromJson to Row and ToJson to RowData
* Add support for expanding environment variables on path and make the function public on String.hpp for any use
* Add option to change default initialization of trace level enable for Debug Enviroment
* Imporve memory allocation on CreateParser to work on memory pool


# update to 1.0.12
* Add Support for Ubuntu 20.4  
* Utils::Context is now ABI Safe and can share between libraries. different dispatchers from different libraries can now share the same thread (better modularity)  
* Cmake LIB and BIN folder can now be defined by cmake define parameter (check for root cmameklist.txt)  
* Support for disable/enable the cli by using --cli(-c) switch with parameters, --cli true enable cli false disable. default is to allow the cli  
* Support new DebugEnvironemnt schema to improve load time and handle better with large arrays  

# Updates to 1.0.11

## BinaryParser   
* Add ability to automatically load Rules.xml on call load from command line terminal  
* Add Ability to call load from command line by using -s switch - call `BinaryParserCLI.exe -s <path to debug environment file>`  
* Add ability to load a regular debug environment file to binary parser cli and allow it to act like adummy application.  

