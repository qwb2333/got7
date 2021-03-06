// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: feed.proto

#ifndef PROTOBUF_feed_2eproto__INCLUDED
#define PROTOBUF_feed_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace idl {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_feed_2eproto();
void protobuf_AssignDesc_feed_2eproto();
void protobuf_ShutdownFile_feed_2eproto();

class FeedRemoteInfo;
class FeedAction;

enum FeedOption {
  CONNECT = 0,
  DISCONNECT = 1,
  MESSAGE = 2,
  PIPE = 3,
  ACK = 4,
  NEW = 5
};
bool FeedOption_IsValid(int value);
const FeedOption FeedOption_MIN = CONNECT;
const FeedOption FeedOption_MAX = NEW;
const int FeedOption_ARRAYSIZE = FeedOption_MAX + 1;

const ::google::protobuf::EnumDescriptor* FeedOption_descriptor();
inline const ::std::string& FeedOption_Name(FeedOption value) {
  return ::google::protobuf::internal::NameOfEnum(
    FeedOption_descriptor(), value);
}
inline bool FeedOption_Parse(
    const ::std::string& name, FeedOption* value) {
  return ::google::protobuf::internal::ParseNamedEnum<FeedOption>(
    FeedOption_descriptor(), name, value);
}
// ===================================================================

class FeedRemoteInfo : public ::google::protobuf::Message {
 public:
  FeedRemoteInfo();
  virtual ~FeedRemoteInfo();

  FeedRemoteInfo(const FeedRemoteInfo& from);

  inline FeedRemoteInfo& operator=(const FeedRemoteInfo& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const FeedRemoteInfo& default_instance();

  void Swap(FeedRemoteInfo* other);

  // implements Message ----------------------------------------------

  FeedRemoteInfo* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const FeedRemoteInfo& from);
  void MergeFrom(const FeedRemoteInfo& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required int32 port = 1;
  inline bool has_port() const;
  inline void clear_port();
  static const int kPortFieldNumber = 1;
  inline ::google::protobuf::int32 port() const;
  inline void set_port(::google::protobuf::int32 value);

  // optional string ip = 2;
  inline bool has_ip() const;
  inline void clear_ip();
  static const int kIpFieldNumber = 2;
  inline const ::std::string& ip() const;
  inline void set_ip(const ::std::string& value);
  inline void set_ip(const char* value);
  inline void set_ip(const char* value, size_t size);
  inline ::std::string* mutable_ip();
  inline ::std::string* release_ip();
  inline void set_allocated_ip(::std::string* ip);

  // @@protoc_insertion_point(class_scope:idl.FeedRemoteInfo)
 private:
  inline void set_has_port();
  inline void clear_has_port();
  inline void set_has_ip();
  inline void clear_has_ip();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::std::string* ip_;
  ::google::protobuf::int32 port_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];

  friend void  protobuf_AddDesc_feed_2eproto();
  friend void protobuf_AssignDesc_feed_2eproto();
  friend void protobuf_ShutdownFile_feed_2eproto();

  void InitAsDefaultInstance();
  static FeedRemoteInfo* default_instance_;
};
// -------------------------------------------------------------------

class FeedAction : public ::google::protobuf::Message {
 public:
  FeedAction();
  virtual ~FeedAction();

  FeedAction(const FeedAction& from);

