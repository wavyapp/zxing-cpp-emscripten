// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>

#include <zxing/LuminanceSource.h>

#include <emscripten.h>

using namespace std;
using namespace zxing;
using namespace zxing::qrcode;


class ImageReaderSource : public zxing::LuminanceSource {
private:
  typedef LuminanceSource Super;

  const zxing::ArrayRef<char> image;

  char convertPixel(const char* pixel) const;

public:
  ImageReaderSource(zxing::ArrayRef<char> image, int width, int height);

  zxing::ArrayRef<char> getRow(int y, zxing::ArrayRef<char> row) const;
  zxing::ArrayRef<char> getMatrix() const;
};

ImageReaderSource::ImageReaderSource(ArrayRef<char> image_, int width, int height)
    : Super(width, height), image(image_) {}

zxing::ArrayRef<char> ImageReaderSource::getRow(int y, zxing::ArrayRef<char> row) const {
  const char* pixelRow = &image[0] + y * getWidth();
  if (!row) {
    row = zxing::ArrayRef<char>(getWidth());
  }
  for (int x = 0; x < getWidth(); x++) {
    row[x] = pixelRow[x];
  }
  return row;
}

zxing::ArrayRef<char> ImageReaderSource::getMatrix() const {
  return image;
  // zxing::ArrayRef<char> matrix(getWidth() * getHeight());
  // memcpy(&matrix[0], &image[0], getWidth() * getHeight());
  // return matrix;
}


vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<Reader> reader(new QRCodeReader);
  return vector<Ref<Result> >(1, reader->decode(image, hints));
}

extern "C" {

  static const char *imagePtr = NULL;
  static zxing::ArrayRef<char> image = NULL;
  static Ref<LuminanceSource> source;

  const char* qr_resize(int width, int height) {
    image = zxing::ArrayRef<char>(width*height);
    imagePtr = &image[0];
    source = Ref<LuminanceSource>(new ImageReaderSource(image, width, height));
    return imagePtr;
  }

  int qr_decode() {
    vector<Ref<Result> > results;
    int res = -1;

    Ref<Binarizer> binarizer;

    try {

      DecodeHints hints(DecodeHints::DEFAULT_HINT);

      binarizer = new GlobalHistogramBinarizer(source);
      Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

      results = decode(binary, hints);

      res = 0;

    } catch (const ReaderException& e) {
      // cell_result = "zxing::ReaderException: " + string(e.what());
      res = -2;
    } catch (const zxing::IllegalArgumentException& e) {
      // cell_result = "zxing::IllegalArgumentException: " + string(e.what());
      res = -3;
    } catch (const zxing::Exception& e) {
      // cell_result = "zxing::Exception: " + string(e.what());
      res = -4;
    } catch (const std::exception& e) {
      // cell_result = "std::exception: " + string(e.what());
      res = -5;
    }

    if (res == 0) {
      std::string result = results[0]->getText()->getText();
      EM_ASM_(
        {
          Module.qr_decode_callback($0);
        },
        result.c_str()
      );
    }

    return res;
  }

}
