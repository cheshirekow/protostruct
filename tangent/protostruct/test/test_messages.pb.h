// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: tangent/protostruct/test/test_messages.proto

#ifndef PROTOBUF_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto__INCLUDED
#define PROTOBUF_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace tangent {
namespace test {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
void protobuf_AssignDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
void protobuf_ShutdownFile_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();

class MyMessageA;
class MyMessageB;
class MyMessageC;

enum MyEnumA {
  MyEnumA_VALUE1 = 0,
  MyEnumA_VALUE2 = 1,
  MyEnumA_VALUE3 = 2,
  MyEnumA_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  MyEnumA_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool MyEnumA_IsValid(int value);
const MyEnumA MyEnumA_MIN = MyEnumA_VALUE1;
const MyEnumA MyEnumA_MAX = MyEnumA_VALUE3;
const int MyEnumA_ARRAYSIZE = MyEnumA_MAX + 1;

const ::google::protobuf::EnumDescriptor* MyEnumA_descriptor();
inline const ::std::string& MyEnumA_Name(MyEnumA value) {
  return ::google::protobuf::internal::NameOfEnum(MyEnumA_descriptor(), value);
}
inline bool MyEnumA_Parse(const ::std::string& name, MyEnumA* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MyEnumA>(
      MyEnumA_descriptor(), name, value);
}
// ===================================================================

class MyMessageA : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:tangent.test.MyMessageA)
                                                       */
{
 public:
  MyMessageA();
  virtual ~MyMessageA();

  MyMessageA(const MyMessageA& from);

  inline MyMessageA& operator=(const MyMessageA& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const MyMessageA& default_instance();

  void Swap(MyMessageA* other);

  // implements Message ----------------------------------------------

  inline MyMessageA* New() const {
    return New(NULL);
  }

  MyMessageA* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MyMessageA& from);
  void MergeFrom(const MyMessageA& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(
      ::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const {
    return _cached_size_;
  }

 private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(MyMessageA* other);

 private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }

 public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional sint32 fieldA = 1;
  void clear_fielda();
  static const int kFieldAFieldNumber = 1;
  ::google::protobuf::int32 fielda() const;
  void set_fielda(::google::protobuf::int32 value);

  // optional double fieldB = 2;
  void clear_fieldb();
  static const int kFieldBFieldNumber = 2;
  double fieldb() const;
  void set_fieldb(double value);

  // optional uint64 fieldC = 3;
  void clear_fieldc();
  static const int kFieldCFieldNumber = 3;
  ::google::protobuf::uint64 fieldc() const;
  void set_fieldc(::google::protobuf::uint64 value);

  // optional .tangent.test.MyEnumA fieldD = 4;
  void clear_fieldd();
  static const int kFieldDFieldNumber = 4;
  ::tangent::test::MyEnumA fieldd() const;
  void set_fieldd(::tangent::test::MyEnumA value);

  // @@protoc_insertion_point(class_scope:tangent.test.MyMessageA)
 private:
  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  double fieldb_;
  ::google::protobuf::int32 fielda_;
  int fieldd_;
  ::google::protobuf::uint64 fieldc_;
  mutable int _cached_size_;
  friend void
  protobuf_AddDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_AssignDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_ShutdownFile_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();

  void InitAsDefaultInstance();
  static MyMessageA* default_instance_;
};
// -------------------------------------------------------------------

class MyMessageB : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:tangent.test.MyMessageB)
                                                       */
{
 public:
  MyMessageB();
  virtual ~MyMessageB();

  MyMessageB(const MyMessageB& from);

  inline MyMessageB& operator=(const MyMessageB& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const MyMessageB& default_instance();

  void Swap(MyMessageB* other);

  // implements Message ----------------------------------------------

  inline MyMessageB* New() const {
    return New(NULL);
  }

  MyMessageB* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MyMessageB& from);
  void MergeFrom(const MyMessageB& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(
      ::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const {
    return _cached_size_;
  }

 private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(MyMessageB* other);

 private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }

 public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .tangent.test.MyMessageA fieldA = 2;
  bool has_fielda() const;
  void clear_fielda();
  static const int kFieldAFieldNumber = 2;
  const ::tangent::test::MyMessageA& fielda() const;
  ::tangent::test::MyMessageA* mutable_fielda();
  ::tangent::test::MyMessageA* release_fielda();
  void set_allocated_fielda(::tangent::test::MyMessageA* fielda);

  // @@protoc_insertion_point(class_scope:tangent.test.MyMessageB)
 private:
  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::tangent::test::MyMessageA* fielda_;
  mutable int _cached_size_;
  friend void
  protobuf_AddDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_AssignDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_ShutdownFile_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();

  void InitAsDefaultInstance();
  static MyMessageB* default_instance_;
};
// -------------------------------------------------------------------

class MyMessageC : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:tangent.test.MyMessageC)
                                                       */
{
 public:
  MyMessageC();
  virtual ~MyMessageC();

  MyMessageC(const MyMessageC& from);

  inline MyMessageC& operator=(const MyMessageC& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const MyMessageC& default_instance();

  void Swap(MyMessageC* other);

  // implements Message ----------------------------------------------

  inline MyMessageC* New() const {
    return New(NULL);
  }

  MyMessageC* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const MyMessageC& from);
  void MergeFrom(const MyMessageC& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(
      ::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const {
    return _cached_size_;
  }

 private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(MyMessageC* other);

 private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }

 public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .tangent.test.MyMessageA fieldA = 1;
  int fielda_size() const;
  void clear_fielda();
  static const int kFieldAFieldNumber = 1;
  const ::tangent::test::MyMessageA& fielda(int index) const;
  ::tangent::test::MyMessageA* mutable_fielda(int index);
  ::tangent::test::MyMessageA* add_fielda();
  ::google::protobuf::RepeatedPtrField< ::tangent::test::MyMessageA>*
  mutable_fielda();
  const ::google::protobuf::RepeatedPtrField< ::tangent::test::MyMessageA>&
  fielda() const;

  // repeated int32 fieldB = 2 [packed = true];
  int fieldb_size() const;
  void clear_fieldb();
  static const int kFieldBFieldNumber = 2;
  ::google::protobuf::int32 fieldb(int index) const;
  void set_fieldb(int index, ::google::protobuf::int32 value);
  void add_fieldb(::google::protobuf::int32 value);
  const ::google::protobuf::RepeatedField< ::google::protobuf::int32>& fieldb()
      const;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32>*
  mutable_fieldb();

  // @@protoc_insertion_point(class_scope:tangent.test.MyMessageC)
 private:
  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::RepeatedPtrField< ::tangent::test::MyMessageA> fielda_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32> fieldb_;
  mutable int _fieldb_cached_byte_size_;
  mutable int _cached_size_;
  friend void
  protobuf_AddDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_AssignDesc_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();
  friend void
  protobuf_ShutdownFile_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto();

  void InitAsDefaultInstance();
  static MyMessageC* default_instance_;
};
// ===================================================================

// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// MyMessageA

// optional sint32 fieldA = 1;
inline void MyMessageA::clear_fielda() {
  fielda_ = 0;
}
inline ::google::protobuf::int32 MyMessageA::fielda() const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageA.fieldA)
  return fielda_;
}
inline void MyMessageA::set_fielda(::google::protobuf::int32 value) {
  fielda_ = value;
  // @@protoc_insertion_point(field_set:tangent.test.MyMessageA.fieldA)
}

