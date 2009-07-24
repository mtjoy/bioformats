//
// showinf.cpp
//

/*
OME Bio-Formats C++ bindings for native access to Bio-Formats Java library.
Copyright (C) 2008-@year@ UW-Madison LOCI and Glencoe Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// A partial C++ version of the Bio-Formats showinf command line utility.
// For the original Java version, see:
//   components/bio-formats/src/loci/formats/tools/ImageInfo.java

#include "bio-formats.h"
#include "loci-common.h"

#include <string>
using std::string;

#include <exception>
using std::exception;

#include <iostream>
using std::cout;
using std::endl;

#include <limits.h>

#if defined (_WIN32)
#define PATHSEP string(";")
#else
#define PATHSEP string(":")
#endif

#define tf(x) (x ? "true" : "false")

// -- Fields --

String* id = NULL;
bool printVersion = false;
bool pixels = true;
bool doCore = true;
bool doMeta = true;
bool filter = true;
bool thumbs = false;
bool minmax = false;
bool merge = false;
bool stitch = false;
bool separate = false;
bool expand = false;
bool omexml = false;
bool normalize = false;
bool preload = false;
String* omexmlVersion = NULL;
int start = 0;
int end = INT_MAX;
int series = 0;
int xCoordinate = 0, yCoordinate = 0, width = 0, height = 0;
String* swapOrder = NULL;
String* shuffleOrder = NULL;

IFormatReader* reader = NULL;
ImageReader* imageReader = NULL;
FileStitcher* fileStitcher = NULL;
ChannelFiller* channelFiller = NULL;
ChannelSeparator* channelSeparator = NULL;
ChannelMerger* channelMerger = NULL;
MinMaxCalculator* minMaxCalc = NULL;
DimensionSwapper* dimSwapper = NULL;

StatusEchoer* status = NULL;

//String* seriesLabel = NULL;

//Double[] preGlobalMin, preGlobalMax;
//Double[] preKnownMin, preKnownMax;
//Double[] prePlaneMin, prePlaneMax;
//bool preIsMinMaxPop = false;

// -- Methods --

/* Initializes the Java virtual machine. */
void createJVM() {
  cout << "Creating JVM... ";
  StaticVmLoader loader(JNI_VERSION_1_4);
  OptionList list;
  list.push_back(jace::ClassPath(
    "jace-runtime.jar" + PATHSEP +
    "bio-formats.jar" + PATHSEP +
    "loci_tools.jar"));
  list.push_back(jace::CustomOption("-Xcheck:jni"));
  list.push_back(jace::CustomOption("-Xmx256m"));
  //list.push_back(jace::CustomOption("-verbose:jni"));
  jace::helper::createVm(loader, list, false);
  cout << "JVM created." << endl;
}

void parseArgs(int argc, const char *argv[]) {
  for (int i=1; i<argc; i++) {
    string arg = argv[i];
    if (arg.substr(0, 1).compare("-") == 0) {
      if (arg.compare("-nopix") == 0) pixels = false;
      else if (arg.compare("-version") == 0) printVersion = true;
      else if (arg.compare("-nocore") == 0) doCore = false;
      else if (arg.compare("-nometa") == 0) doMeta = false;
      else if (arg.compare("-nofilter") == 0) filter = false;
      else if (arg.compare("-thumbs") == 0) thumbs = true;
      else if (arg.compare("-minmax") == 0) minmax = true;
      else if (arg.compare("-merge") == 0) merge = true;
      else if (arg.compare("-stitch") == 0) stitch = true;
      else if (arg.compare("-separate") == 0) separate = true;
      else if (arg.compare("-expand") == 0) expand = true;
      else if (arg.compare("-omexml") == 0) omexml = true;
      else if (arg.compare("-normalize") == 0) normalize = true;
      else if (arg.compare("-preload") == 0) preload = true;
      else if (arg.compare("-xmlversion") == 0) {
        omexmlVersion = new String(argv[++i]);
      }
      //else if (arg.compare("-crop") == 0) {
      //  StringTokenizer st = new StringTokenizer(argv[++i], ",");
      //  xCoordinate = Integer.parseInt(st.nextToken());
      //  yCoordinate = Integer.parseInt(st.nextToken());
      //  width = Integer.parseInt(st.nextToken());
      //  height = Integer.parseInt(st.nextToken());
      //}
      else if (arg.compare("-range") == 0) {
        start = atoi(argv[++i]);
        end = atoi(argv[++i]);
      }
      else if (arg.compare("-series") == 0) {
        series = atoi(argv[++i]);
      }
      else if (arg.compare("-swap") == 0) {
        swapOrder = new String(argv[++i]);
      }
      else if (arg.compare("-shuffle") == 0) {
        shuffleOrder = new String(argv[++i]);
      }
      else cout << "Ignoring unknown command flag: " << arg << endl;
    }
    else {
      if (!id) id = new String(arg);
      else cout << "Ignoring unknown argument: " << arg << endl;
    }
  }
}

