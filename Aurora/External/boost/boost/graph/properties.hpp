//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_PROPERTIES_HPP
#define BOOST_GRAPH_PROPERTIES_HPP

#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/pending/property.hpp>
#include <boost/detail/workaround.hpp>

// Include the property map library and extensions in the BGL.
#include <boost/property_map/property_map.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/property_maps/null_property_map.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/limits.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/if.hpp>

#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
// Stay out of the way of the concept checking class
# define Graph Graph_
# define RandomAccessContainer RandomAccessContainer_
#endif

namespace boost {

  enum default_color_type { white_color, gray_color, green_color, red_color, black_color };

  template <class ColorValue>
  struct color_traits {
    static default_color_type white() { return white_color; }
    static default_color_type gray() { return gray_color; }
    static default_color_type green() { return green_color; }
    static default_color_type red() { return red_color; }
    static default_color_type black() { return black_color; }
  };

  // These functions are now obsolete, replaced by color_traits.
  inline default_color_type white(default_color_type) { return white_color; }
  inline default_color_type gray(default_color_type) { return gray_color; }
  inline default_color_type green(default_color_type) { return green_color; }
  inline default_color_type red(default_color_type) { return red_color; }
  inline default_color_type black(default_color_type) { return black_color; }

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <>
  struct property_traits<default_color_type*> {
    typedef default_color_type value_type;
    typedef std::ptrdiff_t key_type;
    typedef default_color_type& reference;
    typedef lvalue_property_map_tag category;
  };
  // get/put already defined for T*
#endif

  struct graph_property_tag { };
  struct vertex_property_tag { };
  struct edge_property_tag { };

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  // See examples/edge_property.cpp for how to use this.
#define BOOST_INSTALL_PROPERTY(KIND, NAME) \
  template <> struct property_kind<KIND##_##NAME##_t> { \
    typedef KIND##_property_tag type; \
  }
#else
#define BOOST_INSTALL_PROPERTY(KIND, NAME) \
  template <> struct property_kind<KIND##_##NAME##_t> { \
    typedef KIND##_property_tag type; \
  }
#endif

#define BOOST_DEF_PROPERTY(KIND, NAME) \
  enum KIND##_##NAME##_t { KIND##_##NAME }; \
  BOOST_INSTALL_PROPERTY(KIND, NAME)

  BOOST_DEF_PROPERTY(vertex, all);
  BOOST_DEF_PROPERTY(edge, all);
  BOOST_DEF_PROPERTY(graph, all);
  BOOST_DEF_PROPERTY(vertex, index);
  BOOST_DEF_PROPERTY(vertex, index1);
  BOOST_DEF_PROPERTY(vertex, index2);
  BOOST_DEF_PROPERTY(vertex, root);
  BOOST_DEF_PROPERTY(edge, index);
  BOOST_DEF_PROPERTY(edge, name);
  BOOST_DEF_PROPERTY(edge, weight);
  BOOST_DEF_PROPERTY(edge, weight2);
  BOOST_DEF_PROPERTY(edge, color);
  BOOST_DEF_PROPERTY(vertex, name);
  BOOST_DEF_PROPERTY(graph, name);
  BOOST_DEF_PROPERTY(vertex, distance);
  BOOST_DEF_PROPERTY(vertex, distance2);
  BOOST_DEF_PROPERTY(vertex, color);
  BOOST_DEF_PROPERTY(vertex, degree);
  BOOST_DEF_PROPERTY(vertex, in_degree);
  BOOST_DEF_PROPERTY(vertex, out_degree);
  BOOST_DEF_PROPERTY(vertex, current_degree);
  BOOST_DEF_PROPERTY(vertex, priority);
  BOOST_DEF_PROPERTY(vertex, discover_time);
  BOOST_DEF_PROPERTY(vertex, finish_time);
  BOOST_DEF_PROPERTY(vertex, predecessor);
  BOOST_DEF_PROPERTY(vertex, rank);
  BOOST_DEF_PROPERTY(vertex, centrality);
  BOOST_DEF_PROPERTY(vertex, lowpoint);
  BOOST_DEF_PROPERTY(vertex, potential);
  BOOST_DEF_PROPERTY(vertex, update);
  BOOST_DEF_PROPERTY(vertex, underlying);
  BOOST_DEF_PROPERTY(edge, reverse);
  BOOST_DEF_PROPERTY(edge, capacity);
  BOOST_DEF_PROPERTY(edge, flow);
  BOOST_DEF_PROPERTY(edge, residual_capacity);
  BOOST_DEF_PROPERTY(edge, centrality);
  BOOST_DEF_PROPERTY(edge, discover_time);
  BOOST_DEF_PROPERTY(edge, update);
  BOOST_DEF_PROPERTY(edge, finished);
  BOOST_DEF_PROPERTY(edge, underlying);
  BOOST_DEF_PROPERTY(graph, visitor);

  // These tags are used for property bundles
  // BOOST_DEF_PROPERTY(graph, bundle); -- needed in graph_traits.hpp, so enum is defined there
  BOOST_INSTALL_PROPERTY(graph, bundle);
  BOOST_DEF_PROPERTY(vertex, bundle);
  BOOST_DEF_PROPERTY(edge, bundle);

  // These tags are used to denote the owners and local descriptors
  // for the vertices and edges of a distributed graph.
  BOOST_DEF_PROPERTY(vertex, global);
  BOOST_DEF_PROPERTY(vertex, owner);
  BOOST_DEF_PROPERTY(vertex, local);
  BOOST_DEF_PROPERTY(edge, global);
  BOOST_DEF_PROPERTY(edge, owner);
  BOOST_DEF_PROPERTY(edge, local);
  BOOST_DEF_PROPERTY(vertex, local_index);
  BOOST_DEF_PROPERTY(edge, local_index);

