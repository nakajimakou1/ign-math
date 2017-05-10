/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef IGNITION_MATH_EDGE_HH_
#define IGNITION_MATH_EDGE_HH_

#include <algorithm>
#include <cassert>
// int64_t
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <set>

#include "ignition/math/Vertex.hh"

namespace ignition
{
  namespace math
  {
    /// \def EdgeId.
    /// \brief The unique Id for an edge.
    using EdgeId = int64_t;

    /// \brief Used in the Graph constructors for uniform initialization.
    template<typename E>
    struct EdgeInitializer
    {
      /// \brief Constructor.
      /// \param[in] _vertices The vertices of the edge.
      /// \param[in] _data The data stored in the edge.
      /// \param[in] _weight The weight (cost) of the edge.
      EdgeInitializer(const VertexId_A &_vertices,
                      const E &_data,
                      const double _weight = 1)
        : vertices(_vertices),
          data(_data),
          weight(_weight)
      {
      };

      /// \brief IDs of the vertices.
      public: VertexId_A vertices;

      /// \brief User data.
      public: E data;

      /// \brief The weight (cost) of the edge.
      public: double weight = 1;
    };

    /// \brief Generic edge class. An edge has two ends and some constraint
    /// between them. For example, a directed edge only allows traversing the
    /// edge in one direction.
    template<typename E>
    class Edge
    {
      /// \brief Constructor.
      /// \param[in] _id Unique id.
      /// \param[in] _weight The weight (cost) of the edge.
      /// \param[in] _vertices The vertices of the edge.
      /// \param[in] _data The data stored in the edge.
      public: explicit Edge(const EdgeId &_id,
                            const double _weight,
                            const VertexId_A &_vertices,
                            const E &_data)
        : id(_id),
          weight(_weight),
          vertices(_vertices),
          data(_data)
      {
      }

      /// \brief Get the edge Id.
      /// \return The edge Id.
      public: EdgeId Id() const
      {
        return this->id;
      }

      /// \brief Get the destination end that is reachable from a source end of
      /// an edge.
      ///
      /// E.g.: Let's assume that we have an undirected edge (e1) with ends
      /// (v1) and (v2): (v1)--(v2). The operation e1.From(v1) returns (v2).
      /// The operation e1.From(v2) returns (v1).
      ///
      /// E.g.: Let's assume that we have a directed edge (e2) with the tail end
      /// (v1) and the head end (v2): (v1)->(v2). The operation e2.From(v1)
      /// returns (v2). The operation e2.From(v2) returns kNullId.
      ///
      /// \param[in] _from Source vertex.
      /// \return The other vertex of the edge reachable from the "_from"
      /// vertex or kNullId otherwise.
      public: virtual VertexId From(const VertexId &_from) const = 0;

      /// \brief Get the source end that can reach the destination end of
      /// an edge.
      ///
      /// E.g.: Let's assume that we have an undirected edge (e1) with ends
      /// (v1) and (v2): (v1)--(v2). The operation e1.To(v1) returns (v2).
      /// The operation e1.To(v2) returns (v1).
      ///
      /// E.g.: Let's assume that we have a directed edge (e2) with the tail end
      /// (v1) and the head end (v2): (v1)->(v2). The operation e2.To(v1)
      /// returns kNullId. The operation e2.To(v2) returns v1.
      ///
      /// \param[in] _from Destination vertex.
      /// \return The other vertex of the edge that can reach "_to"
      /// vertex or kNullId otherwise.
      public: virtual VertexId To(const VertexId &_to) const = 0;

      /// \brief The cost of traversing the _from end to the other end of the
      /// edge.
      /// \param[in] _from Source vertex.
      /// \return The cost.
      public: double Weight(const VertexId &_from) const
      {
        return this->weight;
      }

      /// \brief Get the two vertices contained in the edge.
      /// \return The two vertices contained in the edge.
      public: VertexId_A Vertices() const
      {
        if (!this->Valid())
          return {kNullId, kNullId};

        return this->vertices;
      }

      /// \brief Get a non-mutable reference to the user data stored in the edge
      /// \return The non-mutable reference to the user data stored in the edge.
      public: const E &Data() const
      {
        return this->data;
      }

      /// \brief Get a mutable reference to the user data stored in the edge.
      /// \return The mutable reference to the user data stored in the edge.
      public: E &Data()
      {
        return this->data;
      }