void printUsage() {
  cout << "To test read a file in any format, run:" << endl <<
    "  showinf file [-nopix] [-nocore] [-nometa] [-thumbs] [-minmax] " << endl <<
    "    [-merge] [-stitch] [-separate] [-expand] [-omexml]" << endl <<
    "    [-normalize] [-range start end] [-series num]" << endl <<
    "    [-swap inputOrder] [-shuffle outputOrder] [-preload]" << endl <<
    "    [-xmlversion v] [-crop x,y,w,h]" << endl <<
    "" << endl <<
    "  -version: print the library version and exit" << endl <<
    "      file: the image file to read" << endl <<
    "    -nopix: read metadata only, not pixels" << endl <<
    "   -nocore: do not output core metadata" << endl <<
    "   -nometa: do not parse format-specific metadata table" << endl <<
    " -nofilter: do not filter metadata fields" << endl <<
    "   -thumbs: read thumbnails instead of normal pixels" << endl <<
    "   -minmax: compute min/max statistics" << endl <<
    "    -merge: combine separate channels into RGB image" << endl <<
    "   -stitch: stitch files with similar names" << endl <<
    " -separate: split RGB image into separate channels" << endl <<
    "   -expand: expand indexed color to RGB" << endl <<
    "   -omexml: populate OME-XML metadata" << endl <<
    "-normalize: normalize floating point images*" << endl <<
    "    -range: specify range of planes to read (inclusive)" << endl <<
    "   -series: specify which image series to read" << endl <<
    "     -swap: override the default input dimension order" << endl <<
    "  -shuffle: override the default output dimension order" << endl <<
    "  -preload: pre-read entire file into a buffer; significantly" << endl <<
    "            reduces the time required to read the images, but" << endl <<
    "            requires more memory" << endl <<
    "  -xmlversion: specify which OME-XML version to generate" << endl <<
  //  "     -crop: crop images before displaying; argument is 'x,y,w,h'" << endl <<
    "" << endl <<
    "* = may result in loss of precision" << endl <<
    "" << endl;
}

void configureReaderPreInit() {
  if (omexml) {
    reader->setOriginalMetadataPopulated(true);
    MetadataStore store = MetadataTools::createOMEXMLMetadata();
    if (!store.isNull()) reader->setMetadataStore(store);
  }

  // determine format
  cout << "Checking file format ";
  cout << "[" << imageReader->getFormat(*id) << "]" << endl;

  cout << "Initializing reader" << endl;
  if (stitch) {
    reader = fileStitcher = new FileStitcher(*reader, true);
    String pat = FilePattern::findPattern(*id);
    if (!pat.isNull()) id = new String(pat);
  }
  if (expand) reader = channelFiller = new ChannelFiller(*reader);
  if (separate) reader = channelSeparator = new ChannelSeparator(*reader);
  if (merge) reader = channelMerger = new ChannelMerger(*reader);
  if (minmax) reader = minMaxCalc = new MinMaxCalculator(*reader);
  if (swapOrder || shuffleOrder) {
    reader = dimSwapper = new DimensionSwapper(*reader);
  }

  status = new StatusEchoer;
  reader->addStatusListener(*status);

  ((IFormatHandler*) reader)->close();
  reader->setNormalized(normalize);
  reader->setMetadataFiltered(filter);
  reader->setMetadataCollected(doMeta);
}

