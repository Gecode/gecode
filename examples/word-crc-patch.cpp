/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <gecode/driver.hh>
#include <gecode/word.hh>

#include <array>
#include <cstdint>
#include <iomanip>
#include <vector>

using namespace Gecode;

namespace {

  /// What the example computes
  enum class Mode {
    Patch, ///< Patch the edited configuration record
    Check  ///< Compute the CRC-32 of "123456789"
  };

  /// Reflected CRC-32 polynomial, as used by zlib and Ethernet
  constexpr std::uint32_t crc32_polynomial = 0xedb88320U;
  /// Published CRC-32 of the ASCII string "123456789"
  constexpr std::uint32_t crc32_check_value = 0xcbf43926U;

  /// Original configuration record, as accepted by the bootloader
  const std::vector<std::uint8_t> original_image = {
    'G', 'C', 'F', 'G',                             //  0: magic
    0x02, 0x01, 0x30, 0x00,                         //  4: version 1.2, length 48
    's', 'e', 'n', 's', 'o', 'r', '-', '7',         //  8: device name
    0xee, 0xff, 0xc0, 0x00,                         // 16: serial number
    0x00, 0xc2, 0x01, 0x00,                         // 20: baud rate 115200
    0xfa, 0x00, 0x00, 0x00,                         // 24: sample period 250 ms
    0x00, 0x00, 0x00, 0x00,                         // 28: reserved (patch area)
    0x12, 0x03, 0xf4, 0xff, 0x00, 0x04, 0x9c, 0x00, // 32: calibration table
    0x31, 0x75, 0x00, 0x00, 0x08, 0x00, 0x01, 0x80
  };
  /// Offset of the little-endian baud rate field
  constexpr unsigned int baud_rate_offset = 20;
  /// Baud rate written into the edited record
  constexpr std::uint32_t edited_baud_rate = 921600;
  /// Offset of the four reserved bytes that the model may choose freely
  constexpr unsigned int patch_offset = 28;
  /// Number of patch bytes, one per byte of CRC state
  constexpr unsigned int patch_size = 4;
  /// Message whose CRC-32 is the published check value
  const std::vector<std::uint8_t> check_message = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9'
  };

  /// Compute the reflected CRC-32 of \a bytes with plain integer code
  std::uint32_t
  compute_crc32(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t state = 0xffffffffU;
    for (std::uint8_t byte : bytes) {
      state ^= byte;
      for (int i=0; i<8; i++)
        state = (state >> 1) ^ (((state & 1U) != 0U) ? crc32_polynomial : 0U);
    }
    return ~state;
  }

  /// Read the little-endian 32-bit value at \a offset of \a bytes
  std::uint32_t
  read_le32(const std::vector<std::uint8_t>& bytes, unsigned int offset) {
    std::uint32_t value = 0;
    for (unsigned int i=0; i<4; i++)
      value |= static_cast<std::uint32_t>(bytes[offset+i]) << (8*i);
    return value;
  }

  /// Return the record with the edited baud rate and the old patch bytes
  std::vector<std::uint8_t>
  make_edited_image(void) {
    std::vector<std::uint8_t> bytes = original_image;
    for (unsigned int i=0; i<4; i++)
      bytes[baud_rate_offset+i] =
        static_cast<std::uint8_t>(edited_baud_rate >> (8*i));
    return bytes;
  }

  /// Return the bytes the model checksums in \a mode
  std::vector<std::uint8_t>
  get_input(Mode mode) {
    return (mode == Mode::Check) ? check_message : make_edited_image();
  }

  /// Test whether byte \a i is chosen by the model in \a mode
  bool
  is_patch_byte(Mode mode, unsigned int i) {
    return (mode == Mode::Patch) &&
      (i >= patch_offset) && (i < patch_offset+patch_size);
  }

}

/** \brief Options for the CRC-32 patching example */
class WordCrcPatchOptions : public Options {
public:
  /// Initialize options
  explicit WordCrcPatchOptions(const char* name)
    : Options(name),
      _check("check","compute the CRC-32 of \"123456789\" instead of patching") {
    add(_check);
  }
  /// Return what to compute
  Mode get_mode(void) const { return _check.value() ? Mode::Check : Mode::Patch; }
private:
  /// Whether to compute the published check value
  Driver::BoolOption _check;
};

/**
 * \brief %Example: CRC-32 checksum patching
 *
 * Bootloaders and update tools often accept a firmware or configuration
 * image when its CRC-32 matches a stored value.  When one field of such an
 * image must change (a configuration value, a serial number, a fixed bug)
 * but the stored checksum cannot, four free patch bytes can be chosen so
 * that the edited image keeps its original CRC-32 and is still accepted.
 * The same technique shows why a CRC protects only against accidental
 * corruption: anyone can change an image without changing its checksum.
 *
 * The image is a 48-byte configuration record.  The edit changes the
 * little-endian baud rate at offset 20 from 115200 to 921600, and the
 * four reserved bytes at offset 28 are the patch area.  The model finds
 * patch bytes such that the CRC-32 of the edited record equals that of
 * the original.
 *
 * Each byte is an 8-bit word.  The model is the standard reflected
 * CRC-32 (polynomial 0xEDB88320, initial value and final xor 0xFFFFFFFF,
 * as used by zlib and Ethernet): each byte is xored into the low bits of
 * the 32-bit state, and each of its eight bits then shifts the state
 * right by one and xors in the polynomial when the bit shifted out is
 * one.  The printed patched CRC is recomputed by plain integer code from
 * the solution.  With \c -check the model instead computes the CRC-32 of
 * "123456789", whose published check value is 0xCBF43926.
 *
 * A CRC is affine over GF(2), and four patch bytes after a known prefix
 * and before a known suffix determine the 32-bit state uniquely, so there
 * is exactly one solution.  Propagating known bits forward from the
 * prefix and backward from the target checksum through the suffix fixes
 * the patch bytes without search: the solution is found at the root.
 *
 * \ingroup Example
 */
