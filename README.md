# Slow std::vector WASM Demonstration

## Overview

Benchmark that compares two methods of creating an image buffer in terms of performance.
The most interesting results come when you run it via the WebAssembly target (build
instructions below)

## Results

Below are the results I saw on my machine. Notice the absurd destruction times on WASM
in both Chrome and Firefox when using `std::vector`, and how not-absurd they are when
running natively or when using raw `new/delete` calls against a `uint8_t*`.

NATIVE (x86 Release, MSVC):
```
---------- std::vector image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             0|             0
  512|        262144|             0|             1|             0
 1024|       1048576|             0|             5|             0
 2048|       4194304|             0|            23|             0
 4096|      16777216|             2|           168|             1


---------- std::string image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             0|             0
  512|        262144|             0|             1|             0
 1024|       1048576|             0|             5|             0
 2048|       4194304|             0|            23|             0
 4096|      16777216|             2|           167|             1


---------- raw image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             0|             0
  512|        262144|             0|             1|             0
 1024|       1048576|             0|             6|             0
 2048|       4194304|             0|            32|             0
 4096|      16777216|             0|           171|             1
```

WASM (Firefox 82, Windows)
```
---------- std::vector image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             3|             3|             4
  512|        262144|            13|            12|            12
 1024|       1048576|            52|            47|            50
 2048|       4194304|           199|           199|           203
 4096|      16777216|           803|           836|           804


---------- std::string image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             3|             0
  512|        262144|             0|            10|             0
 1024|       1048576|             1|            41|             0
 2048|       4194304|             1|           166|             0
 4096|      16777216|             3|           755|             0


---------- raw image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             4|             0
  512|        262144|             0|            10|             0
 1024|       1048576|             0|            40|             0
 2048|       4194304|             0|           167|             0
 4096|      16777216|             0|           758|             0
```

WASM (Chrome 86, Windows)
```
---------- std::vector image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             1|             1|             1
  512|        262144|             6|             6|             6
 1024|       1048576|            27|            25|            27
 2048|       4194304|           111|           105|           108
 4096|      16777216|           440|           404|           429


---------- std::string image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             1|             0
  512|        262144|             0|             5|             0
 1024|       1048576|             0|            20|             0
 2048|       4194304|             0|            83|             0
 4096|      16777216|             1|           332|             0


---------- raw image benchmarks ----------
 size|         bytes|      alloc ms|       fill ms|       dtor ms
  256|         65536|             0|             1|             0
  512|        262144|             0|             5|             0
 1024|       1048576|             0|            20|             0
 2048|       4194304|             0|            84|             0
 4096|      16777216|             0|           343|             0
```
## Building / Running

### Native

Ninja:
```
mkdir out
cd out
cmake ..
ninja vec_benchmark
vec_benchmark
```

Visual Studio
```
mkdir out
cd out
cmake ..
```
Open the Visual Studio solution file created in `out` and run the program

### WebAssembly

Download and install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)

Then, build the program:

```
mkdir ems_out
cd ems_out
emcmake cmake ..
emmake ninja vec_benchmark
```

(I use ninja - you might use make or something else, in which case you'll use `emmake make vec_benchmark` etc)

Then, start up a web server:

```
# If you have Python installed
python -m SimpleHTTPServer
# Or, if you have http-server NPM package:
http-server
```

And navigate to `http://localhost:8000/vec_benchmark.html` in your [favorite browser](https://www.mozilla.org/en-US/firefox/new/).

## Le Backstory

I've been playing with the idea of developing a C++ game that has a native build target
as well as a Web target (via WebAssembly compiled with Emscripten). I'm having remarkable
luck with it, but there are a few tricky things that pop up here and there.

One particularly alarming case was when I was working on initialization code - i.e., the
stuff that happens during the loading screen. I noticed that there were a few huge CPU
stutters, even when I was using worker threads (via WebWorkers) to do heavy CPU work off
the UI thread.

After a lot of hair pulling, I found that the delay was happening specifically during the
"Update" cycle when a high resolution image was loaded - specifically, a 2048x2048 image
containing the roughness map for a PBR material.

Tons and tons of benchmarking later, I found out that the problem was happening through a
long line of destructors that was triggered when a `std::function` was going out of scope
that contained a `std::shared_ptr` in its closure which... several layers deeper, contained
the large `std::vector<uint8_t>` which holds the raw pixel data for that roughness map
texture.

I replaced the `std::vector<uint8_t>` with a naked `uint8_t*` pointer type to determine if
the offender was the WASM implementation of `malloc/free` that was problematic, only to
discover that... it wasn't. For some reason, most of the time spent in my loading screens
were being spent in the `std::vector<uint8_t>` destructor of all places.