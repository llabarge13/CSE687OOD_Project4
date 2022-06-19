# CSE687 Project 4: Map Reduce
June 18, 2022

Authors: Todd Hricik, Lyndsay LaBarge

## Map Reduce Overview
The Visual Studio Solution contains five seperate projects. We are using Visual Studio 2022.

The MapReduce project contains the implementation of the primary command line application.

The UnitTests project contains our Google Unit tests.

The Interfaces project contains out map and reduce interfaces, IMap and IReduce. These are template abstract classes. Concrete classes must include a map and reduce class 
method respectively.

The MapLibrary project contains our DLL library with our concrete Map class implementation. It follows the factory method pattern and contains a single function,
createMapper that returns a pointer to a concrete Map class object. The createMapper function takes single parameter, the output directory the object writes its results to.

The ReduceLibrary project contains our DLL library with our concrete Reduce class implementation. It follows the factory method pattern and contains a single function,
createReducer that returns a pointer to a concrete Reduce class object. The createReducer function takes single parameter, the output directory the object writes its results to.

**New in Project 4** The Sockets project contains the code from Dr.Fawcett's Comm folder (https://ecs.syr.edu/faculty/fawcett/handouts/CSE687-OnLine/Repository/Comm/). We swapped out his logger for boost's logger. This is the only thing we changed.

**New in Project 4** The Stubs project contains the code for the command line executable to start stubs/sockets that can run map and reduce processes. 

We used C++ 17 and the boost C++ library (version 1.79). We used the boost Filesystem as opposed to implementating our own file management class.

## Project 4: Sockets

