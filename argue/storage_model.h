#pragma once
// Copyright 2018-2020 Josh Bialkowski <josh.bialkowski@gmail.com>

namespace argue {

// =============================================================================
//                           Storage Model
// =============================================================================

// Abstract interface into different container types
/* All containers of a particular value type are wrapped by some class which
 * derives from StorageModel<T>, allowing us to interface with those containers
 * without knowing their remaining template parameters */
template <typename T>
class StorageModel {
 public:
  virtual ~StorageModel() {}

  // initialize storage for a given lenth.
  /* Note that the capacity_hint might be zero, in the case of "append"
   * type actions, or narg="+""... in which case the storage should be
   * large enough to hold all the expected values, or should be growable. */
  virtual void init(size_t capacity_hint) = 0;

  // Append an element to the list model
  virtual void append(const T& value) = 0;

  // Assign a value to the scalar model
  virtual void assign(const T& value) = 0;

 protected:
  std::string type_name_;
};

// Abstract the interface into a `std::list`
template <typename T, class Allocator>
class ListModel : public StorageModel<T> {
 public:
  explicit ListModel(std::list<T, Allocator>* dest);
  virtual ~ListModel() {}

  void init(size_t /*capacity_hint*/) override;
  void append(const T& value) override;
  void assign(const T& value) override;

  static std::shared_ptr<StorageModel<T>> create(
      std::list<T, Allocator>* dest) {
    return std::make_shared<ListModel<T, Allocator>>(dest);
  }

 private:
  std::list<T, Allocator>* dest_;
};

// Abstract the interface into a `std::vector`
template <typename T, class Allocator>
class VectorModel : public StorageModel<T> {
 public:
  explicit VectorModel(std::vector<T, Allocator>* dest);
  virtual ~VectorModel() {}

  void init(size_t capacity_hint);
  void append(const T& value);
  void assign(const T& value);

  static std::shared_ptr<StorageModel<T>> create(
      std::vector<T, Allocator>* dest) {
    return std::make_shared<VectorModel<T, Allocator>>(dest);
  }

 private:
  std::vector<T, Allocator>* dest_;
};

// Abstract interface into a scalar pointer
template <typename T>
class ScalarModel : public StorageModel<T> {
 public:
  explicit ScalarModel(T* dest);
  virtual ~ScalarModel() {}

  void init(size_t capacity_hint);
  void append(const T& value);
  void assign(const T& value);

  static std::shared_ptr<StorageModel<T>> create(T* dest) {
    return std::make_shared<ScalarModel<T>>(dest);
  }

 private:
  T* dest_;
};

}  // namespace argue
