/*
  Copyright (c) 2017 Genome Research Ltd.

  Author: Jouni Siren <jouni.siren@iki.fi>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef GBWT_ALGORITHMS_H
#define GBWT_ALGORITHMS_H

#include <gbwt/utils.h>

namespace gbwt
{

/*
  algorithms.h: High-level search algorithms.
*/

//------------------------------------------------------------------------------

struct SearchState
{
  node_type  node;
  range_type range;

  SearchState() : node(0), range(Range::empty_range()) {}
  SearchState(node_type node_id, range_type offset_range) : node(node_id), range(offset_range) {}
  SearchState(node_type node_id, size_type sp, size_type ep) : node(node_id), range(sp, ep) {}

  size_type size() const { return Range::length(this->range); }
  bool empty() const { return Range::empty(this->range); }
};

//------------------------------------------------------------------------------

/*
  If the parameters are invalid or if there are no matches, the search algorithms return an
  empty SearchState.

  Template parameters:
    GBWTType  GBWT or DynamicGBWT
    Iterator  InputIterator
*/

template<class GBWTType, class Iterator>
SearchState
extend(const GBWTType& index, SearchState state, Iterator begin, Iterator end)
{
  while(begin != end && !(Range::empty(state.range)))
  {
    if(!(index.contains(*begin))) { return SearchState(); }
    state.range = index.LF(state, *begin);
    state.node = *begin;
    ++begin;
  }
  return state;
}

template<class GBWTType, class Iterator>
SearchState
find(const GBWTType& index, Iterator begin, Iterator end)
{
  if(begin == end) { return SearchState(); }

  if(!(index.contains(*begin))) { return SearchState(); }
  SearchState state(*begin, 0, index.count(*begin) - 1);
  ++begin;

  return gbwt::extend(index, state, begin, end);
}

template<class GBWTType, class Iterator>
SearchState
prefix(const GBWTType& index, Iterator begin, Iterator end)
{
  SearchState state(ENDMARKER, 0, index.sequences() - 1);
  return gbwt::extend(index, state, begin, end);
}

//------------------------------------------------------------------------------

/*
  If the parameters are invalid, the locate algorithms return invalid_sequence() or an
  empty vector.

  Template parameters:
    GBWTType  GBWT or DynamicGBWT
*/

template<class GBWTType>
size_type
locate(const GBWTType& index, edge_type position)
{
  if(!(index.contains(position.first))) { return invalid_sequence(); }

  // No need to check for invalid_edge(), if the initial position is valid.
  while(true)
  {
    size_type result = index.tryLocate(position);
    if(result != invalid_sequence()) { return result; }
    position = index.LF(position);
  }
}

//------------------------------------------------------------------------------

/*
  If the parameters are invalid, the extraction algorithms return an empty vector.

  Template parameters:
    GBWTType  GBWT or DynamicGBWT
*/

template<class GBWTType>
std::vector<node_type>
extract(const GBWTType& index, size_type sequence)
{
  std::vector<node_type> result;
  if(sequence >= index.sequences()) { return result; }

  edge_type position = index.start(sequence);
  if(position == invalid_edge()) { return result; }

  // No need to check for invalid_edge(), if the initial position is valid.
  while(position.first != ENDMARKER)
  {
    result.push_back(position.first);
    position = index.LF(position);
  }
  return result;
}

//------------------------------------------------------------------------------

} // namespace gbwt

#endif // GBWT_ALGORITHMS_H