// optional double fieldB = 2;
inline void MyMessageA::clear_fieldb() {
  fieldb_ = 0;
}
inline double MyMessageA::fieldb() const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageA.fieldB)
  return fieldb_;
}
inline void MyMessageA::set_fieldb(double value) {
  fieldb_ = value;
  // @@protoc_insertion_point(field_set:tangent.test.MyMessageA.fieldB)
}

// optional uint64 fieldC = 3;
inline void MyMessageA::clear_fieldc() {
  fieldc_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 MyMessageA::fieldc() const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageA.fieldC)
  return fieldc_;
}
inline void MyMessageA::set_fieldc(::google::protobuf::uint64 value) {
  fieldc_ = value;
  // @@protoc_insertion_point(field_set:tangent.test.MyMessageA.fieldC)
}

// optional .tangent.test.MyEnumA fieldD = 4;
inline void MyMessageA::clear_fieldd() {
  fieldd_ = 0;
}
inline ::tangent::test::MyEnumA MyMessageA::fieldd() const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageA.fieldD)
  return static_cast< ::tangent::test::MyEnumA>(fieldd_);
}
inline void MyMessageA::set_fieldd(::tangent::test::MyEnumA value) {
  fieldd_ = value;
  // @@protoc_insertion_point(field_set:tangent.test.MyMessageA.fieldD)
}

// -------------------------------------------------------------------

