#pragma once
// Copyright 2019 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <array>
#include <istream>
#include <streambuf>
#include <string>

namespace util {

/// Input/Output streambuf using an existing fixed-sized char buffer as the
/// storage model
/**
 * To construct an `ostream` with a fixed buffer as character storage, use the
 * following pattern:
 * ```
 * std::array<char, 512> my_storage;
 * FixedStreamBuf stream_buf{my_storage.begin(), 512};
 * std::ostream ostrm{&stream_buf};
 * ostrm << "Hello world!";
 * ```
 *
 * At which point `my_storage` contains the null terminated string
 * "Hello world!".
 */
template <typename CharT, class Traits = std::char_traits<CharT>>
class FixedStreamBuf : public std::basic_streambuf<CharT, Traits> {
 public:
  FixedStreamBuf(CharT* storage_begin, size_t storage_size)
      : storage_begin_{storage_begin},
        storage_size_{storage_size},
        write_count_{0} {
    this->reset();
  }

  FixedStreamBuf(CharT* storage_begin, CharT* storage_end)
      : storage_begin_{storage_begin},
        storage_size_{static_cast<size_t>(storage_end - storage_begin)},
        write_count_{0} {
    this->reset();
  }

  template <size_t N>
  explicit FixedStreamBuf(CharT (&storage)[N])
      : storage_begin_{storage}, storage_size_{N}, write_count_{0} {
    this->reset();
  }

  template <size_t N>
  explicit FixedStreamBuf(std::array<CharT, N>* storage)
      : storage_begin_{storage->data()}, storage_size_{N}, write_count_{0} {
    this->reset();
  }

  ~FixedStreamBuf() override = default;

  void reset() {
    write_count_ = 0;
    this->setp(storage_begin_, storage_begin_ + storage_size_ - 1);
    this->setg(storage_begin_, storage_begin_, storage_begin_);
  }

  CharT* storage_begin() const {
    return storage_begin_;
  }

  size_t size() const {
    return write_count_;
  }

  // Write the null terminator at the current end of the stream. This should
  // be called before any consumer of the data tries to read the buffer as a
  // c-string.
  void write_null_terminator() {
    storage_begin_[write_count_] = 0;
  }

 protected:
  FixedStreamBuf<CharT, Traits>* setbuf(CharT*, std::streamsize) override {
    throw std::runtime_error("Illegal FixedStreamBuf::setbuf()");
  }

  std::streamsize xsputn(const CharT* s, std::streamsize count) override {
    using BaseT = std::basic_streambuf<CharT, Traits>;
    std::streamsize retval = this->BaseT::xsputn(s, count);

    // Update the end-pointer for the read stream, now that potentially new
    // content has been added to the storage model. Note that we need to
    // reserve one character for the null terminator. I think this should
    // happen automatically because we reserve it when we setp() above, but
    // it doesn't hurt to double check.
    if (write_count_ + count + 1 > storage_size_) {
      count = storage_size_ - write_count_ - 1;
    }
    write_count_ += count;

    // Writing out a null terminator after every write seems a bit wasteful,
    // but we might not own the storage buffer so we can't rely use a c_str()
    // mechanism to defer the terminator write.
    // TODO(josh): Actually we could possibly defer the null terminator write.
    // As far as our current usage, there are two cases:
    // 1. Wrap a fixed buffer with a FixedBufStream
    // 2. Store a fixed buffer in a FixedStringStream
    // For case 1. we can write the null terminator in either this destructor
    // or in the destructor for FixedBufStream (possibly needing a helper in
    // this class since the buffer is protected). For 2. we can use a c_str()
    // accessor write the null terminator before returning the data. Both
    // of these are implemented below but I'm keeping this here because
    // otherwise we're leaving a bit of a time-bomb and I'd like to come up
    // with a good strategy to mitigate that risk.
    storage_begin_[write_count_] = 0;

    // Update the readable segment.
    this->setg(storage_begin_, this->gptr(), storage_begin_ + write_count_);
    return retval;
  }

  CharT* storage_begin_;
  size_t storage_size_;
  size_t write_count_;
};

/// Provide storage for bufstreams
/**
 * This class only exists because FixedBufStream needs to construct it's
 * streambuf before using it to construct the ostream interface.
 * FixedBufStream inherits from this class first ensuring the proper order of
 * construction/destruction.
 */
template <typename CharT, class Traits = std::char_traits<CharT>>
class FixedBufStreamBase {
 protected:
  FixedBufStreamBase(CharT* storage_begin, size_t storage_size)
      : streambuf_{storage_begin, storage_size} {}

