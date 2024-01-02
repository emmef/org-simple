#ifndef ORG_SIMPLE_M_DEBUG_H
#define ORG_SIMPLE_M_DEBUG_H
/*
 * org-simple/debug.h
 *
 * Added by michel on 2020-05-17
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

#ifdef ORG_SIMPLE_CORE_DEBUG_ENABLED
#include <cstdio>
#include <typeinfo>
#define debugLineArgs(F, ...)                                                  \
  std::printf("%s: " F "\n", typeid(*this).name(), __VA_ARGS__)
#define debugLineClass(S, C) std::printf("%s: " S "\n", typeid(C).name())
#define debugLineClassArgs(F, C, ...)                                          \
  std::printf("%s: " F "\n", typeid(C).name(), __VA_ARGS__)
#define debugLine(S) std::printf("%s: " S "\n", typeid(*this).name())
#else
#define debugLineArgs(F, ...)
#define debugLineClass(S, C)
#define debugLineClassArgs(F, C, ...)
#define debugLine(S)
#endif

#endif // ORG_SIMPLE_M_DEBUG_H
