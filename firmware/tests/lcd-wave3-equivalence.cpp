#include "fl/chipsets/led_timing.h"
#include "fl/channels/wave3.h"
#include <cstdio>
#include <cstring>
#include <random>
int main() {
 fl::ChipsetTiming timing{}; timing.T1=250; timing.T2=400; timing.T3=600;
 auto lut=fl::buildWave3ExpansionLUT(timing);
 std::mt19937 rng(47);
 for(int trial=0;trial<10000;++trial) {
  fl::u8 lanes[16]={}, expected[48], actual[48], packed[24], zero[3];
  for(int i=0;i<(trial%8)+1;++i) lanes[i]=rng();
  fl::wave3Transpose_16(lanes,lut,expected);
  fl::wave3Transpose_8(reinterpret_cast<const fl::u8 (&)[8]>(lanes),lut,packed);
  fl::wave3(0,lut,zero);
  for(int i=0;i<24;++i) { actual[2*i]=packed[i];actual[2*i+1]=(zero[i/8]&(0x80>>(i%8)))?255:0; }
  if(std::memcmp(expected,actual,48)) return 1;
 }
 std::puts("10000 random eight-lane vectors match the 16-lane encoder byte-for-byte");
}