      /// \brief Get if the edge is valid. An edge is valid if its linked in a
      /// graph and its vertices are reachable.
      /// \return True when the edge is valid or false otherwise (invalid Id).
      public: bool Valid() const
      {
        return this->id != kNullId;
      }

      /// \brief Unique edge Id.
      private: EdgeId id = kNullId;

      /// \brief The weight (cost) of the edge.
      private: double weight = 1.0;

      /// \brief The set of Ids of the two vertices.
      private: VertexId_A vertices;

      /// \brief User data.
      private: E data;
    };

    /// \def EdgeId_S
    /// \brief A set of edge Ids.
    using EdgeId_S = std::set<EdgeId>;

    /// \def EdgeRef_M
    /// \brief A map of edges. The key is the edge Id. The value is a reference
    /// to the edge.
    template<typename EdgeType>
    using EdgeRef_M = std::map<EdgeId, std::reference_wrapper<const EdgeType>>;

    /// \brief An undirected edge represents a connection between two vertices.
    template<typename E>
    class UndirectedEdge : public Edge<E>
    {
      /// \brief An invalid undirected edge.
      public: static UndirectedEdge<E> NullEdge;

      // Documentation inherited.
      using Edge<E>::Edge;

      // Documentation inherited.
      public: VertexId From(const VertexId &_from) const override
      {
        if (!this->Valid())
          return kNullId;

        assert(this->Vertices().size() == 2u);
        if (std::find(this->Vertices().begin(), this->Vertices().end(),
              _from) == this->Vertices().end())
        {
          return kNullId;
        }

        if (this->Vertices().front() == _from)
          return this->Vertices().back();

        return this->Vertices().front();
      }

      // Documentation inherited.
      public: VertexId To(const VertexId &_to) const override
      {
        return this->From(_to);
      }

      /// \brief Stream insertion operator. The output uses DOT graph
      /// description language.
      /// \param[out] _out The output stream.
      /// \param[in] _e Edge to write to the stream.
      /// \ref https://en.wikipedia.org/wiki/DOT_(graph_description_language).
      public: friend std::ostream &operator<<(std::ostream &_out,
                                              const UndirectedEdge<E> &_e)
      {
        auto vertices = _e.Vertices();
        auto it = vertices.begin();
        _out << "  " << *it << " -- ";
        ++it;
        _out << *it << " [label=" << _e.Weight(*it) << "];" << std::endl;
        return _out;
      }
    };

    /// \brief An invalid undirected edge.
    template<typename E>
    UndirectedEdge<E> UndirectedEdge<E>::NullEdge(
      kNullId, 1.0, {kNullId, kNullId}, E());

    /// \brief A directed edge represents a connection between two vertices.
    template<typename E>
    class DirectedEdge : public Edge<E>
    {
      /// \brief An invalid directed edge.
      public: static DirectedEdge<E> NullEdge;

      // Documentation inherited.
      using Edge<E>::Edge;

      /// \brief Get the Id of the tail vertex in this edge.
      /// \return An id of the tail vertex in this edge.
      /// \sa Head()
      public: VertexId Tail() const
      {
        return this->Vertices().front();
      }

      /// \brief Get the Id of the head vertex in this edge.
      /// \return An id of the head vertex in this edge.
      /// \sa Tail()
      public: VertexId Head() const
      {
        return this->Vertices().back();
      }

      // Documentation inherited.
      public: VertexId From(const VertexId &_from) const override
      {
        if (_from != this->Tail())
          return kNullId;

        return this->Head();
      }

      // Documentation inherited.
      public: VertexId To(const VertexId &_to) const override
      {
        if (_to != this->Head())
          return kNullId;

        return this->Tail();
      }

      /// \brief Stream insertion operator. The output uses DOT graph
      /// description language.
      /// \param[out] _out The output stream.
      /// \param[in] _e Edge to write to the stream.
      /// \ref https://en.wikipedia.org/wiki/DOT_(graph_description_language).
      public: friend std::ostream &operator<<(std::ostream &_out,
                                              const DirectedEdge<E> &_e)
      {
        _out << "  " << _e.Tail() << " -> " << _e.Head()
             << " [label=" << _e.Weight(_e.Tail()) << "];" << std::endl;
        return _out;
      }
    };

    /// \brief An invalid directed edge.
    template<typename E>
    DirectedEdge<E> DirectedEdge<E>::NullEdge(
      kNullId, 1.0, {kNullId, kNullId}, E());
  }
}
#endif