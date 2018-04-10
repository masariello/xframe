/***************************************************************************
* Copyright (c) 2017, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XFRAME_XVARIABLE_VIEW_HPP
#define XFRAME_XVARIABLE_VIEW_HPP

#include "xvariable.hpp"
#include "xcoordinate_view.hpp"

namespace xf
{

    /*******************
     * xvariable_view  *
     *******************/

    template <class CT>
    class xvariable_view
    {
    public:

        using self_type = xvariable_view<CT>;
        using xexpression_type = std::decay_t<CT>;

        static constexpr bool is_const = std::is_const<std::remove_reference_t<CT>>::value;
        using value_type = typename xexpression_type::value_type;
        using reference = std::conditional_t<is_const,
                                             typename xexpression_type::const_reference,
                                             typename xexpression_type::reference>;
        using const_reference = typename xexpression_type::const_reference;
        using pointer = std::conditional_t<is_const,
                                           typename xexpression_type::const_pointer,
                                           typename xexpression_type::pointer>;
        using const_pointer = typename xexpression_type::const_pointer;
        using size_type = typename xexpression_type::size_type;
        using difference_type = typename xexpression_type::difference_type;

        using coordinate_type = xcoordinate_view_type_t<typename xexpression_type::coordinate_type>;
        using dimension_type = typename xexpression_type::dimension_type;
        using dimension_list = typename dimension_type::label_list;

        using squeeze_map = std::map<typename dimension_type::key_type, typename dimension_type::mapped_type>;

        template <class E>
        xvariable_view(E&& e, coordinate_type&& coord, dimension_type&& dim, squeeze_map&& squeeze);

        size_type size() const noexcept;
        size_type dimension() const noexcept;
        const dimension_list& dimension_labels() const noexcept;
        const coordinate_type& coordinates() const noexcept;
        const dimension_type& dimension_mapping() const noexcept;

        xexpression_type& data() noexcept;
        const xexpression_type& data() const noexcept;

        template <std::size_t N = dynamic()>
        using selector_traits = xselector_traits<coordinate_type, dimension_type, N>;
        template <std::size_t N = dynamic()>
        using selector_type = typename selector_traits<N>::selector_type;
        template <std::size_t N = dynamic()>
        using selector_map_type = typename selector_traits<N>::selector_map_type;

        template <std::size_t N = dynamic()>
        reference select(const selector_map_type<N>& selector);

        template <std::size_t N = dynamic()>
        reference select(selector_map_type<N>&& selector);

        bool operator==(const self_type& rhs) const noexcept;
        bool operator!=(const self_type& rhs) const noexcept;

    private:

        template <class S>
        reference select_impl(const S& selector);

        template <class Idx>
        void fill_squeeze(Idx& index) const;

        CT m_e;
        coordinate_type m_coordinate;
        dimension_type m_dimension;
        squeeze_map m_squeeze;
    };

    /***************************
     * xvariable_view builders *
     ***************************/

    template <class E, class L = DEFAULT_LABEL_LIST>
    auto select(E&& e, std::map<typename std::decay_t<E>::key_type, xaxis_slice<L>>&& slices);

    /*********************************
     * xvariable_view implementation *
     *********************************/

    template <class CT>
    template <class E>
    inline xvariable_view<CT>::xvariable_view(E&& e, coordinate_type&& coord, dimension_type&& dim, squeeze_map&& squeeze)
        : m_e(std::forward<E>(e)),
          m_coordinate(std::move(coord)),
          m_dimension(std::move(dim)),
          m_squeeze(std::move(squeeze))
    {
    }

    template <class CT>
    inline auto xvariable_view<CT>::size() const noexcept -> size_type
    {
        return std::accumulate(m_coordinate.cbegin(), m_coordinate.cend(), size_type(1),
            [](size_type val, auto&& entry) {
                return val * entry.second.size();
            }
        );
    }

    template <class CT>
    inline auto xvariable_view<CT>::dimension() const noexcept -> size_type
    {
        return m_dimension.size();
    }

    template <class CT>
    inline auto xvariable_view<CT>::dimension_labels() const noexcept -> const dimension_list&
    {
        return m_dimension.labels();
    }

    template <class CT>
    inline auto xvariable_view<CT>::coordinates() const noexcept -> const coordinate_type&
    {
        return m_coordinate;
    }

    template <class CT>
    inline auto xvariable_view<CT>::dimension_mapping() const noexcept -> const dimension_type&
    {
        return m_dimension;
    }

    template <class CT>
    inline auto xvariable_view<CT>::data() noexcept -> xexpression_type&
    {
        return m_e;
    }

    template <class CT>
    inline auto xvariable_view<CT>::data() const noexcept -> const xexpression_type&
    {
        return m_e;
    }

    template <class CT>
    template <std::size_t N>
    inline auto xvariable_view<CT>::select(const selector_map_type<N>& selector) -> reference
    {
        return select(selector_map_type<N>(selector));
    }

    template <class CT>
    template <std::size_t N>
    inline auto xvariable_view<CT>::select(selector_map_type<N>&& selector) -> reference
    {
        return select_impl(selector_type<N>(std::move(selector)));
    }

    template <class CT>
    inline bool xvariable_view<CT>::operator==(const self_type& rhs) const noexcept
    {
        return m_e == rhs.m_e && m_coordinate == rhs.m_coordinate && m_dimension == rhs.m_dimension;
    }

    template <class CT>
    inline bool xvariable_view<CT>::operator!=(const self_type& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <class CT>
    template <class S>
    inline auto xvariable_view<CT>::select_impl(const S& selector) -> reference
    {
        typename S::index_type idx = selector.get_index(coordinates(), m_e.dimension_mapping());
        fill_squeeze(idx);
        return data().element(idx.cbegin(), idx.cend());
    }

    template <class CT>
    template <class Idx>
    inline void xvariable_view<CT>::fill_squeeze(Idx& index) const
    {
        for (const auto& sq : m_squeeze)
        {
            index[m_e.dimension_mapping()[sq.first]] = sq.second;
        }
    }

    /******************************************
     * xvariable_view builders implementation *
     ******************************************/

    namespace detail
    {
        template <class M, class C>
        inline void fill_coordinate_map_impl(const C& coord, M& cmap)
        {
        }

        template <class M, class C, class L0, class SL0, class... L, class... SL>
        inline void fill_coordinate_map_impl(const C& coord, M& cmap, std::pair<L0, SL0> sl0, std::pair<L, SL>... sl)
        {
            using axis_view_type = typename M::axis_type;
            const auto& ax = coord[sl0.first];
            cmap.emplace(std::make_pair(sl0.first, axis_view_type(ax, sl0.build_islice(ax))));
            fill_coordinate_map_impl(coord, cmap, std::move(sl)...);
        }
    }
    
    template <class E, class L>
    auto select(E&& e, std::map<typename std::decay_t<E>::key_type, xaxis_slice<L>>&& slices)
    {
        using coordinate_type = typename std::decay_t<E>::coordinate_type;
        using dimension_type = typename std::decay_t<E>::dimension_type;
        using dimension_label_list = typename dimension_type::label_list;
        using view_type = xvariable_view<xtl::closure_type_t<E>>;
        using squeeze_map = typename view_type::squeeze_map;
        using coordinate_view_type = typename view_type::coordinate_type;
        using map_type = typename coordinate_view_type::map_type;
        using axis_type = typename coordinate_view_type::axis_type;

        const coordinate_type& underlying_coords = e.coordinates();
        map_type coord_map;
        squeeze_map sq_map;
        dimension_label_list dim_label_list;

        for(const auto& dim_label: e.dimension_labels())
        {
            const auto& axis = underlying_coords[dim_label];
            auto slice_iter = slices.find(dim_label);
            if (slice_iter != slices.end())
            {
                if (auto* sq = (slice_iter->second).get_squeeze())
                {
                    sq_map[dim_label] = axis[*sq];
                }
                else
                {
                    coord_map.emplace(dim_label, axis_type(axis, (slice_iter->second).build_islice(axis)));
                    dim_label_list.push_back(dim_label);
                }
            }
            else
            {
                using size_type = typename std::decay_t<E>::size_type;
                coord_map.emplace(dim_label, axis_type(axis, xt::xall<size_type>(axis.size())));
                dim_label_list.push_back(dim_label);
            }
        }

        coordinate_view_type coordinate_view(std::move(coord_map));
        dimension_type view_dimension(std::move(dim_label_list));

        return view_type(std::forward<E>(e),
                         std::move(coordinate_view),
                         std::move(view_dimension),
                         std::move(sq_map));
    }
}

#endif