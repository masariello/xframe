/***************************************************************************
* Copyright (c) 2017, Johan Mabille, Sylvain Corlay and Wolf Vollprecht    *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XFRAME_CONFIG_HPP
#define XFRAME_CONFIG_HPP

#define XFRAME_VERSION_MAJOR 0
#define XFRAME_VERSION_MINOR 0
#define XFRAME_VERSION_PATCH 2

#ifndef DEFAULT_LABEL_LIST
#include <cstddef>
#include "xtl/xbasic_fixed_string.hpp"
#include "xtl/xmeta_utils.hpp"
#define DEFAULT_LABEL_LIST xtl::mpl::vector<int, std::size_t, char, xtl::xfixed_string<55>>
#endif

#ifndef DEFAULT_JOIN
#define DEFAULT_JOIN join::inner
#endif

#ifndef XFRAME_DEFAULT_DATA_CONTAINER
#include "xtensor/xarray.hpp"
#include "xtensor/xoptional_assembly.hpp"
#define XFRAME_DEFAULT_DATA_CONTAINER(T) xt::xoptional_assembly<xt::xarray<T>, xt::xarray<bool>>
#endif

// A higher number leads to an ICE on VS 2015
#ifndef STATIC_DIMENSION_LIMIT
#define STATIC_DIMENSION_LIMIT 4
#endif

#ifndef XFRAME_ENABLE_TRACE
#define XFRAME_ENABLE_TRACE 0
#endif

#ifndef XFRAME_OUT
#define XFRAME_OUT std::cout
#endif

#endif
