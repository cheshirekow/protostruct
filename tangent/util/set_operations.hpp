// Copyright (C) 2014 Josh Bialkowski (josh.bialkowski@gmail.com)
/**
 *  @file
 *  @date   Sept 17, 2014
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 *  @brief
 */
#pragma once
#include <algorithm>
#include "tangent/util/null_out.h"

namespace set {

/// Returns true if the seconds set is a subset of the first set
template <typename InputIt1, typename InputIt2>
bool contains(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2);

/// Returns true if the seconds set is a subset of the first set
template <typename Container1, typename Container2>
bool contains(const Container1& container1, const Container2& container2);

/// Collect in out1 elements from the first set not found in the second, and
/// collect in out2 elements from the second set not found in the first
template <typename InputIt1, typename InputIt2, typename OutputIt1,
          typename OutputIt2>
void symmetric_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                          InputIt2 last2, OutputIt1 out1, OutputIt2 out2);

/// Collect in out1 elements from the first set not found in the second, and
/// collect in out2 elements from the second set not found in the first, and
/// collect in intersect the elements from first and second that are common
/// to both.
template <typename InputIt1, typename InputIt2, typename OutputIt1,
          typename OutputIt2, typename OutputIt3>
void intersection_and_difference(InputIt1 first1, InputIt1 last1,
                                 InputIt2 first2, InputIt2 last2,
                                 OutputIt1 out1, OutputIt2 out2,
                                 OutputIt3 intersect);

/// Return all elements that are in either A or B but not in both
template <typename InputIt1, typename InputIt2, typename OutputIt>
void complement_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                             InputIt2 last2, OutputIt out);

}  // namespace set

// ----------------------------------------------------------------------------
//                          Implementation
// ----------------------------------------------------------------------------

namespace set {

template <class Container1, class Container2, typename OutputIterator>
void get_union(const Container1& container1, const Container2& container2,
               OutputIterator out) {
  std::set_union(container1.begin(), container1.end(), container2.begin(),
                 container2.end(), out);
}

template <class Container1, class Container2, typename OutputIterator>
void get_intersection(const Container1& container1,
                      const Container2& container2, OutputIterator out) {
  std::set_intersection(container1.begin(), container1.end(),
                        container2.begin(), container2.end(), out);
}

template <class Container1, class Container2, typename OutputIterator>
void intersection(const Container1& container1, const Container2& container2,
                  OutputIterator out) {
  std::set_intersection(container1.begin(), container1.end(),
                        container2.begin(), container2.end(), out);
}

template <class Container>
void sort(Container* container) {
  std::sort(container->begin(), container->end());
}

template <class Container, class Compare>
void sort(Container* container, Compare compare) {
  std::sort(container->begin(), container->end(), compare);
}

template <class Container1, class Container2, class Output>
void difference(const Container1& container1, const Container2& container2,
                Output out) {
  std::set_difference(container1.begin(), container1.end(), container2.begin(),
                      container2.end(), out);
}

/// Returns true if the seconds set is a subset of the first set
template <typename InputIt1, typename InputIt2>
bool contains(InputIt1 first1, InputIt1 last1, InputIt2 first2,
              InputIt2 last2) {
  while (first2 != last2) {
    for (; *first1 < *first2; ++first1) {
      if (first1 == last1) {
        return false;
      }
    }
    if (*first2 < *first1) {
      return false;
    } else {
      ++first2;
      ++first1;
    }
  }
  return true;
}

template <typename Container1, typename Container2>
bool contains(const Container1& container1, const Container2& container2) {
  return contains(container1.begin(), container1.end(), container2.begin(),
                  container2.end());
}

template <typename InputIt1, typename InputIt2, typename OutputIt1,
          typename OutputIt2>
void symmetric_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                          InputIt2 last2, OutputIt1 out1, OutputIt2 out2) {
  while (first1 != last1) {
    if (first2 == last2) {
      std::copy(first1, last1, out1);
      return;
    }
    if (*first1 < *first2) {
      *out1++ = *first1++;
    } else {
      if (*first2 < *first1) {
        *out2++ = *first2;
      } else {
        ++first1;
      }
      ++first2;
    }
  }
  std::copy(first2, last2, out2);
  return;
}

template <class Container1, class Container2, class Output1, class Output2>
void symmetric_difference(const Container1& container1,
                          const Container2& container2, Output1 out1,
                          Output2 out2) {
  symmetric_difference(container1.begin(), container1.end(), container2.begin(),
                       container2.end(), out1, out2);
}

template <typename InputIt1, typename InputIt2, typename OutputIt>
void complement_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                             InputIt2 last2, OutputIt out) {
  while (first1 != last1) {
    if (first2 == last2) {
      std::copy(first1, last1, out);
      return;
    }
    if (*first1 < *first2) {
      *out++ = *first1++;
    } else {
      if (*first2 < *first1) {
        *out++ = *first2;
      } else {
        ++first1;
      }
      ++first2;
    }
  }
  std::copy(first2, last2, out);
  return;
}

template <class Container1, class Container2, class Output>
void complement_intersection(const Container1& container1,
                             const Container2& container2, Output out) {
  complement_intersection(container1.begin(), container1.end(),
                          container2.begin(), container2.end(), out);
}

template <typename InputIt1, typename InputIt2, typename OutputIt1,
          typename OutputIt2, typename OutputIt3>
void intersection_and_difference(InputIt1 first1, InputIt1 last1,
                                 InputIt2 first2, InputIt2 last2,
                                 OutputIt1 out1, OutputIt2 out2,
                                 OutputIt3 intersect) {
  while (first1 != last1) {
    if (first2 == last2) {
      std::copy(first1, last1, out1);
      return;
    }
    // *first1 is in the first set, but not the second set
    if (*first1 < *first2) {
      *out1++ = *first1++;
    } else {
      // *first2 is in the second set, but not the first
      if (*first2 < *first1) {
        *out2++ = *first2++;
        // *first1 == *first2 and it is in both sets
      } else {
        *intersect++ = *first1++;
        ++first2;
      }
    }
  }
  std::copy(first2, last2, out2);
  return;
}

template <typename Container1, typename Container2, typename OutputIt1,
          typename OutputIt2, typename OutputIt3>
void intersection_and_difference(const Container1& container1,
                                 const Container2& container2, OutputIt1 out1,
                                 OutputIt2 out2, OutputIt3 intersect) {
  intersection_and_difference(container1.begin(), container1.end(),
                              container2.begin(), container2.end(), out1, out2,
                              intersect);
}

}  // namespace set