#undef BOOST_DEF_PROPERTY

  namespace detail {

    struct dummy_edge_property_selector {
      template <class Graph, class Property, class Tag>
      struct bind_ {
        typedef identity_property_map type;
        typedef identity_property_map const_type;
      };
    };
    struct dummy_vertex_property_selector {
      template <class Graph, class Property, class Tag>
      struct bind_ {
        typedef identity_property_map type;
        typedef identity_property_map const_type;
      };
    };

  } // namespace detail

  // Graph classes can either partially specialize property_map
  // or they can specialize these two selector classes.
  template <class GraphTag>
  struct edge_property_selector {
    typedef detail::dummy_edge_property_selector type;
  };

  template <class GraphTag>
  struct vertex_property_selector {
    typedef detail::dummy_vertex_property_selector type;
  };

  namespace detail {

    template <typename A> struct return_void {typedef void type;};

    template <typename Graph, typename Enable = void>
    struct graph_tag_or_void {
      typedef void type;
    };

    template <typename Graph>
    struct graph_tag_or_void<Graph, typename return_void<typename Graph::graph_tag>::type> {
      typedef typename Graph::graph_tag type;
    };

    template <class Graph, class PropertyTag>
    struct edge_property_map {
      typedef typename edge_property_type<Graph>::type Property;
      typedef typename graph_tag_or_void<Graph>::type graph_tag;
      typedef typename edge_property_selector<graph_tag>::type Selector;
      typedef typename Selector::template bind_<Graph,Property,PropertyTag>
        Bind;
      typedef typename Bind::type type;
      typedef typename Bind::const_type const_type;
    };
    template <class Graph, class PropertyTag>
    class vertex_property_map {
    public:
      typedef typename vertex_property_type<Graph>::type Property;
      typedef typename graph_tag_or_void<Graph>::type graph_tag;
      typedef typename vertex_property_selector<graph_tag>::type Selector;
      typedef typename Selector::template bind_<Graph,Property,PropertyTag>
        Bind;
    public:
      typedef typename Bind::type type;
      typedef typename Bind::const_type const_type;
    };

    // This selects the kind of property map, whether is maps from
    // edges or from vertices.
    //
    // It is overly complicated because it's a workaround for
    // partial specialization.
    struct choose_vertex_property_map {
      template <class Graph, class Property>
      struct bind_ {
        typedef vertex_property_map<Graph, Property> type;
      };
    };
    struct choose_edge_property_map {
      template <class Graph, class Property>
      struct bind_ {
        typedef edge_property_map<Graph, Property> type;
      };
    };
    template <class Kind>
    struct property_map_kind_selector {
      // VC++ gets confused if this isn't defined, even though
      // this never gets used.
      typedef choose_vertex_property_map type;
    };
    template <> struct property_map_kind_selector<vertex_property_tag> {
      typedef choose_vertex_property_map type;
    };
    template <> struct property_map_kind_selector<edge_property_tag> {
      typedef choose_edge_property_map type;
    };
  } // namespace detail

  template <class Graph, class Property>
  struct property_map {
  // private:
    typedef typename property_kind<Property>::type Kind;
    typedef typename detail::property_map_kind_selector<Kind>::type Selector;
    typedef typename Selector::template bind_<Graph, Property> Bind;
    typedef typename Bind::type Map;
  public:
    typedef typename Map::type type;
    typedef typename Map::const_type const_type;
  };

  // shortcut for accessing the value type of the property map
  template <class Graph, class Property>
  class property_map_value {
    typedef typename property_map<Graph, Property>::const_type PMap;
  public:
    typedef typename property_traits<PMap>::value_type type;
  };

  template <class Graph, class Property>
  class graph_property {
  public:
    typedef typename property_value<
      typename boost::graph_property_type<Graph>::type, Property
    >::type type;
  };

  template <class Graph>
  class vertex_property {
  public:
    typedef typename Graph::vertex_property_type type;
  };
  template <class Graph>
  class edge_property {
  public:
    typedef typename Graph::edge_property_type type;
  };

  template <typename Graph>
  class degree_property_map
    : public put_get_helper<typename graph_traits<Graph>::degree_size_type,
                            degree_property_map<Graph> >
  {
  public:
    typedef typename graph_traits<Graph>::vertex_descriptor key_type;
    typedef typename graph_traits<Graph>::degree_size_type value_type;
    typedef value_type reference;
    typedef readable_property_map_tag category;
    degree_property_map(const Graph& g) : m_g(g) { }
    value_type operator[](const key_type& v) const {
      return degree(v, m_g);
    }
  private:
    const Graph& m_g;
  };
  template <typename Graph>
  inline degree_property_map<Graph>
  make_degree_map(const Graph& g) {
    return degree_property_map<Graph>(g);
  }

  //========================================================================
  // Iterator Property Map Generating Functions contributed by
  // Kevin Vanhorn. (see also the property map generating functions
  // in boost/property_map/property_map.hpp)