  inline FeedAction& operator=(const FeedAction& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const FeedAction& default_instance();

  void Swap(FeedAction* other);

  // implements Message ----------------------------------------------

  FeedAction* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const FeedAction& from);
  void MergeFrom(const FeedAction& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required .idl.FeedOption option = 1;
  inline bool has_option() const;
  inline void clear_option();
  static const int kOptionFieldNumber = 1;
  inline ::idl::FeedOption option() const;
  inline void set_option(::idl::FeedOption value);

  // optional int32 fd = 2;
  inline bool has_fd() const;
  inline void clear_fd();
  static const int kFdFieldNumber = 2;
  inline ::google::protobuf::int32 fd() const;
  inline void set_fd(::google::protobuf::int32 value);

  // optional bytes data = 3;
  inline bool has_data() const;
  inline void clear_data();
  static const int kDataFieldNumber = 3;
  inline const ::std::string& data() const;
  inline void set_data(const ::std::string& value);
  inline void set_data(const char* value);
  inline void set_data(const void* value, size_t size);
  inline ::std::string* mutable_data();
  inline ::std::string* release_data();
  inline void set_allocated_data(::std::string* data);

  // optional .idl.FeedRemoteInfo remoteInfo = 4;
  inline bool has_remoteinfo() const;
  inline void clear_remoteinfo();
  static const int kRemoteInfoFieldNumber = 4;
  inline const ::idl::FeedRemoteInfo& remoteinfo() const;
  inline ::idl::FeedRemoteInfo* mutable_remoteinfo();
  inline ::idl::FeedRemoteInfo* release_remoteinfo();
  inline void set_allocated_remoteinfo(::idl::FeedRemoteInfo* remoteinfo);

  // @@protoc_insertion_point(class_scope:idl.FeedAction)
 private:
  inline void set_has_option();
  inline void clear_has_option();
  inline void set_has_fd();
  inline void clear_has_fd();
  inline void set_has_data();
  inline void clear_has_data();
  inline void set_has_remoteinfo();
  inline void clear_has_remoteinfo();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  int option_;
  ::google::protobuf::int32 fd_;
  ::std::string* data_;
  ::idl::FeedRemoteInfo* remoteinfo_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_feed_2eproto();
  friend void protobuf_AssignDesc_feed_2eproto();
  friend void protobuf_ShutdownFile_feed_2eproto();

  void InitAsDefaultInstance();
  static FeedAction* default_instance_;
};
// ===================================================================


// ===================================================================

// FeedRemoteInfo

// required int32 port = 1;
inline bool FeedRemoteInfo::has_port() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void FeedRemoteInfo::set_has_port() {
  _has_bits_[0] |= 0x00000001u;
}
inline void FeedRemoteInfo::clear_has_port() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void FeedRemoteInfo::clear_port() {
  port_ = 0;
  clear_has_port();
}
inline ::google::protobuf::int32 FeedRemoteInfo::port() const {
  return port_;
}
inline void FeedRemoteInfo::set_port(::google::protobuf::int32 value) {
  set_has_port();
  port_ = value;
}

// optional string ip = 2;
inline bool FeedRemoteInfo::has_ip() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void FeedRemoteInfo::set_has_ip() {
  _has_bits_[0] |= 0x00000002u;
}
inline void FeedRemoteInfo::clear_has_ip() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void FeedRemoteInfo::clear_ip() {
  if (ip_ != &::google::protobuf::internal::kEmptyString) {
    ip_->clear();
  }
  clear_has_ip();
}
inline const ::std::string& FeedRemoteInfo::ip() const {
  return *ip_;
}
inline void FeedRemoteInfo::set_ip(const ::std::string& value) {
  set_has_ip();
  if (ip_ == &::google::protobuf::internal::kEmptyString) {
    ip_ = new ::std::string;
  }
  ip_->assign(value);
}
inline void FeedRemoteInfo::set_ip(const char* value) {
  set_has_ip();
  if (ip_ == &::google::protobuf::internal::kEmptyString) {
    ip_ = new ::std::string;
  }
  ip_->assign(value);
}
inline void FeedRemoteInfo::set_ip(const char* value, size_t size) {
  set_has_ip();
  if (ip_ == &::google::protobuf::internal::kEmptyString) {
    ip_ = new ::std::string;
  }
  ip_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* FeedRemoteInfo::mutable_ip() {
  set_has_ip();
  if (ip_ == &::google::protobuf::internal::kEmptyString) {
    ip_ = new ::std::string;
  }
  return ip_;
}
inline ::std::string* FeedRemoteInfo::release_ip() {
  clear_has_ip();
  if (ip_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = ip_;
    ip_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void FeedRemoteInfo::set_allocated_ip(::std::string* ip) {
  if (ip_ != &::google::protobuf::internal::kEmptyString) {
    delete ip_;
  }
  if (ip) {
    set_has_ip();
    ip_ = ip;
  } else {
    clear_has_ip();
    ip_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// -------------------------------------------------------------------

// FeedAction

// required .idl.FeedOption option = 1;
inline bool FeedAction::has_option() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void FeedAction::set_has_option() {
  _has_bits_[0] |= 0x00000001u;
}
inline void FeedAction::clear_has_option() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void FeedAction::clear_option() {
  option_ = 0;
  clear_has_option();
}
inline ::idl::FeedOption FeedAction::option() const {
  return static_cast< ::idl::FeedOption >(option_);
}
inline void FeedAction::set_option(::idl::FeedOption value) {
  assert(::idl::FeedOption_IsValid(value));
  set_has_option();
  option_ = value;
}

// optional int32 fd = 2;
inline bool FeedAction::has_fd() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void FeedAction::set_has_fd() {
  _has_bits_[0] |= 0x00000002u;
}
inline void FeedAction::clear_has_fd() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void FeedAction::clear_fd() {
  fd_ = 0;
  clear_has_fd();
}
inline ::google::protobuf::int32 FeedAction::fd() const {
  return fd_;
}
inline void FeedAction::set_fd(::google::protobuf::int32 value) {
  set_has_fd();
  fd_ = value;
}

// optional bytes data = 3;
inline bool FeedAction::has_data() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void FeedAction::set_has_data() {
  _has_bits_[0] |= 0x00000004u;
}
inline void FeedAction::clear_has_data() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void FeedAction::clear_data() {
  if (data_ != &::google::protobuf::internal::kEmptyString) {
    data_->clear();
  }
  clear_has_data();
}
inline const ::std::string& FeedAction::data() const {
  return *data_;
}
inline void FeedAction::set_data(const ::std::string& value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(value);
}
inline void FeedAction::set_data(const char* value) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(value);
}
inline void FeedAction::set_data(const void* value, size_t size) {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  data_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* FeedAction::mutable_data() {
  set_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    data_ = new ::std::string;
  }
  return data_;
}
inline ::std::string* FeedAction::release_data() {
  clear_has_data();
  if (data_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = data_;
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void FeedAction::set_allocated_data(::std::string* data) {
  if (data_ != &::google::protobuf::internal::kEmptyString) {
    delete data_;
  }
  if (data) {
    set_has_data();
    data_ = data;
  } else {
    clear_has_data();
    data_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional .idl.FeedRemoteInfo remoteInfo = 4;
inline bool FeedAction::has_remoteinfo() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void FeedAction::set_has_remoteinfo() {
  _has_bits_[0] |= 0x00000008u;
}
inline void FeedAction::clear_has_remoteinfo() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void FeedAction::clear_remoteinfo() {
  if (remoteinfo_ != NULL) remoteinfo_->::idl::FeedRemoteInfo::Clear();
  clear_has_remoteinfo();
}
inline const ::idl::FeedRemoteInfo& FeedAction::remoteinfo() const {
  return remoteinfo_ != NULL ? *remoteinfo_ : *default_instance_->remoteinfo_;
}
inline ::idl::FeedRemoteInfo* FeedAction::mutable_remoteinfo() {
  set_has_remoteinfo();
  if (remoteinfo_ == NULL) remoteinfo_ = new ::idl::FeedRemoteInfo;
  return remoteinfo_;
}
inline ::idl::FeedRemoteInfo* FeedAction::release_remoteinfo() {
  clear_has_remoteinfo();
  ::idl::FeedRemoteInfo* temp = remoteinfo_;
  remoteinfo_ = NULL;
  return temp;
}
inline void FeedAction::set_allocated_remoteinfo(::idl::FeedRemoteInfo* remoteinfo) {
  delete remoteinfo_;
  remoteinfo_ = remoteinfo;
  if (remoteinfo) {
    set_has_remoteinfo();
  } else {
    clear_has_remoteinfo();
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace idl

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ::idl::FeedOption>() {
  return ::idl::FeedOption_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_feed_2eproto__INCLUDED