For our Sockets, we used Dr. Fawcett's Code from his Comm folder (linked above). 
We used our Workflow class as the controller and made stubs in the Stubs project.
You must run `Stubs.exe` before running `MapReduce.exe`. See [Stubs](#Stubs) for how to run `Stubs.exe` and [MapReduce](#MapReduce) for how to run `MapReduce.exe`. 

Note there is a bug in Dr. Fawcett's code. We didn't know the best way to fix it so we left it. In his Sender class (`Sockets\Comm.cpp` line 82), he only creates a new client thread if the Sender is sending a message to a new endpoint. In the case that the endpoint is the same, he does not check to see if the old client handler still exists or is valid. If the client dies (e.g. if the controller shuts down), if you start a new client at the same endpoint and try to send a message to it, it still tries to use the old client handler which is not valid. As a side effect, if you try to run `MapReduce.exe` twice without shutting down `Stubs.exe`, the stubs will not be able to send messages back to the controller since they are trying to use a no longer valid thread. No error messages occur, but the messages never arrive. To avoid this, you should restart `Stubs.exe` each time you want to run  `MapReduce.exe`.

His code is also not the best for catching errors but we did our best.

### Message Formats
#### Map Request (to stub)
```
to:localhost:9092
output_directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp
from:localhost:8080
command:run_map
partitions:3
input_files:C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\All'sWellThatEndsWell.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\AsYouLIkeIte.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\Frankenstein.txt
```

#### Reduce Request (to stub)
```
to:localhost:9091
output_directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp
from:localhost:8080
command:run_reduce
partition:0
input_files:C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp\34380p0,C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp\35324p0
```

#### Heartbeat (to controller)
```
message:reduce thread: 13580
to:localhost:8080
from:localhost:9092
name:heartbeat
```

#### Operation success (to controller)
```
message:Reduce process 13580 complete.
to:localhost:8080
from:localhost:9092
name:success
```

#### Operation failed (to controller)
```
message:Error in map.
to:localhost:8080
from:localhost:9092
name:failure
```

## Build
All projects use a properties sheet. To build the projects, you will need to change the values of the user macros BoostRoot and BoostLib in MapReduce > PropertySheet.props, UnitTests > PropertySheet.props, Interfaces > PropertySheet.props, MapLibrary > PropertySheet.props, ReduceLibrary > PropertySheet.props, Stubs > PropertySheet.props, and Sockets > PropertySheet.props. By default they are set to `C:\Program Files\boost\boost_ 1_79_0` and `C:\Program Files\boost\boost_1_79_0\stage\lib`. To build, use either the Debug or Release configuration for x64 platforms.

## Usage
You must run `Stubs.exe` BEFORE running `MapReduce.exe`.

### Stubs
```
 .\Stubs.exe --help
Allowed options:
  --help                Show options
  --map-dll arg         Path to map DLL. Required.
  --reduce-dll arg      Path to reduce DLL. Required.
  --stubs arg           Endpoint(s) for map/reduce stub(s). Required.
```
#### Command
```
.\Stubs.exe --map-dll ".\MapLibrary.dll" --reduce-dll ".\ReduceLibrary.dll" --stubs localhost:9091 localhost:9092
```
#### Command Line Output 
```
[2022-06-18 19:23:30.029409] [0x00002c4c] [info]    Info in Executive constructor: Map DLL located.
[2022-06-18 19:23:30.290413] [0x00002c4c] [info]    Info in Executive constructor: Reduce DLL located.
[2022-06-18 19:23:30.351923] [0x00003344] [info]    Started stub at localhost:9092
[2022-06-18 19:23:30.351923] [0x00003344] [info]    localhost:9092 started running
[2022-06-18 19:23:30.352423] [0x00003344] [info]    localhost:9092 waiting for message.
[2022-06-18 19:23:30.418936] [0x00003b9c] [info]    Started stub at localhost:9091
[2022-06-18 19:23:30.418936] [0x00003b9c] [info]    localhost:9091 started running
[2022-06-18 19:23:30.419435] [0x00003b9c] [info]    localhost:9091 waiting for message.
[2022-06-18 19:25:57.921663] [0x00003b9c] [info]    localhost:9091received messsage.
[2022-06-18 19:25:57.921663] [0x00003b9c] [info]    localhost:9091received map request.
[2022-06-18 19:25:57.922663] [0x00003b9c] [info]    map - input files:C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\AliceInWonderland.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\AMidSummerNightsDream.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\Cymbeline.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\Love'sLabourLost.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\MuchAdoAboutNothing.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\PrideAndPrejudice.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheComedyOfErrors.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheMerryWivesOfWindsor.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheTwelthNight.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TroilusAndCressida .txt
[2022-06-18 19:25:57.922663] [0x00003b9c] [info]    map - output directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp
[2022-06-18 19:25:57.923164] [0x00003b9c] [info]    map - number of partitions:3
[2022-06-18 19:25:57.923663] [0x00003b9c] [info]    Started map operation.
[2022-06-18 19:25:57.924163] [0x00003b9c] [info]    localhost:9091 waiting for message.
[2022-06-18 19:25:57.924163] [0x000055f0] [info]    Running map process for AliceInWonderland.txt
[2022-06-18 19:25:57.925164] [0x00003344] [info]    localhost:9092received messsage.
[2022-06-18 19:25:57.925164] [0x00003344] [info]    localhost:9092received map request.
[2022-06-18 19:25:57.925164] [0x00003344] [info]    map - input files:C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\All'sWellThatEndsWell.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\AsYouLIkeIte.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\Frankenstein.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\MeasureForMeasure.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\PericlesPrinceOfTyre.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TamingOfTheShrew.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheMerchantOfVenice.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheTempest.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\TheTwoGentlemenOfVerona.txt,C:\Users\llaba\source\repos\CSEOOD687_Project4\MapReduce\books\Winter'sTale.txt
[2022-06-18 19:25:57.925663] [0x00003344] [info]    map - output directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\x64\Release\temp
[2022-06-18 19:25:57.925663] [0x00003344] [info]    map - number of partitions:3
[2022-06-18 19:25:57.926164] [0x00003344] [info]    Started map operation.
[2022-06-18 19:25:57.926164] [0x00003344] [info]    localhost:9092 waiting for message.
[2022-06-18 19:25:57.927164] [0x00008b18] [info]    Running map process for All'sWellThatEndsWell.txt
[2022-06-18 19:25:57.988674] [0x00008b18] [info]    Map process completed for All'sWellThatEndsWell.txt
[2022-06-18 19:25:57.989174] [0x00008b18] [info]    Running map process for MeasureForMeasure.txt
[2022-06-18 19:25:57.990675] [0x000055f0] [info]    Map process completed for AliceInWonderland.txt
[2022-06-18 19:25:57.990675] [0x000055f0] [info]    Running map process for Love'sLabourLost.txt
....
```

### MapReduce
```
.\MapReduce.exe --help
Allowed options:
  --help                Show options
  --version             Verison number
  --input arg           Input directory path. Required.
  --temp arg            Intermediate directory path. Required.
  --output arg          Output directory path. Required.
  --stubs arg           Endpoint(s) for map/reduce stub(s). Required.
  --port arg (=8080)    Port for workflow/controller process. Optional.
                        Defaults to 8080.
  --mappers arg (=2)    Number of map threads. Optional. Defaults to 2.
  --reducers arg (=3)   Number of reduce threads. Optional. Defaults to 3.
```
All command line requirements are required except for the configurable number of mappers and reducers. 

The input directory must contain UTF-8 encoded text files. If the input directory contains a file that does not meet the requirements, that file will be skipped during processing.

If the intermediate directory or the output directory does not already exist, the application will create the directories.

You can configure the number of map and reduce operations via the command line using the ```--mappers``` and ```--reducers``` arguments. Note that the number of map and reduce operations is capped at the number of input files e.g. if there are 20 input files, you cannot have more than 20 map or reduce operations. If you pass a larger number via the command line, only 20 operations will run.

#### Command
```
.\MapReduce.exe --input "..\..\MapReduce\books" --temp ".\temp" --output ".\output" --stubs localhost:9091 localhost:9092
```
#### Command Line Output 
```
[2022-06-18 19:25:57.908161] [0x000084b4] [info]    Started controller at localhost:8080
[2022-06-18 19:25:57.914162] [0x000084b4] [info]    Connected to endpoint at localhost:9091
[2022-06-18 19:25:57.914661] [0x000084b4] [info]    Connected to endpoint at localhost:9092
[2022-06-18 19:25:57.915162] [0x000084b4] [info]    targetDir member has been set in Workflow constructor.
[2022-06-18 19:25:57.915162] [0x000084b4] [warning] Warning in Workflow constructor: argv[2] is not a directory

[2022-06-18 19:25:57.915662] [0x000084b4] [info]    Info in Workflow constructor: Creating directory at .\temp now...
[2022-06-18 19:25:57.916163] [0x000084b4] [info]    Info in Workflow constructor: Directory for intermediate files created.
[2022-06-18 19:25:57.916163] [0x000084b4] [info]    Info in Workflow constructor: outDir member has been set in Workflow constructor.
[2022-06-18 19:25:57.920163] [0x000084b4] [info]    Sent map request to localhost:9091
[2022-06-18 19:25:57.923164] [0x000084b4] [info]    Sent map request to localhost:9092
[2022-06-18 19:25:57.936666] [0x000084b4] [info]    Map process heartbeat: map thread: 35608
[2022-06-18 19:25:57.936666] [0x000084b4] [info]    Map process heartbeat: map thread: 22000
[2022-06-18 19:25:59.940516] [0x000084b4] [info]    Map process completed: Map process 22000 complete.
[2022-06-18 19:25:59.941515] [0x000084b4] [info]    Map process completed: Map process 35608 complete.
[2022-06-18 19:25:59.955518] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35196
[2022-06-18 19:25:59.956018] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35092
[2022-06-18 19:25:59.956518] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 33892
[2022-06-18 19:26:01.982872] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35196
[2022-06-18 19:26:01.982872] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35092
[2022-06-18 19:26:01.983871] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 33892
[2022-06-18 19:26:03.995722] [0x000084b4] [info]    Reduce process completed: Reduce process 35196 complete.
[2022-06-18 19:26:03.997722] [0x000084b4] [info]    Reduce process completed: Reduce process 35092 complete.
[2022-06-18 19:26:04.010724] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 33892
[2022-06-18 19:26:06.025576] [0x000084b4] [info]    Reduce process completed: Reduce process 33892 complete.
[2022-06-18 19:26:08.071433] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35396
[2022-06-18 19:26:10.100787] [0x000084b4] [info]    Reduce process heartbeat: reduce thread: 35396
[2022-06-18 19:26:12.116140] [0x000084b4] [info]    Reduce process success: Reduce process 35396 complete.
[2022-06-18 19:26:12.116140] [0x000084b4] [info]    Writing success file...
[2022-06-18 19:26:12.118140] [0x000084b4] [info]    Map reduce process complete.
[2022-06-18 19:26:12.118140] [0x000084b4] [info]    Removing intermediate files...
```

### File Output
Assuming the application completed with no errors, the results of the map reduce operation are located in the specified output directory. It will output 2 files.

`reduce.txt` - file containing the map reduce output

`SUCCESS` - file indicating the operation completed successfully


## Unit Test Results
The unit tests are defined in UnitTests\tests.cpp.

```
[==========] Running 12 tests from 4 test cases.
[----------] Global test environment set-up.
[----------] 3 tests from mapTest
[ RUN      ] mapTest.checkOutputPath
".\temp\test.txt"
[       OK ] mapTest.checkOutputPath (1 ms)
[ RUN      ] mapTest.checkMapResult
[       OK ] mapTest.checkMapResult (2 ms)
[ RUN      ] mapTest.badOutputDirectory
[       OK ] mapTest.badOutputDirectory (2 ms)
[----------] 3 tests from mapTest (6 ms total)

[----------] 3 tests from sortTest
[ RUN      ] sortTest.checkSortOutput
[       OK ] sortTest.checkSortOutput (0 ms)
[ RUN      ] sortTest.badInputFilePath
[       OK ] sortTest.badInputFilePath (0 ms)
[ RUN      ] sortTest.badInputFileFormat
[       OK ] sortTest.badInputFileFormat (1 ms)
[----------] 3 tests from sortTest (2 ms total)

[----------] 4 tests from reduceTest
[ RUN      ] reduceTest.checkOutputDirectory
[       OK ] reduceTest.checkOutputDirectory (6 ms)
[ RUN      ] reduceTest.checkOutputPath
[       OK ] reduceTest.checkOutputPath (0 ms)
[ RUN      ] reduceTest.checkBadDirectory
[       OK ] reduceTest.checkBadDirectory (0 ms)
[ RUN      ] reduceTest.checkReduceOutput
[       OK ] reduceTest.checkReduceOutput (7 ms)
[----------] 4 tests from reduceTest (15 ms total)

[----------] 2 tests from WorkflowTest
[ RUN      ] WorkflowTest.testConstructor
[2022-06-18 19:29:47.068264] [0x00008b44] [debug]   starting Receiver
[2022-06-18 19:29:47.068264] [0x00008b44] [debug]   starting ClientHandler
[2022-06-18 19:29:47.069264] [0x00008b44] [debug]   staring bind operation
[2022-06-18 19:29:47.069764] [0x00008b44] [debug]   netstat uport = 33571
[2022-06-18 19:29:47.072264] [0x00008b44] [debug]   server created ListenSocket
[2022-06-18 19:29:47.073764] [0x00008b44] [debug]   bind operation complete
[2022-06-18 19:29:47.073764] [0x00008b44] [debug]   starting TCP listening socket setup
[2022-06-18 19:29:47.138775] [0x00008b44] [debug]   server TCP listening socket setup complete
[2022-06-18 19:29:47.139775] [0x00008b44] [debug]   Debug in Workflow constructor: Entering constructor.
[2022-06-18 19:29:47.140276] [0x00008b44] [debug]   starting Receiver
[2022-06-18 19:29:47.140777] [0x00008b44] [debug]   starting ClientHandler
[2022-06-18 19:29:47.141776] [0x00008b44] [debug]   staring bind operation
[2022-06-18 19:29:47.141776] [0x00008b44] [debug]   netstat uport = 36895
[2022-06-18 19:29:47.143277] [0x00008b44] [debug]   server created ListenSocket
[2022-06-18 19:29:47.143776] [0x00008804] [debug]   server waiting for connection
[2022-06-18 19:29:47.144126] [0x00008b44] [debug]   bind operation complete
[2022-06-18 19:29:47.144276] [0x00008b44] [debug]   starting TCP listening socket setup
[2022-06-18 19:29:47.209288] [0x00008b44] [debug]   server TCP listening socket setup complete
[2022-06-18 19:29:47.210788] [0x00008b44] [info]    Started controller at localhost:8080
[2022-06-18 19:29:47.211288] [0x0000828c] [debug]   server waiting for connection
[2022-06-18 19:29:47.220290] [0x00008b44] [info]    Connected to endpoint at localhost:9091
[2022-06-18 19:29:47.220290] [0x00008804] [debug]   server accepted connection
[2022-06-18 19:29:47.222790] [0x00008b44] [info]    targetDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.223790] [0x00008b44] [info]    Info in Workflow constructor: intermediateDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.223790] [0x00008b44] [info]    Info in Workflow constructor: Directory for intermediate files created.
[2022-06-18 19:29:47.224292] [0x00008b44] [info]    Info in Workflow constructor: outDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.224791] [0x00008b44] [info]    Removing intermediate files...
[2022-06-18 19:29:47.225291] [0x00003f44] [debug]   send thread shutting down
[2022-06-18 19:29:47.225791] [0x00005fa8] [debug]   send thread shutting down
[2022-06-18 19:29:47.225791] [0x000039e4] [debug]   terminating ClientHandler thread
[2022-06-18 19:29:47.226291] [0x00008b44] [debug]   Socket System cleaning up

[2022-06-18 19:29:47.227791] [0x000039e4] [debug]   ClientHandler destroyed;
[2022-06-18 19:29:47.228291] [0x00008b44] [debug]   SocketListener instance destroyed
[2022-06-18 19:29:47.229292] [0x00008b44] [debug]   SocketConnecter instance destroyed
[2022-06-18 19:29:47.229292] [0x00008804] [debug]   server accept failed with error: 10004
[2022-06-18 19:29:47.230292] [0x0000828c] [debug]   server accept failed with error: 10004
[2022-06-18 19:29:47.230292] [0x00008804] [debug]   this occurs when application shuts down while listener thread is blocked on Accept call
[2022-06-18 19:29:47.230791] [0x00008b44] [debug]   Socket System cleaning up

[2022-06-18 19:29:47.231292] [0x0000828c] [debug]   this occurs when application shuts down while listener thread is blocked on Accept call
[2022-06-18 19:29:47.231292] [0x00008804] [debug]   Listen thread stopping
[2022-06-18 19:29:47.231792] [0x0000828c] [debug]   Listen thread stopping
[       OK ] WorkflowTest.testConstructor (164 ms)
[ RUN      ] WorkflowTest.testRun
[2022-06-18 19:29:47.252295] [0x00008b44] [debug]   starting Receiver
[2022-06-18 19:29:47.252295] [0x00008b44] [debug]   starting ClientHandler
[2022-06-18 19:29:47.252795] [0x00008b44] [debug]   staring bind operation
[2022-06-18 19:29:47.252795] [0x00008b44] [debug]   netstat uport = 33571
[2022-06-18 19:29:47.253295] [0x00008b44] [debug]   server created ListenSocket
[2022-06-18 19:29:47.253295] [0x00008b44] [debug]   bind operation complete
[2022-06-18 19:29:47.253795] [0x00008b44] [debug]   starting TCP listening socket setup
[2022-06-18 19:29:47.318807] [0x00008b44] [debug]   server TCP listening socket setup complete
[2022-06-18 19:29:47.321307] [0x00008b44] [info]    Started stub at localhost:9091
[2022-06-18 19:29:47.321808] [0x00006514] [debug]   server waiting for connection
[2022-06-18 19:29:47.322308] [0x00008b44] [debug]   Debug in Workflow constructor: Entering constructor.
[2022-06-18 19:29:47.325808] [0x00008b44] [debug]   starting Receiver
[2022-06-18 19:29:47.327809] [0x00006f04] [info]    localhost:9091 started running
[2022-06-18 19:29:47.328809] [0x00008b44] [debug]   starting ClientHandler
[2022-06-18 19:29:47.329308] [0x00006f04] [info]    localhost:9091 waiting for message.
[2022-06-18 19:29:47.329809] [0x00008b44] [debug]   staring bind operation
[2022-06-18 19:29:47.330309] [0x00006f04] [debug]   localhost:9091 deQing message
[2022-06-18 19:29:47.330309] [0x00008b44] [debug]   netstat uport = 36895
[2022-06-18 19:29:47.330809] [0x00008b44] [debug]   server created ListenSocket
[2022-06-18 19:29:47.331309] [0x00008b44] [debug]   bind operation complete
[2022-06-18 19:29:47.331309] [0x00008b44] [debug]   starting TCP listening socket setup
[2022-06-18 19:29:47.403322] [0x00008b44] [debug]   server TCP listening socket setup complete
[2022-06-18 19:29:47.405323] [0x00008b44] [info]    Started controller at localhost:8080
[2022-06-18 19:29:47.405822] [0x00008454] [debug]   server waiting for connection
[2022-06-18 19:29:47.408823] [0x00008b44] [info]    Connected to endpoint at localhost:9091
[2022-06-18 19:29:47.408823] [0x00006514] [debug]   server accepted connection
[2022-06-18 19:29:47.408823] [0x00008b44] [info]    targetDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.409824] [0x00008b44] [info]    Info in Workflow constructor: intermediateDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.409824] [0x00008b44] [info]    Info in Workflow constructor: Directory for intermediate files created.
[2022-06-18 19:29:47.410324] [0x00008b44] [info]    Info in Workflow constructor: outDir member has been set in Workflow constructor.
[2022-06-18 19:29:47.411324] [0x00008b44] [info]    Sent map request to localhost:9091
[2022-06-18 19:29:47.411324] [0x000070f8] [debug]   controller send thread sending
[2022-06-18 19:29:47.411825] [0x00008b44] [debug]   Message contents: output_directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\temp2
to:localhost:9091
from:localhost:8080
command:run_map
partitions:1
input_files:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\shakespeare\Cymbeline.txt


[2022-06-18 19:29:47.412324] [0x00006e10] [debug]   localhost:9091 RecvThread read message:
[2022-06-18 19:29:47.412824] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:47.412824] [0x00006f04] [info]    localhost:9091received messsage.
[2022-06-18 19:29:47.414324] [0x00006f04] [info]    localhost:9091received map request.
[2022-06-18 19:29:47.414823] [0x00006f04] [info]    map - input files:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\shakespeare\Cymbeline.txt
[2022-06-18 19:29:47.415324] [0x00006f04] [info]    map - output directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\temp2
[2022-06-18 19:29:47.415324] [0x00006f04] [info]    map - number of partitions:1
[2022-06-18 19:29:47.415824] [0x00006f04] [info]    Started map operation.
[2022-06-18 19:29:47.416324] [0x00006f04] [info]    localhost:9091 waiting for message.
[2022-06-18 19:29:47.417825] [0x00006f04] [debug]   localhost:9091 deQing message
[2022-06-18 19:29:47.419325] [0x00008afc] [info]    Running map process for Cymbeline.txt
[2022-06-18 19:29:47.428326] [0x00002e04] [debug]   localhost:9091 send thread sending heartbeat
[2022-06-18 19:29:47.428826] [0x00002e04] [debug]   attempting to connect to new endpoint: localhost:8080
[2022-06-18 19:29:47.430326] [0x00002e04] [debug]     connected to localhost:8080
[2022-06-18 19:29:47.430326] [0x00008454] [debug]   server accepted connection
[2022-06-18 19:29:47.431327] [0x000080a8] [debug]   controller RecvThread read message: heartbeat
[2022-06-18 19:29:47.431327] [0x00008b44] [debug]   Received message: message:map thread: 35580
to:localhost:8080
from:localhost:9091
name:heartbeat


[2022-06-18 19:29:47.431826] [0x00008b44] [info]    Map process heartbeat: map thread: 35580
[2022-06-18 19:29:47.432326] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:47.492837] [0x00008afc] [info]    Map process completed for Cymbeline.txt
[2022-06-18 19:29:49.434597] [0x00008afc] [info]    Map process 35580 complete.
[2022-06-18 19:29:49.435100] [0x00002e04] [debug]   localhost:9091 send thread sending success
[2022-06-18 19:29:49.436100] [0x000080a8] [debug]   controller RecvThread read message: success
[2022-06-18 19:29:49.436100] [0x00008b44] [debug]   Received message: message:Map process 35580 complete.
to:localhost:8080
from:localhost:9091
name:success


[2022-06-18 19:29:49.436600] [0x00008b44] [info]    Map process completed: Map process 35580 complete.
[2022-06-18 19:29:49.437601] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:49.437601] [0x000070f8] [debug]   controller send thread sending
[2022-06-18 19:29:49.438101] [0x00006e10] [debug]   localhost:9091 RecvThread read message:
[2022-06-18 19:29:49.438101] [0x00006f04] [info]    localhost:9091received messsage.
[2022-06-18 19:29:49.438600] [0x00006f04] [info]    localhost:9091received map request.
[2022-06-18 19:29:49.438600] [0x00006f04] [info]    reduce - input files:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\temp2\35580p0
[2022-06-18 19:29:49.439101] [0x00006f04] [info]    reduce - output directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\temp2
[2022-06-18 19:29:49.439601] [0x00006f04] [info]    reduce - partition:0
[2022-06-18 19:29:49.439601] [0x00006f04] [info]    Started reduce operation.
[2022-06-18 19:29:49.440101] [0x00006f04] [info]    localhost:9091 waiting for message.
[2022-06-18 19:29:49.440601] [0x00006f04] [debug]   localhost:9091 deQing message
[2022-06-18 19:29:49.441101] [0x00005b38] [info]    Running sort on 35580p0
[2022-06-18 19:29:49.450103] [0x00002e04] [debug]   localhost:9091 send thread sending heartbeat
[2022-06-18 19:29:49.451603] [0x000080a8] [debug]   controller RecvThread read message: heartbeat
[2022-06-18 19:29:49.451603] [0x00008b44] [info]    Reduce process heartbeat: reduce thread: 23352
[2022-06-18 19:29:49.451603] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:49.455604] [0x00005b38] [info]    Running reduce operation.
[2022-06-18 19:29:51.460826] [0x00005b38] [info]    Reduce process 23352 complete.
[2022-06-18 19:29:51.461329] [0x00002e04] [debug]   localhost:9091 send thread sending success
[2022-06-18 19:29:51.462328] [0x000080a8] [debug]   controller RecvThread read message: success
[2022-06-18 19:29:51.462328] [0x00008b44] [info]    Reduce process completed: Reduce process 23352 complete.
[2022-06-18 19:29:51.463329] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:51.463329] [0x000070f8] [debug]   controller send thread sending
[2022-06-18 19:29:51.463828] [0x00006e10] [debug]   localhost:9091 RecvThread read message:
[2022-06-18 19:29:51.464329] [0x00006f04] [info]    localhost:9091received messsage.
[2022-06-18 19:29:51.464329] [0x00006f04] [info]    localhost:9091received map request.
[2022-06-18 19:29:51.464329] [0x00006f04] [info]    reduce - input files:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\temp2\reduce0.txt
[2022-06-18 19:29:51.464829] [0x00006f04] [info]    reduce - output directory:C:\Users\llaba\source\repos\CSEOOD687_Project4\UnitTests\output
[2022-06-18 19:29:51.465329] [0x00006f04] [info]    reduce - partition:0
[2022-06-18 19:29:51.465329] [0x00006f04] [info]    Started reduce operation.
[2022-06-18 19:29:51.465829] [0x00006f04] [info]    localhost:9091 waiting for message.
[2022-06-18 19:29:51.466329] [0x00006f04] [debug]   localhost:9091 deQing message
[2022-06-18 19:29:51.466329] [0x000067f4] [info]    Running sort on reduce0.txt
[2022-06-18 19:29:51.469329] [0x000067f4] [info]    Running reduce operation.
[2022-06-18 19:29:51.476331] [0x00002e04] [debug]   localhost:9091 send thread sending heartbeat
[2022-06-18 19:29:51.477331] [0x000080a8] [debug]   controller RecvThread read message: heartbeat
[2022-06-18 19:29:51.477331] [0x00008b44] [debug]   controller deQing message
[2022-06-18 19:29:53.487348] [0x000067f4] [info]    Reduce process 26612 complete.
[2022-06-18 19:29:53.487348] [0x00002e04] [debug]   localhost:9091 send thread sending success
[2022-06-18 19:29:53.488349] [0x000080a8] [debug]   controller RecvThread read message: success
[2022-06-18 19:29:53.488349] [0x00008b44] [info]    Reduce process success: Reduce process 26612 complete.
[2022-06-18 19:29:53.488849] [0x00008b44] [info]    Writing success file...
[2022-06-18 19:29:53.490349] [0x00008b44] [info]    Map reduce process complete.
[2022-06-18 19:29:53.490849] [0x00002e04] [debug]   send thread shutting down
[2022-06-18 19:29:53.491348] [0x000080a8] [debug]   terminating ClientHandler thread
[2022-06-18 19:29:53.491848] [0x00008b44] [info]    Removing intermediate files...
[2022-06-18 19:29:53.492349] [0x000080a8] [debug]   ClientHandler destroyed;
[2022-06-18 19:29:53.499350] [0x000070f8] [debug]   send thread shutting down
[2022-06-18 19:29:53.499851] [0x00006e10] [debug]   terminating ClientHandler thread
[2022-06-18 19:29:53.500351] [0x00008b44] [debug]   Socket System cleaning up

[2022-06-18 19:29:53.500851] [0x00006e10] [debug]   ClientHandler destroyed;
[2022-06-18 19:29:53.501350] [0x00006514] [debug]   server accept failed with error: 10004
[2022-06-18 19:29:53.501350] [0x00008b44] [debug]   Socket System cleaning up

[2022-06-18 19:29:53.501851] [0x00008454] [debug]   server accept failed with error: 10004
[2022-06-18 19:29:53.501851] [0x00006514] [debug]   this occurs when application shuts down while listener thread is blocked on Accept call
[2022-06-18 19:29:53.503851] [0x00008454] [debug]   this occurs when application shuts down while listener thread is blocked on Accept call
[       OK ] [2022-06-18 19:29:53.503851] [0x00006514] [debug]   Listen thread stopping
WorkflowTest.testRun[2022-06-18 19:29:53.504351] [0x00008454] [debug]   Listen thread stopping
 (6269 ms)
[----------] 2 tests from WorkflowTest (6439 ms total)

[----------] Global test environment tear-down
[==========] 12 tests from 4 test cases ran. (6463 ms total)
[  PASSED  ] 12 tests.
```