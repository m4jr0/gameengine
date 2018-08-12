// Copyright 2018 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef KOMA_DEBUG_HPP_
#define KOMA_DEBUG_HPP_

#ifdef _DEBUG

#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif  // DBG_NEW

#endif  // _DEBUG

#endif  // KOMA_DEBUG_HPP_