// MyMessageB

// optional .tangent.test.MyMessageA fieldA = 2;
inline bool MyMessageB::has_fielda() const {
  return !_is_default_instance_ && fielda_ != NULL;
}
inline void MyMessageB::clear_fielda() {
  if (GetArenaNoVirtual() == NULL && fielda_ != NULL) delete fielda_;
  fielda_ = NULL;
}
inline const ::tangent::test::MyMessageA& MyMessageB::fielda() const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageB.fieldA)
  return fielda_ != NULL ? *fielda_ : *default_instance_->fielda_;
}
inline ::tangent::test::MyMessageA* MyMessageB::mutable_fielda() {
  if (fielda_ == NULL) {
    fielda_ = new ::tangent::test::MyMessageA;
  }
  // @@protoc_insertion_point(field_mutable:tangent.test.MyMessageB.fieldA)
  return fielda_;
}
inline ::tangent::test::MyMessageA* MyMessageB::release_fielda() {
  // @@protoc_insertion_point(field_release:tangent.test.MyMessageB.fieldA)

  ::tangent::test::MyMessageA* temp = fielda_;
  fielda_ = NULL;
  return temp;
}
inline void MyMessageB::set_allocated_fielda(
    ::tangent::test::MyMessageA* fielda) {
  delete fielda_;
  fielda_ = fielda;
  if (fielda) {
  } else {
  }
  // @@protoc_insertion_point(field_set_allocated:tangent.test.MyMessageB.fieldA)
}

// -------------------------------------------------------------------

// MyMessageC

// repeated .tangent.test.MyMessageA fieldA = 1;
inline int MyMessageC::fielda_size() const {
  return fielda_.size();
}
inline void MyMessageC::clear_fielda() {
  fielda_.Clear();
}
inline const ::tangent::test::MyMessageA& MyMessageC::fielda(int index) const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageC.fieldA)
  return fielda_.Get(index);
}
inline ::tangent::test::MyMessageA* MyMessageC::mutable_fielda(int index) {
  // @@protoc_insertion_point(field_mutable:tangent.test.MyMessageC.fieldA)
  return fielda_.Mutable(index);
}
inline ::tangent::test::MyMessageA* MyMessageC::add_fielda() {
  // @@protoc_insertion_point(field_add:tangent.test.MyMessageC.fieldA)
  return fielda_.Add();
}
inline ::google::protobuf::RepeatedPtrField< ::tangent::test::MyMessageA>*
MyMessageC::mutable_fielda() {
  // @@protoc_insertion_point(field_mutable_list:tangent.test.MyMessageC.fieldA)
  return &fielda_;
}
inline const ::google::protobuf::RepeatedPtrField< ::tangent::test::MyMessageA>&
MyMessageC::fielda() const {
  // @@protoc_insertion_point(field_list:tangent.test.MyMessageC.fieldA)
  return fielda_;
}

// repeated int32 fieldB = 2 [packed = true];
inline int MyMessageC::fieldb_size() const {
  return fieldb_.size();
}
inline void MyMessageC::clear_fieldb() {
  fieldb_.Clear();
}
inline ::google::protobuf::int32 MyMessageC::fieldb(int index) const {
  // @@protoc_insertion_point(field_get:tangent.test.MyMessageC.fieldB)
  return fieldb_.Get(index);
}
inline void MyMessageC::set_fieldb(int index, ::google::protobuf::int32 value) {
  fieldb_.Set(index, value);
  // @@protoc_insertion_point(field_set:tangent.test.MyMessageC.fieldB)
}
inline void MyMessageC::add_fieldb(::google::protobuf::int32 value) {
  fieldb_.Add(value);
  // @@protoc_insertion_point(field_add:tangent.test.MyMessageC.fieldB)
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32>&
MyMessageC::fieldb() const {
  // @@protoc_insertion_point(field_list:tangent.test.MyMessageC.fieldB)
  return fieldb_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32>*
MyMessageC::mutable_fieldb() {
  // @@protoc_insertion_point(field_mutable_list:tangent.test.MyMessageC.fieldB)
  return &fieldb_;
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// @@protoc_insertion_point(namespace_scope)

}  // namespace test
}  // namespace tangent

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
struct is_proto_enum< ::tangent::test::MyEnumA>
    : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::tangent::test::MyEnumA>() {
  return ::tangent::test::MyEnumA_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_tangent_2fprotostruct_2ftest_2ftest_5fmessages_2eproto__INCLUDED