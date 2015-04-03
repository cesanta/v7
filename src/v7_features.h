#ifndef V7_FEATURES_H_INCLUDED
#define V7_FEATURES_H_INCLUDED

#define V7_BUILD_PROFILE_MINIMAL 1
#define V7_BUILD_PROFILE_MEDIUM 2
#define V7_BUILD_PROFILE_FULL 3

#ifndef V7_BUILD_PROFILE
#define V7_BUILD_PROFILE V7_BUILD_PROFILE_FULL
#endif

/* Only one will actually be used based on V7_BUILD_PROFILE. */
#include "features_minimal.h"
#include "features_medium.h"
#include "features_full.h"

#endif /* V7_FEATURES_H_INCLUDED */