#if !defined(BOOST_NO_STD_ITERATOR_TRAITS)
  // A helper function for creating a vertex property map out of a
  // random access iterator and the internal vertex index map from a
  // graph.
  template <class PropertyGraph, class RandomAccessIterator>
  inline
  iterator_property_map<
    RandomAccessIterator,
    typename property_map<PropertyGraph, vertex_index_t>::type,
    typename std::iterator_traits<RandomAccessIterator>::value_type,
    typename std::iterator_traits<RandomAccessIterator>::reference
  >
  make_iterator_vertex_map(RandomAccessIterator iter, const PropertyGraph& g)
  {
    return make_iterator_property_map(iter, get(vertex_index, g));
  }

  // Use this next function when vertex_descriptor is known to be an
  // integer type, with values ranging from 0 to num_vertices(g).
  //
  template <class RandomAccessIterator>
  inline
  iterator_property_map<
    RandomAccessIterator,
    identity_property_map,
    typename std::iterator_traits<RandomAccessIterator>::value_type,
    typename std::iterator_traits<RandomAccessIterator>::reference
  >
  make_iterator_vertex_map(RandomAccessIterator iter)
  {
    return make_iterator_property_map(iter, identity_property_map());
  }
#endif

  template <class PropertyGraph, class RandomAccessContainer>
  inline
  iterator_property_map<
    typename RandomAccessContainer::iterator,
    typename property_map<PropertyGraph, vertex_index_t>::type,
    typename RandomAccessContainer::value_type,
    typename RandomAccessContainer::reference
  >
  make_container_vertex_map(RandomAccessContainer& c, const PropertyGraph& g)
  {
    BOOST_ASSERT(c.size() >= num_vertices(g));
    return make_iterator_vertex_map(c.begin(), g);
  }

  template <class RandomAccessContainer> inline
  iterator_property_map<
    typename RandomAccessContainer::iterator,
    identity_property_map,
    typename RandomAccessContainer::value_type,
    typename RandomAccessContainer::reference
  >
  make_container_vertex_map(RandomAccessContainer& c)
  {
    return make_iterator_vertex_map(c.begin());
  }

#if defined (BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
#  define BOOST_GRAPH_NO_BUNDLED_PROPERTIES
#endif

#if BOOST_WORKAROUND(__SUNPRO_CC, BOOST_TESTED_AT(0x590)) && !defined (BOOST_GRAPH_NO_BUNDLED_PROPERTIES)
// This compiler cannot define a partial specialization based on a
// pointer-to-member type, as seen in boost/graph/subgraph.hpp line 985 (as of
// trunk r53912)
#  define BOOST_GRAPH_NO_BUNDLED_PROPERTIES
#endif

