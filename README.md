# ZXing Emscripten build

Based on the [ZXing C++ Port](https://github.com/glassechidna/zxing-cpp) with CMakeLists.txt from [ZXing Emscripten](https://github.com/fta2012/zxing-emscripten)

To build:

  1. `cd build`
  2. `emconfigure cmake ..`
  3. `emmake make -j4`
  4. `cd ..`
  5. `serve -p 3000`
  6. `open http://localhost:3000/emscripten/test/test.html`

To use:

``` javascript
    <script type="text/javascript">
      var tick = function() {
        if (window.ZXing) {
          ZXing = ZXing();
          doSomeDetecting();
        } else {
          setTimeout(tick, 10);
        }
      };
      setTimeout(tick, 10);

      var doSomeDetecting = function() {

          var resultString;

          // JS callback to receive the result pointer from C++
          var decodeCallback = function(ptr, len, resultIndex, resultCount, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y) {
            // Convert the result C string into a JS string.
            var result = new Uint8Array(ZXing.HEAPU8.buffer, ptr, len);
            resultString = String.fromCharCode.apply(null, result);
            // Do something with the QR finder points.
            // drawPointsOnCanvas(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
          };
          var decodePtr = ZXing.Runtime.addFunction(decodeCallback);

          // Get a write pointer for the QR image data array.
          // The write pointer is a pointer to a width*height Uint8Array of grayscale values.
          var imageWritePtr = ZXing._resize(width, height);

          // Copy your image data to the QR image data array.
          for (var i=0, j=0; i<myGrayscaleImageData.length; i++, j++) {
            ZXing.HEAPU8[imageWritePtr + j] = myGrayscaleImageData[i];
          }

          // Detect a QRcode in the image.
          var err = ZXing._decode_qr(decodePtr);

          // Detect a barcode in the image.
          // err = ZXing._decode_any(decodePtr);

          // Detect multiple QRcodes in the image.
          // If there are multiple QRcodes detected, decodePtr is called with each.
          // err = ZXing._decode_qr_multi(decodePtr);

          // Detect multiple barcodes in the image.
          // If there are multiple barcodes detected, decodePtr is called with each.
          // err = ZXing._decode_multi(decodePtr);

          console.log("error code", err);
          console.log("result", resultString);

        }
      };
    </script>
    <script async src="zxing.js"></script>
```

To hack: 

  See `emscripten/zxing.js.cpp`. 

# ZXing C++ Port

[ZXing](https://github.com/zxing/zxing) is/was a Java library.

At some point a complete C++ port/rewrite was created and maintained in the official [ZXing](https://github.com/zxing/zxing) repo. However, at the time of writing the C++ port is no longer maintained and has been removed from the official ZXing repo.

This project was forked from the [last ZXing commit](https://github.com/zxing/zxing/commit/00f6340) to contain the C++ project, with the following exceptions

 * scons (Python) build system has been deleted.
 * Deleted black box tests, because they refer to a large test data in ZXing repo.
 * Added appropriate copyright/licensing details (based on those in the ZXing repo).
 * Updated README.md

Removal of build systems was done to minimise maintenance burden.

If tests and XCode projects (other than those produced automatically be CMake) are desired, then another repo should be created and this repo referenced as a submodule. 

# Building using CMake

CMake is a tool, that generates native makefiles and workspaces. It integrates well with a number of IDEs including Qt Creator and Visual Studio.

Usage with CLion or Qt Creator:

  1. Simply open `CMakeLists.txt` as a new project
  2. Additional command line arguments can be specified (see below)

Usage with Makefiles, Visual Studio, etc. (see `cmake --help` for a complete list of generators):

  1. `mkdir build`
  2. `cd` to `build`
  3. Unix: run `cmake -G "Unix Makefiles" ..`
  3. Windows: run `cmake -G "Visual Studio 10" ..`
  
You can switch between build modes by specifying:

  - `-DCMAKE_BUILD_TYPE=Debug` or
  - `-DCMAKE_BUILD_TYPE=Release`

# Development tips

To profile the code (very useful to optimize the code):

  1. Install Valgrind
  2. Run `valgrind --tool=callgrind build/zxing - path/to/test/data/*.jpg > report.html`
  3. Analyze output using KCachegrind