void configureReaderPostInit() {
  if (swapOrder) dimSwapper->swapDimensions(*swapOrder);
  if (shuffleOrder) dimSwapper->setOutputOrder(*shuffleOrder);
}

void checkWarnings() {
  int pixelType = reader->getPixelType();
  if (!normalize && (pixelType == FormatTools::FLOAT() ||
    pixelType == FormatTools::DOUBLE()))
  {
    cout << "Warning: Java does not support "
      "display of unnormalized floating point data." << endl;
    cout << "Please use the '-normalize' option "
      "to avoid receiving a cryptic exception." << endl;
  }

  if (reader->isRGB() && reader->getRGBChannelCount() > 4) {
    cout << "Warning: Java does not support "
      "merging more than 4 channels." << endl;
    cout << "Please use the '-separate' option "
      "to avoid receiving a cryptic exception." << endl;
  }
}

/* Reads core metadata from the currently initialized reader. */
void readCoreMetadata() {
  if (!doCore) return; // skip core metadata printout

  // read basic metadata
  cout << endl;
  cout << "Reading core metadata" << endl;
  cout << "Filename = " << *id << endl;
  StringArray used = reader->getUsedFiles();
  int usedLength = used.isNull() ? -1 : (int) used.length();
  bool usedValid = usedLength > 0;
  if (usedValid) {
    for (int u=0; u<usedLength; u++) {
      if (used[u].isNull()) {
        usedValid = false;
        break;
      }
    }
  }
  if (!usedValid) {
    cout <<
      "************ Warning: invalid used files list ************" << endl;
  }
  if (used.isNull()) {
    cout << "Used files = null" << endl;
  }
  else if (usedLength == 0) {
    cout << "Used files = []" << endl;
  }
  else if (usedLength > 1) {
    cout << "Used files:" << endl;
    for (int u=0; u<usedLength; u++) cout << "\t" << used[u] << endl;
  }
  else if (id && !id->equals(used[0])) {
    cout << "Used files = [" << used[0] << "]" << endl;
  }
  int seriesCount = reader->getSeriesCount();
  cout << "Series count = " << seriesCount << endl;
  MetadataStore ms = reader->getMetadataStore();
  MetadataRetrieve mr = MetadataTools::asRetrieve(ms);
  for (int j=0; j<seriesCount; j++) {
    reader->setSeries(j);

    // read basic metadata for series #i
    int imageCount = reader->getImageCount();
    bool rgb = reader->isRGB();
    int sizeX = reader->getSizeX();
    int sizeY = reader->getSizeY();
    int sizeZ = reader->getSizeZ();
    int sizeC = reader->getSizeC();
    int sizeT = reader->getSizeT();
    int pixelType = reader->getPixelType();
    int effSizeC = reader->getEffectiveSizeC();
    int rgbChanCount = reader->getRGBChannelCount();
    bool indexed = reader->isIndexed();
    bool falseColor = reader->isFalseColor();
    ByteArray2D table8 = reader->get8BitLookupTable();
    ShortArray2D table16 = reader->get16BitLookupTable();
    IntArray cLengths = reader->getChannelDimLengths();
    StringArray cTypes = reader->getChannelDimTypes();
    int thumbSizeX = reader->getThumbSizeX();
    int thumbSizeY = reader->getThumbSizeY();
    bool little = reader->isLittleEndian();
    String dimOrder = reader->getDimensionOrder();
    bool orderCertain = reader->isOrderCertain();
    bool thumbnail = reader->isThumbnailSeries();
    bool interleaved = reader->isInterleaved();
    bool metadataComplete = reader->isMetadataComplete();

    // output basic metadata for series #i
    cout << "Series #" << j;
    if (j < mr.getImageCount()) {
      cout << " -- " << mr.getImageName(j);
    }
    cout << ":" << endl;
    cout << "\tImage count = " << imageCount << endl;
    cout << "\tRGB = " << tf(rgb) << " (" << rgbChanCount << ")";
    if (merge) cout << " (merged)";
    else if (separate) cout << " (separated)";
    cout << endl;
    if (rgb != (rgbChanCount != 1)) {
      cout << "\t************ Warning: RGB mismatch ************" << endl;
    }
    cout << "\tInterleaved = " << tf(interleaved) << endl;
    cout << "\tIndexed = " << tf(indexed) << " (" <<
      (falseColor ? "false" : "true") << " color";
    if (!table8.isNull()) {
      int numTables8 = table8.length();
      cout << ", 8-bit LUT: " << numTables8 << " x ";
      cout << "?" << endl;//TEMP
      //ByteArray firstTable8 = table8[0]; // FIXME
      //if (firstTable8.isNull()) cout << "null";
      //else cout << firstTable8.length();
    }
    if (!table16.isNull()) {
      int numTables16 = table16.length();
      cout << ", 16-bit LUT: " << numTables16 << " x ";
      cout << "?" << endl;//TEMP
      //ShortArray firstTable16 = table16[0]; // FIXME
      //if (firstTable16.isNull()) cout << "null";
      //else cout << firstTable16.length();
    }
    cout << ")" << endl;
    if (indexed && table8.isNull() && table16.isNull()) {
      cout << "\t************ Warning: no LUT ************" << endl;
    }
    if (!table8.isNull() && !table16.isNull()) {
      cout << "\t************ Warning: multiple LUTs ************" << endl;
    }
    cout << "\tWidth = " << sizeX << endl;
    cout << "\tHeight = " << sizeY << endl;
    cout << "\tSizeZ = " << sizeZ << endl;
    cout << "\tSizeT = " << sizeT << endl;
    cout << "\tSizeC = " << sizeC;
    if (sizeC != effSizeC) cout << " (effectively " << effSizeC << ")";
    int cProduct = 1;
    int numDims = cLengths.length();
    if (numDims == 1 && FormatTools::CHANNEL().equals(cTypes[0])) {
      cProduct = cLengths[0];
    }
    else {
      cout << " (";
      for (int i=0; i<numDims; i++) {
        if (i > 0) cout << " x ";
        cout << cLengths[i] << " " << cTypes[i];
        cProduct *= cLengths[i];
      }
      cout << ")";
    }
    cout << endl;
    if (numDims == 0 || cProduct != sizeC) {
      cout <<
        "\t************ Warning: C dimension mismatch ************" << endl;
    }
    if (imageCount != sizeZ * effSizeC * sizeT) {
      cout << "\t************ Warning: ZCT mismatch ************" << endl;
    }
    cout << "\tThumbnail size = " <<
      thumbSizeX << " x " << thumbSizeY << endl;
    cout << "\tEndianness = " <<
      (little ? "intel (little)" : "motorola (big)") << endl;
    cout << "\tDimension order = " << dimOrder <<
      (orderCertain ? " (certain)" : " (uncertain)") << endl;
    cout << "\tPixel type = " <<
      FormatTools::getPixelTypeString(pixelType) << endl;
    cout << "\tMetadata complete = " << tf(metadataComplete) << endl;
    cout << "\tThumbnail series = " << tf(thumbnail) << endl;
  }
}

