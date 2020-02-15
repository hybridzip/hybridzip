#include <iostream>
#include <chrono>
#include <cstring>

#define HZRANS_USE_AVX 0

#include <hzip/bitio/bitio.h>
#include <hzip/core/blob/hzmthread.h>
#include <hzip/utils/boost_utils.h>
#include <hzip/core/blob/hzblobpack.h>
#include <hzip/core/models/vtrie.h>

#define FILENAME "/home/supercmmetry/Documents/dickens/dickens"
#define OFILENAME "/home/supercmmetry/Documents/dickens/dickens.hz"
#define O2FILENAME "/home/supercmmetry/Documents/dickens/dickens.orig.txt"


int main() {
    std::cout << "hzip-unit-test started ... " << std::endl;

//    hzboost::deleteFileIfExists(OFILENAME);
//    hzboost::deleteFileIfExists(O2FILENAME);
//
//    VTrieModel *vm = new VTrieModel();
//
////    auto callback = [vm](uint64_t byte, uint64_t *ptr) {
////        vm->update(byte);
////        auto dist = vm->getDist();
////        for(int i = 0; i < 0x100; i++) {
////            if (dist->child[i] == nullptr) {
////                ptr[i] = 1;
////            } else {
////                ptr[i] = dist->child[i]->count << 12;
////            }
////        }
////    };
//
//    auto callback = [](uint64_t byte, uint64_t *ptr) {
//        ptr[byte]++;
//    };
//
//
//    auto proc = HZUProcessor(1);
//    proc.setCallback(callback);
//    proc.setBufferSize(1048576);
//
//    auto ces = new hz_cross_encoder [1];
//
//    *ces = [](hzrans64_t *state, std::stack<uint32_t> *data) {};
//    proc.setCrossEncoders(ces);
//
//    auto extractors = new std::function<uint64_t(void)>;
//
//    auto temp_stream = new bitio::bitio_stream(FILENAME, bitio::READ, 1024);
//
//
//    *extractors = [temp_stream]() {
//        return temp_stream->read(0x8);
//    };
//
//    proc.setExtractors(extractors);
//
//    hzrblob_set set = proc.encode();
//
//    auto ostream = bitio::bitio_stream(OFILENAME, bitio::WRITE, 1024);
//    hzBlobPacker packer;
//    packer.pack(set);
//    packer.commit(ostream);
//    set.destroy();
//
//    ostream.close();
//    ostream = bitio::bitio_stream(OFILENAME, bitio::READ, 1024);
//
//    hzBlobUnpacker unpacker;
//    set = unpacker.unpack(&ostream);
//
//    ostream.close();
//
//
//    // reset VTrieModel.
//    *vm = VTrieModel();
//
//    auto vec = proc.decode(set);
//    set.destroy();
//
//    auto o2stream = bitio::bitio_stream(O2FILENAME, bitio::WRITE, 1024);
//    for(auto iter: vec) {
//        o2stream.write(iter, 0x8);
//    }
//
//    o2stream.flush();
//    o2stream.close();
    return 0;
}