#ifndef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
  template<typename Graph, typename Descriptor, typename Bundle, typename T>
  struct bundle_property_map
    : put_get_helper<T&, bundle_property_map<Graph, Descriptor, Bundle, T> >
  {
    typedef Descriptor key_type;
    typedef typename remove_const<T>::type value_type;
    typedef T& reference;
    typedef lvalue_property_map_tag category;

    bundle_property_map() { }
    bundle_property_map(Graph* g_, T Bundle::* pm_) : g(g_), pm(pm_) {}

    reference operator[](key_type k) const { return (*g)[k].*pm; }
  private:
    Graph* g;
    T Bundle::* pm;
  };

  namespace detail {
    template<typename VertexBundle, typename EdgeBundle, typename Bundle>
      struct is_vertex_bundle
      : mpl::and_<is_convertible<VertexBundle*, Bundle*>,
                  mpl::and_<mpl::not_<is_void<VertexBundle> >,
                            mpl::not_<is_same<VertexBundle, no_property> > > >
      { };
  }

  // Specialize the property map template to generate bundled property maps.
  template <typename Graph, typename T, typename Bundle>
  struct property_map<Graph, T Bundle::*>
  {
  private:
    typedef graph_traits<Graph> traits;
    typedef typename Graph::vertex_bundled vertex_bundled;
    typedef typename Graph::edge_bundled edge_bundled;
    typedef typename mpl::if_c<(detail::is_vertex_bundle<vertex_bundled, edge_bundled, Bundle>::value),
                       typename traits::vertex_descriptor,
                       typename traits::edge_descriptor>::type
      descriptor;
    typedef typename mpl::if_c<(detail::is_vertex_bundle<vertex_bundled, edge_bundled, Bundle>::value),
                       vertex_bundled,
                       edge_bundled>::type
      actual_bundle;

  public:
    typedef bundle_property_map<Graph, descriptor, actual_bundle, T> type;
    typedef bundle_property_map<const Graph, descriptor, actual_bundle, const T>
      const_type;
  };
#endif

// These metafunctions help implement the process of determining the vertex
// and edge properties of a graph.
namespace graph_detail {
    template<typename Retag>
    struct retagged_property {
        typedef typename Retag::type type;
    };

    // Search the normalized PropList (as returned by retagged<>::type) for
    // the given bundle. Return the type error if no such bundle can be found.
    template <typename PropList, typename Bundle>
    struct retagged_bundle {
      typedef typename property_value<PropList, Bundle>::type Value;
      typedef typename mpl::if_<
        is_same<Value, detail::error_property_not_found>, no_bundle, Value
      >::type type;
    };

    template<typename Prop, typename Bundle>
    class normal_property {
      // Normalize the property into a property list.
      typedef detail::retag_property_list<Bundle, Prop> List;
    public:
      // Extract the normalized property and bundle types.
      typedef typename retagged_property<List>::type property;
      typedef typename retagged_bundle<property, Bundle>::type bundle;
    };

    template<typename Prop>
    struct graph_prop : normal_property<Prop, graph_bundle_t>
    { };

    template<typename Prop>
    struct vertex_prop : normal_property<Prop, vertex_bundle_t>
    { };

    template<typename Prop>
    struct edge_prop : normal_property<Prop, edge_bundle_t>
    { };
} // namespace graph_detail

// NOTE: These functions are declared, but never defined since they need to
// be overloaded by graph implementations. However, we need them to be
// declared for the functions below.
template<typename Graph, typename Tag>
typename graph_property<Graph, graph_bundle_t>::type&
get_property(Graph& g, Tag);

template<typename Graph, typename Tag>
typename graph_property<Graph, graph_bundle_t>::type const&
get_property(Graph const& g, Tag);

#ifndef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
// NOTE: This operation is a simple adaptor over the overloaded get_property
// operations.
template<typename Graph>
inline typename graph_property<Graph, graph_bundle_t>::type&
get_property(Graph& g) {
  return get_property(g, graph_bundle);
}

template<typename Graph>
inline typename graph_property<Graph, graph_bundle_t>::type const&
get_property(Graph const& g) {
  return get_property(g, graph_bundle);
}
#endif

} // namespace boost

#if BOOST_WORKAROUND(BOOST_MSVC, < 1300)
// Stay out of the way of the concept checking class
# undef Graph
# undef RandomAccessIterator
#endif

#endif /* BOOST_GRAPH_PROPERTIES_HPPA */