class WordCrcPatch : public Script {
public:
  /// Actual model
  explicit WordCrcPatch(const WordCrcPatchOptions& opt)
    : Script(opt), mode(opt.get_mode()),
      image(*this,static_cast<int>(get_input(mode).size()),8,0,0xffU),
      crc(*this,32) {
    const std::vector<std::uint8_t> input = get_input(mode);
    for (int i=0; i<image.size(); i++)
      if (!is_patch_byte(mode,i))
        dom(*this,image[i],input[i]);
    post_crc32();
    if (mode == Mode::Patch) {
      dom(*this,crc,compute_crc32(original_image));
      branch(*this,image.slice(patch_offset,1,patch_size),
             WORD_VAR_NONE(),WORD_VAL_LSB());
    }
  }
  /// Constructor for cloning \a s
  WordCrcPatch(WordCrcPatch& s) : Script(s), mode(s.mode) {
    image.update(*this,s.image);
    crc.update(*this,s.crc);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordCrcPatch(*this);
  }
  /// Print solution
  void print(std::ostream& os) const override {
    const std::vector<std::uint8_t> solution = get_image_values();
    os << std::hex << std::setfill('0');
    if (mode == Mode::Check) {
      os << "\tCRC-32(\"123456789\") = 0x" << std::setw(8) << crc.val()
         << ", independent check 0x" << std::setw(8) << compute_crc32(solution)
         << ", published 0x" << std::setw(8) << crc32_check_value;
    } else {
      const std::uint32_t original_crc = compute_crc32(original_image);
      const std::uint32_t patched_crc = compute_crc32(solution);
      os << "\toriginal CRC-32 = 0x" << std::setw(8) << original_crc << std::endl
         << std::dec
         << "\tedited field    = baud rate at offset " << baud_rate_offset << ": "
         << read_le32(original_image,baud_rate_offset) << " -> "
         << read_le32(solution,baud_rate_offset) << std::endl
         << "\tpatch bytes     = offset " << patch_offset << ":" << std::hex;
      for (unsigned int i=0; i<patch_size; i++)
        os << " 0x" << std::setw(2) << static_cast<unsigned int>(solution[patch_offset+i]);
      os << std::endl
         << "\tpatched CRC-32  = 0x" << std::setw(8) << patched_crc
         << ((patched_crc == original_crc) ? " (equals original)" : " (DIFFERS from original)");
    }
    os << std::dec << std::setfill(' ') << std::endl;
  }
private:
  /// What the model computes
  const Mode mode;
  /// Checksummed bytes
  WordVarArray image;
  /// CRC-32 of the image
  WordVar crc;
  /// Return the assigned image bytes
  std::vector<std::uint8_t> get_image_values(void) const {
    std::vector<std::uint8_t> bytes;
    for (int i=0; i<image.size(); i++)
      bytes.push_back(static_cast<std::uint8_t>(image[i].val()));
    return bytes;
  }
  /// Post that \a crc is the CRC-32 of \a image
  void post_crc32(void) {
    WordVar state(*this,32,0xffffffffU,0xffffffffU);
    for (int i=0; i<image.size(); i++)
      state = absorb_byte(state,image[i]);
    complement(*this,state,crc);
  }
  /// Return the CRC state after \a byte has been absorbed into \a state
  WordVar absorb_byte(WordVar state, WordVar byte) {
    WordVar wide(*this,32), next(*this,32);
    zero_extend(*this,byte,32,wide);
    rel(*this,state,WOT_XOR,wide,next);
    for (int i=0; i<8; i++)
      next = shift_out_bit(next);
    return next;
  }
  /// Return the CRC state after shifting one bit out of \a state
  WordVar shift_out_bit(WordVar state) {
    BoolVar low_bit(*this,0,1);
    channel(*this,state,0,low_bit);
    WordVar shifted(*this,32), feedback(*this,32), next(*this,32);
    WordVar zero(*this,32,0,0);
    logical_shift_right(*this,state,1,shifted);
    ite(*this,low_bit,crc32_polynomial,zero,feedback);
    rel(*this,shifted,WOT_XOR,feedback,next);
    return next;
  }
};

/** \brief Main-function
 *  \relates WordCrcPatch
 */
int
main(int argc, char* argv[]) {
  WordCrcPatchOptions opt("WordCrcPatch");
  opt.parse(argc,argv);
  Script::run<WordCrcPatch,DFS,WordCrcPatchOptions>(opt);
  return 0;
}

// STATISTICS: example-any