void initPreMinMaxValues() {
  // TODO
}

void printMinMaxValues() {
  // TODO
}

void readPixels() {
  cout << endl;
  cout << "Reading ";
  if (reader->getSeriesCount() > 1) cout << "series #" << series;
  cout << " pixel data ";
  status->setVerbose(false);
  int num = reader->getImageCount();
  if (start < 0) start = 0;
  if (start >= num) start = num - 1;
  if (end < 0) end = 0;
  if (end >= num) end = num - 1;
  if (end < start) end = start;

  int sizeX = reader->getSizeX();
  int sizeY = reader->getSizeY();
  int sizeC = reader->getSizeC();

  if (width == 0) width = sizeX;
  if (height == 0) height = sizeY;

  int pixelType = reader->getPixelType();

  cout << "(" << start << "-" << end << ") ";
  for (int i=start; i<=end; i++) {
    status->setEchoNext(true);
    if (thumbs) reader->openThumbBytes(i);
    else reader->openBytes(i, xCoordinate, yCoordinate, width, height);
    cout << ".";
    flush(cout);
  }
  cout << " ";
  cout << "[done]" << endl;

  if (minmax) printMinMaxValues();

  cout << endl;
}

void printGlobalMetadata() {
  // TODO
}

void printOriginalMetadata() {
  // TODO
}

