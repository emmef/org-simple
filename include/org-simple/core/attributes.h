#ifndef ORG_SIMPLE_CORE_ATTRIBUTES_H
#define ORG_SIMPLE_CORE_ATTRIBUTES_H
/*
 * org-simple/core/attributes.h
 *
 * Added by michel on 2019-08-18
 * Copyright (C) 2015-2020 Michel Fleur.
 * Source https://github.com/emmef/org-simple
 * Email org-simple@emmef.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if __cplusplus >= 201703L
#if defined(__clang__) || defined(__GNUC__)
#define org_force_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define org_force_inline __forceinline
#endif
#else
#define org_force_inline inline
#endif

#endif // ORG_SIMPLE_CORE_ATTRIBUTES_H