  FixedBufStreamBase(CharT* storage_begin, CharT* storage_end)
      : streambuf_{storage_begin, storage_end} {}

  template <size_t N>
  explicit FixedBufStreamBase(CharT (&storage)[N]) : streambuf_{storage} {}

  template <size_t N>
  explicit FixedBufStreamBase(std::array<CharT, N>* storage)
      : streambuf_{storage} {}

  virtual ~FixedBufStreamBase() {
    // Since we are releasing control of the buffer presumably someone else is
    // going to try to use it. We need to make sure we've written out the null
    // terminator before anyone tries to read it.
    streambuf_.write_null_terminator();
  }

 protected:
  FixedStreamBuf<CharT, Traits> streambuf_;
};

/// Provide an ostream interface around a fixed sized buffer
/**
 * For example:
 * ```
 * char strbuf[14];
 * FixedBufStream{strbuf} << "hello" << " world" << "!\n";
 * // `strbuf` contains "Hello world!\n"
 * ```
 */
template <typename CharT, class Traits = std::char_traits<CharT>>
class FixedBufStream : public FixedBufStreamBase<CharT, Traits>,
                       public std::basic_iostream<CharT, Traits> {
 public:
  typedef FixedBufStreamBase<CharT, Traits> Base1;
  typedef std::basic_iostream<CharT, Traits> Base2;

  FixedBufStream(const FixedBufStream&& other)
      : Base1(other.storage_begin(), other.storage_size()),
        Base2(&this->streambuf_) {}

  FixedBufStream(CharT* storage_begin, size_t storage_size)
      : Base1(storage_begin, storage_size), Base2(&this->streambuf_) {}

  FixedBufStream(CharT* storage_begin, CharT* storage_end)
      : Base1(storage_begin, storage_end), Base2(&this->streambuf_) {}

  template <size_t N>
  explicit FixedBufStream(CharT (&storage)[N])
      : Base1(storage), Base2(&this->streambuf_) {}

  template <size_t N>
  explicit FixedBufStream(std::array<CharT, N>* storage)
      : Base1(storage), Base2(&this->streambuf_) {}

  ~FixedBufStream() override = default;

  size_t size() {
    return this->streambuf_.size();
  }

  void reset() {
    this->streambuf_.reset();
    this->clear();
  }

 private:
  CharT* storage_begin() const {
    return this->streambuf_.storage_begin();
  }
  size_t storage_size() const {
    return this->streambuf_.size();
  }
};

using FixedCharStream = FixedBufStream<char>;

/// Provide storage for stringstreams
/**
 * This class only exists because FixedStringStream needs to construct it's
 * storage and streambuf before using them to construct the ostream interface.
 * FixedStringStream inherits from this class first ensuring the proper order of
 * construction/destruction.
 */
template <typename CharT, size_t buf_size_,
          class Traits = std::char_traits<CharT>>
class FixedStringStreamBase {
 protected:
  FixedStringStreamBase() : streambuf_(&storage_) {
    storage_[0] = '\0';
  }

  virtual ~FixedStringStreamBase() = default;

 public:
  /// Return a const reference to the underlying string storage.
  const std::array<CharT, buf_size_>& str() const {
    return storage_;
  }

 protected:
  std::array<CharT, buf_size_> storage_;
  FixedStreamBuf<CharT, Traits> streambuf_;
};

/// Input/Output stream backed by a FixedString storage model
/**
 * Works like std::stringstream. For example:
 * ```
 * FixedStringStreamTpl<char, 100> stream;
 * stream << "hello" << " world" << "!\n";
 * char strbuf[13];
 * stream.str().copy(&strbuf[0], &strbuf[13]);
 * // `strbuf` contains "Hello world!\n"
 * ```
 */
template <typename CharT, size_t buf_size_,
          class Traits = std::char_traits<CharT>>
class FixedStringStreamTpl
    : public FixedStringStreamBase<CharT, buf_size_, Traits>,
      public std::basic_iostream<CharT, Traits> {
 public:
  FixedStringStreamTpl()
      : std::basic_iostream<CharT, Traits>(&this->streambuf_) {}
  ~FixedStringStreamTpl() override = default;

  const CharT* c_str() {
    this->streambuf_->write_null_terminator();
    return this->streambuf_->storage_begin();
  }

  void reset() {
    this->streambuf_.reset();
    this->clear();
  }
};

template <size_t N>
using FixedStringStream = FixedStringStreamTpl<char, N>;

}  // namespace util