void printOMEXML() {
  cout << endl;
  MetadataStore ms = reader->getMetadataStore();
  String version = MetadataTools::getOMEXMLVersion(ms);
  if (!version.isNull()) cout << "Generating OME-XML" << endl;
  else cout << "Generating OME-XML (schema version " << version << ")" << endl;
  MetadataRetrieve mr = MetadataTools::asRetrieve(ms);
  if (!mr.isNull()) {
    String xml = MetadataTools::getOMEXML(mr);
    cout << XMLTools::indentXML(xml) << endl;
    MetadataTools::validateOMEXML(xml);
  }
  else {
    cout << "The metadata could not be converted to OME-XML." << endl;
    if (omexmlVersion) {
      cout << omexmlVersion <<
        " is probably not a legal schema version." << endl;
    }
    else {
      cout << "The OME-XML Java library is probably not available." << endl;
    }
  }
}

void destroyObjects() {
  delete id;
  id = NULL;
  delete omexmlVersion;
  omexmlVersion = NULL;
  delete swapOrder;
  swapOrder = NULL;
  delete shuffleOrder;
  shuffleOrder = NULL;

  delete imageReader;
  imageReader = NULL;
  delete fileStitcher;
  fileStitcher = NULL;
  delete channelFiller;
  channelFiller = NULL;
  delete channelSeparator;
  channelSeparator = NULL;
  delete channelMerger;
  channelMerger = NULL;
  delete minMaxCalc;
  minMaxCalc = NULL;
  delete dimSwapper;
  dimSwapper = NULL;

  delete status;
  status = NULL;
}

/* Displays information on the given file. */
bool testRead(int argc, const char *argv[]) {
  parseArgs(argc, argv);
  if (printVersion) {
    cout << "Version: " << FormatTools::VERSION() << endl;
    cout << "SVN revision: " << FormatTools::SVN_REVISION() << endl;
    cout << "Build date: " << FormatTools::DATE() << endl;
    return true;
  }

  if (!id) {
    printUsage();
    return false;
  }

  reader = imageReader = new ImageReader;

  configureReaderPreInit();

  reader->setId(*id);

  configureReaderPostInit();
  checkWarnings();
  readCoreMetadata();
  reader->setSeries(series);
  initPreMinMaxValues();

  // read pixels
  if (pixels) readPixels();

  // read format-specific metadata table
  if (doMeta) {
    printGlobalMetadata();
    printOriginalMetadata();
  }

  // output and validate OME-XML
  if (omexml) printOMEXML();

  destroyObjects();
}

int main(int argc, const char *argv[]) {
  try {
    createJVM();
    testRead(argc, argv);
  }
  catch (FormatException& fe) {
    fe.printStackTrace();
    return -2;
  }
  catch (IOException& ioe) {
    ioe.printStackTrace();
    return -3;
  }
  catch (JNIException& jniException) {
    cout << "An unexpected JNI error occurred. " << jniException.what() << endl;
    return -4;
  }
  catch (std::exception& e) {
    cout << "An unexpected C++ error occurred. " << e.what() << endl;
    return -5;
  }

}
